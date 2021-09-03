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

    groupsStack = createStackView(NULL, 0, 1, 20, 0, (HAEdgeInsets){0, 4, 4, 0});
    UIButton *startBtn = createButton(CFSTR("Start"), UIColor.systemGreenColor, nil,
                                      UIFontTextStyleSubheadline,
                                      UIColor.secondarySystemGroupedBackgroundColor, true, true, 0,
                                      self, @selector(startEndWorkout:));
    UIView *btnContainer = createView(nil, false);
    [btnContainer addSubview:startBtn];

    for (unsigned i = 0; i < workout->activities->size; ++i) {
        ExerciseContainer *v = [[ExerciseContainer alloc]
                                initWithGroup:&workout->activities->arr[i] tag:i
                                target:self action:@selector(handleTap:)];
        [groupsStack addArrangedSubview:v];
        [v release];
    }

    UIStackView *vStack = createStackView((id[]){btnContainer, groupsStack}, 2, 1, 20, 0,
                                          (HAEdgeInsets){10, 0, 0, 0});
    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [scrollView.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:guide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:guide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [startBtn.topAnchor constraintEqualToAnchor:btnContainer.topAnchor],
        [startBtn.bottomAnchor constraintEqualToAnchor:btnContainer.bottomAnchor],
        [startBtn.leadingAnchor constraintEqualToAnchor:btnContainer.leadingAnchor constant:8],
        [startBtn.widthAnchor constraintEqualToAnchor:btnContainer.widthAnchor multiplier:0.4],
        [startBtn.heightAnchor constraintEqualToConstant: 30]
    }, 14);

    [btnContainer release];
    [vStack release];
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
        setButtonTitle(btn, CFSTR("End"), 0);
        setButtonColor(btn, UIColor.systemRedColor, 0);
        setTag(btn, 1);
        workout->startTime = time(NULL);
        [self handleTapForGroup:0 exercise:0 option:EventOptionStartGroup];
    } else {
        Workout *w = NULL;
        pthread_mutex_lock(&timerLock);
        if (workout) {
            workout->stopTime = time(NULL) + 1;
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
            workout->stopTime = time(NULL) + 1;
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
