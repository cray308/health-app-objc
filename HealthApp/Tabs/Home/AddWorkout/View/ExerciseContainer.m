//
//  ExerciseContainer.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "ExerciseContainer.h"
#include "ViewControllerHelpers.h"
#import "WorkoutViewController.h"

@implementation ExerciseContainer
- (id) initWithGroup: (ExerciseGroup *)exerciseGroup tag: (int)tag parent: (id)parent
               timer: (WorkoutTimer *)timer exerciseTimer: (WorkoutTimer *)exerciseTimer {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    group = exerciseGroup;
    size = group->exercises->size;
    viewsArr = calloc(size, sizeof(ExerciseView *));
    self->parent = parent;
    self->timer = timer;
    self->exerciseTimer = exerciseTimer;
    setTag(self, tag);

    headerLabel = createLabel(NULL, UIFontTextStyleTitle3, NSTextAlignmentNatural);
    CFStringRef headerStr = exerciseGroup_createHeader(group);
    if (headerStr) {
        setLabelText(headerLabel, headerStr);
        CFRelease(headerStr);
    }
    [self addSubview:headerLabel];

    UIStackView *exerciseStack = createStackView(NULL, 0, 1, 5, 0, (HAEdgeInsets){5, 4, 4, 0});
    [self addSubview:exerciseStack];

    activateConstraints((id []){
        [headerLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [headerLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [headerLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [headerLabel.heightAnchor constraintEqualToConstant: 20],

        [exerciseStack.topAnchor constraintEqualToAnchor:headerLabel.bottomAnchor],
        [exerciseStack.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [exerciseStack.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [exerciseStack.bottomAnchor constraintEqualToAnchor:self.bottomAnchor]
    }, 8);

    for (int i = 0; i < size; ++i) {
        ExerciseView *v = [[ExerciseView alloc] initWithExercise:&group->exercises->arr[i] tag:i
                                                          target:self action:@selector(handleTap:)
                                                           timer:exerciseTimer];
        [exerciseStack addArrangedSubview:v];
        viewsArr[i] = v;
    }
    [exerciseStack release];
    return self;
}

- (void) dealloc {
    for (int i = 0; i < size; ++i) [viewsArr[i] release];
    free(viewsArr);
    [headerLabel release];
    [super dealloc];
}

- (void) startCircuitAndTimer: (bool)startTimer {
    exerciseTimer->container = (int) self.tag;
    currentIndex = 0;
    for (int i = 0; i < size; ++i)
        [viewsArr[i] reset];

    if (group->type == ExerciseContainerTypeAMRAP && startTimer) {
        int duration = 60 * group->reps;
        startWorkoutTimer(timer, duration, (int) self.tag, -1);
        scheduleNotification(duration, CFSTR("Finished AMRAP circuit!"));
    }
    [viewsArr[0] handleTap];
}

- (void) restartExerciseAtIndex: (int)index moveToNext: (bool)moveToNext refTime: (time_t)refTime {
    ExerciseView *v = nil;
    bool endExercise = false;
    pthread_mutex_lock(&timerLock);

    if (group && index == currentIndex) {
        ExerciseView *temp = viewsArr[index];
        if (temp->exercise->type == ExerciseTypeDuration) {
            v = temp;
            int diff = (int) (refTime - exerciseTimer->refTime);
            if (diff >= exerciseTimer->duration)
                endExercise = true;
            else
                startWorkoutTimer(exerciseTimer, exerciseTimer->duration - diff,
                                  -1, (int) v->button.tag);
        }
    }
    pthread_mutex_unlock(&timerLock);

    if (v && endExercise)
        [self stopExerciseAtIndex:index moveToNext:moveToNext];
}

- (void) restartGroupAtIndex: (int)index refTime: (time_t)refTime {
    WorkoutViewController *p = nil;
    bool endGroup = false;
    pthread_mutex_lock(&timerLock);

    if (group && index == self.tag && group->type == ExerciseContainerTypeAMRAP) {
        p = parent;
        int diff = (int) (refTime - timer->refTime);
        if (diff >= timer->duration)
            endGroup = true;
        else
            startWorkoutTimer(timer, timer->duration - diff, (int) self.tag, -1);
    }
    pthread_mutex_unlock(&timerLock);

    if (p && endGroup)
        [self finishGroupAtIndex:index];
}

- (void) stopExerciseAtIndex: (int)index moveToNext: (bool)moveToNext {
    ExerciseView *v = nil;
    pthread_mutex_lock(&timerLock);
    if (group && index == currentIndex) {
        ExerciseView *temp = viewsArr[index];
        if (temp->exercise->type == ExerciseTypeDuration)
            v = temp;
    }
    pthread_mutex_unlock(&timerLock);
    if (v) {
        if (moveToNext)
            [self handleTap:v->button];
        else
            v->button.userInteractionEnabled = true;
    }
}

- (void) finishGroupAtIndex: (int)index {
    WorkoutViewController *p = nil;
    pthread_mutex_lock(&timerLock);
    if (group && index == self.tag && group->type == ExerciseContainerTypeAMRAP) {
        p = parent;
        group = NULL;
    }
    pthread_mutex_unlock(&timerLock);
    if (p) [p finishedExerciseGroup];
}

- (void) handleTap: (UIButton *)btn {
    bool isDone = false;
    pthread_mutex_lock(&timerLock);
    if (group && (int) btn.tag == currentIndex) {
        ExerciseView *v = viewsArr[currentIndex];
        if ([v handleTap]) {
            if (++currentIndex == size) {
                switch (group->type) {
                    case ExerciseContainerTypeRounds:
                        if (++group->completedReps == group->reps) {
                            isDone = true;
                            group = NULL;
                        } else {
                            CFStringRef headerStr = exerciseGroup_createHeader(group);
                            setLabelText(headerLabel, headerStr);
                            CFRelease(headerStr);
                        }
                        break;

                    case ExerciseContainerTypeDecrement:
                        if (--group->completedReps == 0) {
                            isDone = true;
                            group = NULL;
                        } else {
                            ExerciseEntry *e;
                            array_iter(group->exercises, e) {
                                if (e->type == ExerciseTypeReps) e->reps -= 1;
                            }
                        }
                        break;

                    default:
                        break;
                }
                if (group)
                    [self startCircuitAndTimer:0];
            } else {
                [viewsArr[currentIndex] handleTap];
            }
        }
    }
    pthread_mutex_unlock(&timerLock);
    if (isDone)
        [parent finishedExerciseGroup];
}
@end
