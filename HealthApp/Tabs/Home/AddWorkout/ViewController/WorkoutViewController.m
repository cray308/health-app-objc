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
    AddWorkoutCoordinator *delegate;
    Workout *workout;
    UIStackView *groupsStack;
    ExerciseContainer *firstContainer;
    NSObject *observers[2];
}
@end

@implementation WorkoutViewController
- (id) initWithDelegate: (AddWorkoutCoordinator *)delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = delegate;
    workout = delegate->workout;
    setupTimers(workout, self);
    return self;
}

- (void) dealloc {
    cleanupWorkoutNotifications(observers);
    [groupsStack release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = (__bridge NSString*) workout->title;
    setTintColor(self.navigationController.navigationBar, UIColor.systemBlueColor);

    groupsStack = createStackView(NULL, 0, 1, 20, 0, (HAEdgeInsets){20, 4, 20, 4});
    UIButton *startBtn = createButton(localize(CFSTR("start")),
                                      UIColor.systemGreenColor, UIFontTextStyleSubheadline,
                                      nil, true, true, 0, self, @selector(startEndWorkout:));
    setNavButton(self.navigationItem, false, startBtn, self.view.frame.size.width);

    for (unsigned i = 0; i < workout->activities->size; ++i) {
        ExerciseContainer *v = [[ExerciseContainer alloc]
                                initWithGroup:&workout->activities->arr[i] tag:i
                                target:self action:@selector(handleTap:)];
        [groupsStack addArrangedSubview:v];
        [v release];
    }

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:groupsStack];

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [scrollView.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:guide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:guide.bottomAnchor],

        [groupsStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [groupsStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [groupsStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [groupsStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [groupsStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor]
    }, 9);

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

    ExerciseView *v = firstContainer->viewsArr[workout->group->index];
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
                [firstContainer->viewsArr[i++] configureWithEntry:e];
            }
            if (header)
                CFRelease(header);
            break;
        case TransitionFinishedExercise:
            v = firstContainer->viewsArr[workout->group->index];
            [v configureWithEntry:workout->entry];
        case TransitionNoChange:
            break;
    }

cleanup:
    pthread_mutex_unlock(&timerLock);
    if (finishedWorkout)
        addWorkoutCoordinator_completedWorkout(delegate, false, true);
}

- (void) finishedWorkoutTimerForType: (uchar)type group: (uint)group entry: (uint)entry {
    [self handleTapForGroup:group exercise:entry option:type ? 0 : EventOptionFinishGroup];
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
