#include "AddWorkoutCoordinator.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"
#import "StatusButton.h"
#import "SwiftBridging.h"

void exerciseView_configure(StatusButton *v, ExerciseEntry *e) {
    CFStringRef setsStr = exerciseEntry_createSetsTitle(e);
    CFStringRef title = exerciseEntry_createTitle(e);
    setButtonTitle(v->button, title, 0);
    setLabelText(v->headerLabel, setsStr);
    statusButton_updateAccessibility(v, e->stateNames[e->state]);

    switch (e->state) {
        case ExerciseStateDisabled:
            setBackground(v->box, UIColor.systemGrayColor);
            enableButton(v->button, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseDuration)
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

@interface WorkoutViewController: UIViewController @end
@interface WorkoutViewController() {
    @public AddWorkoutCoordinator *delegate;
    @public Workout *workout;
    Container containers[10], *first;
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
    if (!checkGreaterThanMinVersion())
        self.navigationController.navigationBar.tintColor = UIColor.systemRedColor;

    UIStackView *stack = createStackView(NULL, 0, 1, 20, (Padding){20, 8, 20, 8});
    UIButton *startBtn = createButton(localize(CFSTR("start")), UIColor.systemGreenColor,
                                      0, 0, self, @selector(startEndWorkout:), -1);
    setNavButton(self.navigationItem, false, startBtn, self.view.frame.size.width);

    for (unsigned i = 0; i < workout->activities->size; ++i) {
        Circuit *c = &workout->activities->arr[i];
        CFStringRef header = circuit_createHeader(c);
        [stack addArrangedSubview:createContainer(&containers[i], header, 0, 0, 0)];

        for (unsigned j = 0; j < c->exercises->size; ++j) {
            id v = statusButton_init(NULL, false, (i << 8) | j, self, @selector(handleTap:));
            exerciseView_configure(v, &c->exercises->arr[j]);
            container_add(&containers[i], v);
        }
        hideView(containers[i].headerLabel, !header);
        if (header)
            CFRelease(header);
    }

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    pin(scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
    addVStackToScrollView(stack, scrollView);

    [scrollView release];
    [stack release];

    first = &containers[0];
    hideView(first->divider, true);
    __weak WorkoutViewController *weakSelf = self;
    setupDeviceEventNotifications(observers, ^(id note _U_){ [weakSelf restartTimers]; },
                                  ^(id note _U_){ [weakSelf stopTimers]; });
}

- (void) startEndWorkout: (UIButton *)btn {
    if (!btn.tag) {
        setButtonTitle(btn, localize(CFSTR("end")), 0);
        setButtonColor(btn, UIColor.systemRedColor, 0);
        setTag(btn, 1);
        workout->startTime = time(NULL);
        [self handleTapForGroup:0 exercise:0 event:EventStartGroup];
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
    [self handleTapForGroup:groupIdx exercise:exerciseIdx event:EventNone];
}

- (void) handleTapForGroup: (unsigned)gIdx exercise: (unsigned)eIdx event: (byte)event {
    bool finishedWorkout = false;
    pthread_mutex_lock(&timerLock);
    if (!workout) {
        goto cleanup;
    } else if (gIdx != workout->index || eIdx != workout->group->index) {
        if (event != EventFinishGroup || gIdx != workout->index)
            goto cleanup;
    }

    StatusButton *v = first->views->arr[workout->group->index];
    switch (workout_findTransitionForEvent(workout, v, v->button, event)) {
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
            CFStringRef header = circuit_createHeader(workout->group);
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
        [self handleTapForGroup:groupIdx exercise:exerciseIdx event:EventNone];
    if (endGroup)
        [self handleTapForGroup:groupIdx exercise:0 event:EventFinishGroup];
}
@end

id workoutVC_init(AddWorkoutCoordinator *delegate) {
    WorkoutViewController *this = [[WorkoutViewController alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    this->workout = delegate->workout;
    setupTimers(this->workout, this);
    return this;
}

void workoutVC_finishedTimer(WorkoutViewController *vc, byte type, unsigned gIdx, unsigned eIdx) {
    [vc handleTapForGroup:gIdx exercise:eIdx event:type ? EventNone : EventFinishGroup];
}
