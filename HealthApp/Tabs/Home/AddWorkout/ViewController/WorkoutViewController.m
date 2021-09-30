//
//  WorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#import "WorkoutViewController.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"
#import "ExerciseContainer.h"

@interface WorkoutViewController() {
    @public AddWorkoutCoordinator *delegate;
    @public Workout *workout;
    UIStackView *groupsStack;
    ExerciseContainer *firstContainer;
    NSObject *observers[2];
}
@end

@implementation WorkoutViewController
- (void) dealloc {
    cleanupWorkoutNotifications(observers);
    [groupsStack release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = _nsstr(workout->title);
    setTintColor(self.navigationController.navigationBar, UIColor.systemBlueColor);

    groupsStack = createStackView(NULL, 0, 1, 20, (Padding){20, 4, 20, 4});
    UIButton *startBtn = createButton(localize(CFSTR("start")), UIColor.systemGreenColor,
                                      0, 0, self, @selector(startEndWorkout:));
    setNavButton(self.navigationItem, false, startBtn, self.view.frame.size.width);

    unsigned i = 0; ExerciseGroup *g;
    array_iter(workout->activities, g) {
        UIView *v = exerciseContainer_init(g, i++, self, @selector(handleTap:));
        [groupsStack addArrangedSubview:v];
        [v release];
    }

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:groupsStack];

    pin(scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
    pin(groupsStack, scrollView, (Padding){0}, 0);
    setEqualWidths(groupsStack, scrollView);

    [scrollView release];

    firstContainer = groupsStack.arrangedSubviews[0];
    firstContainer->divider.hidden = true;
    __weak WorkoutViewController *weakSelf = self;
    observers[0] = createDeviceEventNotification(UIApplicationDidBecomeActiveNotification,
                                                 ^(id note _U_){ [weakSelf restartTimers]; });
    observers[1] = createDeviceEventNotification(UIApplicationWillResignActiveNotification,
                                                 ^(id note _U_){ [weakSelf stopTimers]; });
}

- (void) startEndWorkout: (UIButton *)btn {
    if (!btn.tag) { // tapped start
        setButtonTitle(btn, localize(CFSTR("end")), 0);
        setButtonColor(btn, UIColor.systemRedColor, 0);
        setTag(btn, 1);
        workout->startTime = time(NULL);
        [self handleTapForGroup:0 exercise:0 option:EventOptionStartGroup];
    } else {
        Workout *w = NULL;
        pthread_mutex_lock(&timerLock);
        if (workout) {
            workout_setDuration(workout);
            w = workout;
            workout = NULL;
        }
        pthread_mutex_unlock(&timerLock);
        if (w)
            addWorkoutCoordinator_stoppedWorkout(delegate);
    }
}

- (void) handleTap: (UIButton *)btn {
    unsigned tag = (unsigned) btn.tag, groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & 0xff;
    [self handleTapForGroup:groupIdx exercise:exerciseIdx option:0];
}

- (void) handleTapForGroup: (uint)groupIdx exercise: (uint)exerciseIdx option: (uint)option {
    bool finishedWorkout = false;
    pthread_mutex_lock(&timerLock);
    if (!workout) {
        goto cleanup;
    } else if (groupIdx != workout->index || exerciseIdx != workout->group->index) {
        if (option != EventOptionFinishGroup || groupIdx != workout->index)
            goto cleanup;
    }

    StatusButton *v = firstContainer->viewsArr[workout->group->index];
    switch (workout_findTransitionForEvent(workout, v, v->button, option)) {
        case TransitionCompletedWorkout:
            finishedWorkout = true;
            workout_setDuration(workout);
            workout = NULL;
            break;
        case TransitionFinishedCircuitDeleteFirst:
            firstContainer = groupsStack.arrangedSubviews[1];
            [groupsStack.arrangedSubviews.firstObject removeFromSuperview];
        case TransitionFinishedCircuit: ;
            firstContainer->divider.hidden = true;
            CFStringRef header = exerciseGroup_createHeader(workout->group);
            setLabelText(firstContainer->headerLabel, header);
            ExerciseEntry *e;
            int i = 0;
            array_iter(workout->group->exercises, e) {
                exerciseView_configure(firstContainer->viewsArr[i++], e);
            }
            if (header)
                CFRelease(header);
            break;
        case TransitionFinishedExercise:
            v = firstContainer->viewsArr[workout->group->index];
            exerciseView_configure(v, workout->entry);
        case TransitionNoChange:
            break;
    }

cleanup:
    pthread_mutex_unlock(&timerLock);
    if (finishedWorkout)
        addWorkoutCoordinator_completedWorkout(delegate, false, true);
}

- (void) stopTimers {
    pthread_mutex_lock(&timerLock);
    workout_stopTimers(workout);
    pthread_mutex_unlock(&timerLock);
}

- (void) restartTimers {
    bool endExercise = false, endGroup = false;
    unsigned groupIdx = 0, exerciseIdx = 0;
    pthread_mutex_lock(&timerLock);
    if (workout) {
        time_t now = time(NULL);
        endExercise = workout_restartExerciseTimer(workout, now);
        endGroup = workout_restartGroupTimer(workout, now);
        if (endExercise) {
            groupIdx = workout->savedInfo.exerciseInfo.group;
            exerciseIdx = workout->savedInfo.exerciseInfo.tag;
        }
        if (endGroup)
            groupIdx = workout->savedInfo.groupTag;
    }
    pthread_mutex_unlock(&timerLock);

    if (endExercise)
        [self handleTapForGroup:groupIdx exercise:exerciseIdx option:0];
    if (endGroup)
        [self handleTapForGroup:groupIdx exercise:0 option:EventOptionFinishGroup];
}
@end

id workoutVC_init(AddWorkoutCoordinator *delegate) {
    WorkoutViewController *this = [[WorkoutViewController alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    this->workout = delegate->workout;
    setupTimers(this->workout, this);
    return this;
}

void workoutVC_finishedTimer(WorkoutViewController *vc, uchar type, uint group, uint entry) {
    [vc handleTapForGroup:group exercise:entry option:type ? 0 : EventOptionFinishGroup];
}
