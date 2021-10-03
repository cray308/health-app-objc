//
//  WorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#import "WorkoutViewController.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"
#import "StatusButton.h"

void exerciseView_configure(StatusButton *v, ExerciseEntry *e) {
    CFStringRef setsStr = exerciseEntry_createSetsTitle(e);
    CFStringRef title = exerciseEntry_createTitle(e);
    setButtonTitle(v->button, title, 0);
    setLabelText(v->headerLabel, setsStr);

    switch (e->state) {
        case ExerciseStateDisabled:
            setBackground(v->box, UIColor.systemGrayColor);
            enableButton(v->button, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseTypeDuration)
                v->button.userInteractionEnabled = false;
        case ExerciseStateResting:
            enableButton(v->button, true);
            setBackground(v->box, UIColor.systemOrangeColor);
            break;
        case ExerciseStateCompleted:
            enableButton(v->button, false);
            setBackground(v->box, UIColor.systemGreenColor);
    }

    CFRelease(title);
    if (setsStr)
        CFRelease(setsStr);
}

@interface WorkoutViewController() {
    @public AddWorkoutCoordinator *delegate;
    @public Workout *workout;
    SectionContainer containers[10], *first;
    NSObject *observers[2];
}
@end

@implementation WorkoutViewController
- (void) dealloc {
    cleanupWorkoutNotifications(observers);
    containers_free(containers, 10);
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = _nsstr(workout->title);
    setTintColor(self.navigationController.navigationBar, UIColor.systemBlueColor);

    UIStackView *stack = createStackView(NULL, 0, 1, 20, (Padding){20, 8, 20, 8});
    UIButton *startBtn = createButton(localize(CFSTR("start")), UIColor.systemGreenColor,
                                      0, 0, self, @selector(startEndWorkout:), -1);
    setNavButton(self.navigationItem, false, startBtn, self.view.frame.size.width);

    for (uint i = 0; i < workout->activities->size; ++i) {
        ExerciseGroup *g = &workout->activities->arr[i];
        CFStringRef header = exerciseGroup_createHeader(g);
        [stack addArrangedSubview:createContainer(&containers[i], header, 0, 0, 0)];

        for (unsigned j = 0; j < g->exercises->size; ++j) {
            id v = statusButton_init(NULL, false, (i << 8) | j, self, @selector(handleTap:));
            exerciseView_configure(v, &g->exercises->arr[j]);
            container_add(&containers[i], v);
        }
        hideView(containers[i].headerLabel, !header);
        if (header)
            CFRelease(header);
    }

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:stack];

    pin(scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
    pin(stack, scrollView, (Padding){0}, 0);
    setEqualWidths(stack, scrollView);

    [scrollView release];
    [stack release];

    first = &containers[0];
    hideView(first->divider, true);
    __weak WorkoutViewController *weakSelf = self;
    setupDeviceEventNotifications(observers, ^(id note _U_){ [weakSelf restartTimers]; },
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

    StatusButton *v = first->views->arr[workout->group->index];
    switch (workout_findTransitionForEvent(workout, v, v->button, option)) {
        case TransitionCompletedWorkout:
            finishedWorkout = true;
            workout_setDuration(workout);
            workout = NULL;
            break;
        case TransitionFinishedCircuitDeleteFirst:
            first = &containers[workout->index];
            removeView(containers[workout->index - 1].view);
        case TransitionFinishedCircuit: ;
            hideView(first->divider, true);
            CFStringRef header = exerciseGroup_createHeader(workout->group);
            setLabelText(first->headerLabel, header);
            ExerciseEntry *e; int i = 0;
            array_iter(workout->group->exercises, e)
                exerciseView_configure(first->views->arr[i++], e);
            if (header)
                CFRelease(header);
            break;
        case TransitionFinishedExercise:
            v = first->views->arr[workout->group->index];
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
