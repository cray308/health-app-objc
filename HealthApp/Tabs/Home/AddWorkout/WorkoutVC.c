#include "WorkoutVC.h"
#include "AppDelegate.h"
#include "UpdateMaxesVC.h"
#include "Views.h"
#include "WorkoutVC_Ext.h"

extern CFStringRef UIApplicationDidBecomeActiveNotification;
extern uint32_t UIAccessibilityLayoutChangedNotification;

Class WorkoutVCClass;

enum {
    EventFinishCircuit = 1,
    EventFinishExercise
};

static void handleEvent(id, WorkoutVC *, int, int, int);
static void restartTimers(CFNotificationCenterRef, void *,
                          CFStringRef, const void *, CFDictionaryRef);

void initWorkoutData(int week) {
    initNotificationCache();
    initExerciseData(week);
}

#pragma mark - VC init/free

id workoutVC_init(Workout *workout) {
    id self = new(WorkoutVCClass);
    WorkoutVC *d = getIVVC(WorkoutVC, self);
    d->workout = workout;
    d->containers = malloc((unsigned)workout->size * sizeof(CVPair));
    d->timers[0].type = 0;
    d->timers[1].type = 1;
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVC *d = getIVVC(WorkoutVC, self);

    for (int i = 0; i < d->workout->size; ++i) {
        Circuit *c = &d->workout->circuits[i];
        if (c->header) CFRelease(c->header);
        Exercise *eEnd = &c->exercises[c->size];
        for (Exercise *e = c->exercises; e < eEnd; ++e) {
            CFRelease(e->title);
            if (e->rest) CFRelease(e->rest);
            if (e->header) CFRelease(e->header);
        }
        free(c->exercises);
        if (d->containers[i].view) releaseView(d->containers[i].view);
    }
    free(d->workout->circuits);
    free(d->workout);
    free(d->containers);
    msgSupV(supSig(), self, VC, _cmd);
}

static void handleFinishedWorkout(WorkoutVC *d, bool pop) {
    int *lifts = NULL;
    if (d->weights[3]) {
        lifts = malloc(sizeof(int) << 2);
        memcpy(lifts, d->weights, sizeof(int) << 2);
        d->workout->duration = max(d->workout->duration, MinWorkoutDuration);
    }
    addWorkout(d->workout, d->workout->day, lifts, pop);
}

static void cleanupNotifications(id self, WorkoutVC *d) {
    id unCenter = getNotificationCenter();
    msgV(objSig(void), unCenter, sel_getUid("removeAllPendingNotificationRequests"));
    msgV(objSig(void), unCenter, sel_getUid("removeAllDeliveredNotifications"));
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    d->done = true;
    d->timers[0].active = d->timers[1].active = 0;
}

#pragma mark - Lifecycle

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSupV(supSig(), self, VC, _cmd);

    WorkoutVC *d = getIVVC(WorkoutVC, self);
    CFStringRef titleKey = createWorkoutTitleKey(d->workout->type, d->workout->nameIdx);
    id startButton = createButton(localize(CFSTR("start")), ColorGreen, self, getCustomButtonSel());
    setupNavItem(self, titleKey, (id []){nil, startButton});
    CFRelease(titleKey);

    id stack = createVStack(NULL, 0);
    setSpacing(stack, GroupSpacing);

    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef exerciseProgressFormat = localize(CFSTR("exerciseProgress"));
    SEL tapSel = getTapSel();
    StatusView *sv;
    for (int i = 0; i < d->workout->size; ++i) {
        Circuit *c = &d->workout->circuits[i];
        CVPair *cv = &d->containers[i];
        cv->view = containerView_init(&cv->data, c->header);
        addArrangedSubview(stack, cv->view);

        bool addHint = c->size > 1;
        for (int j = 0; j < c->size; ++j) {
            Exercise *e = &c->exercises[j];
            id btn = statusView_init(&sv, e->header, e->title, (int)((i << 8) | j), self, tapSel);
            sv->exercise = e;
            setTitle(sv->button, e->title, ControlStateDisabled);
            setEnabled(sv->button, false);
            if (addHint) {
                CFStringRef exerciseHint = formatStr(locale, exerciseProgressFormat, j + 1, c->size);
                setAccessibilityHint(sv->button, exerciseHint);
                CFRelease(exerciseHint);
            }
            exerciseView_configure(sv);
            addArrangedSubview(cv->data->stack, btn);
            releaseView(btn);
        }
    }
    CFRelease(locale);
    CFRelease(exerciseProgressFormat);
    setHidden(d->containers[0].data->divider, true);

    setupHierarchy(self, stack, createScrollView(), ColorPrimaryBGGrouped);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), self, restartTimers,
                                    UIApplicationDidBecomeActiveNotification, NULL,
                                    CFNotificationSuspensionBehaviorDrop);
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id button) {
    WorkoutVC *d = getIVVC(WorkoutVC, self);
    if (!getTag(button)) {
        CFStringRef str = localize(CFSTR("end"));
        setTitle(button, str, ControlStateNormal);
        CFRelease(str);
        setTitleColor(button, getColor(ColorRed), ControlStateNormal);
        setTag(button, 1);
        d->workout->startTime = time(NULL);
        circuit_start(d->workout->circuits, d->timers, true);
        CFArrayRef views = getArrangedSubviews(d->containers[0].data->stack);
        StatusView *sv = getIVV(StatusView, CFArrayGetValueAtIndex(views, 0));
        exerciseView_configure(sv);
        id nextView = d->workout->circuits[0].header ? d->containers[0].data->header : sv->button;
        UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
    } else {
        cleanupNotifications(self, d);
        if (workout_isCompleted(d->workout)) {
            handleFinishedWorkout(d, true);
        } else {
            addWorkout(d->workout, UCHAR_MAX, NULL, true);
        }
    }
}

void workoutVC_viewWillDisappear(id self, SEL _cmd, bool animated) {
    msgSupV(supSig(bool), self, VC, _cmd, animated);

    if (msgV(objSig(bool), self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *d = getIVVC(WorkoutVC, self);
        if (!d->done) {
            cleanupNotifications(self, d);
            if (d->workout->startTime) {
                if (workout_isCompleted(d->workout)) {
                    handleFinishedWorkout(d, false);
                } else {
                    addWorkout(d->workout, UCHAR_MAX, NULL, false);
                }
            }
        }
    }
}

#pragma mark - Event Handling

void workoutVC_handleTap(id self, SEL _cmd _U_, id button) {
    int tag = (int)getTag(button);
    int section = (tag & 0xff00) >> 8, row = tag & UCHAR_MAX;
    handleEvent(self, getIVVC(WorkoutVC, self), section, row, 0);
}

void handleEvent(id self, WorkoutVC *d, int section, int row, int event) {
    Circuit *circuit = &d->workout->circuits[section];
    CVPair *pair = &d->containers[section];
    CFArrayRef views = getArrangedSubviews(pair->data->stack);
    StatusView *v = getIVV(StatusView, CFArrayGetValueAtIndex(views, row));
    id nextView = nil;
    bool exerciseDone;
    int transition = 0;
    switch (event) {
        case EventFinishCircuit:
            transition = workout_increment(d->workout, d->timers);
            break;

        case EventFinishExercise:
            setUserInteractionEnabled(v->button, true);
            if (d->workout->type == WorkoutEndurance) {
                setTextColor(v->header, getColor(ColorGreen));
                CFStringRef msg = localize(CFSTR("exerciseDurationMessage"));
                setText(v->header, msg);
                CFRelease(msg);
                statusView_updateAccessibility(v);
                nextView = v->button;
                break;
            }
        default:
            exerciseDone = exercise_cycle(v->exercise, d->timers);
            exerciseView_configure(v);
            if (exerciseDone) {
                transition = circuit_increment(circuit, d->timers, v->exercise);
                if (transition == TransitionFinishedCircuitDeleteFirst)
                    transition = workout_increment(d->workout, d->timers);
            }
    }

    if (transition == TransitionCompletedWorkout) {
        cleanupNotifications(self, d);
        workout_setDuration(d->workout);
        if (UIAccessibilityIsVoiceOverRunning()) {
            CFStringRef msg = localize(CFSTR("workoutCompleteMsg"));
            dispatch_after(dispatch_time(0, 4000000000), dispatch_get_main_queue(), ^{
                UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)msg);
                CFRelease(msg);
            });
        }
        handleFinishedWorkout(d, true);
        return;
    } else if (transition == TransitionFinishedCircuitDeleteFirst) {
        removeFromSuperview(pair->view);
        releaseView(pair->view);
        (pair++)->view = nil;
        views = getArrangedSubviews(pair->data->stack);
        setHidden(pair->data->divider, true);
        exerciseView_configure(getIVV(StatusView, CFArrayGetValueAtIndex(views, 0)));
        nextView = pair->data->header;
    } else if (transition == TransitionFinishedCircuit) {
        if (circuit->reps > 1 && circuit->type == CircuitRounds) {
            CFLocaleRef locale = CFLocaleCopyCurrent();
            CFStringRef newNumber = formatStr(locale, CFSTR("%d"), circuit->completedReps + 1);
            updateRange(circuit->header, &circuit->range, newNumber, locale);
            setText(pair->data->header, circuit->header);
            nextView = pair->data->header;
        }

        bool refreshButtons = circuit->type == CircuitDecrement;
        for (int i = 0; i < circuit->size; ++i) {
            v = getIVV(StatusView, CFArrayGetValueAtIndex(views, i));
            if (refreshButtons && v->exercise->type == ExerciseReps) {
                setEnabled(v->button, true);
                setTitle(v->button,
                         v->exercise->title, ControlStateNormal | ControlStateDisabled);
                setEnabled(v->button, false);
            }
            exerciseView_configure(v);
        }

        if (!nextView) nextView = getIVV(StatusView, CFArrayGetValueAtIndex(views, 0))->button;
    } else if (transition == TransitionFinishedExercise) {
        v = getIVV(StatusView, CFArrayGetValueAtIndex(views, circuit->index));
        exerciseView_configure(v);
        nextView = v->button;
    } else if (d->workout->testMax) {
        presentModalVC(self, updateMaxesVC_init(self, row));
    }

    if (nextView) UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
}

#pragma mark - Notifications

void restartTimers(CFNotificationCenterRef center _U_, void *self,
                   CFStringRef name _U_, const void *object _U_, CFDictionaryRef userInfo _U_) {
    WorkoutVC *d = getIVVC(WorkoutVC, self);
    time_t now = time(NULL);
    for (int i = 1; i >= 0; --i) {
        WorkoutTimer *t = &d->timers[i];
        if (t->active) {
            int diff = (int)(now - t->refTime);
            if (diff >= t->duration) {
                t->active = 0;
                handleEvent(self, d, t->section, t->row, i + 1);
            } else {
                workoutTimer_start(t, t->duration - diff, false);
            }
        }
    }
}

void workoutVC_receivedNotification(id self, int identifier) {
    WorkoutVC *d = getIVVC(WorkoutVC, self);
    int type = identifier % 2;
    WorkoutTimer *timer = &d->timers[type];
    if (!timer->active || timer->identifier != identifier) return;

    timer->active = 0;
    if (type == TimerCircuit && d->timers[TimerExercise].active) {
        d->timers[TimerExercise].active = 0;
        CFStringRef idStr = formatStr(NULL, CFSTR("%d"), d->timers[TimerExercise].identifier);
        CFArrayRef ids = CFArrayCreate(NULL, (const void *[]){idStr}, 1, &kCFTypeArrayCallBacks);
        msgV(objSig(void, CFArrayRef), getNotificationCenter(),
             sel_getUid("removePendingNotificationRequestsWithIdentifiers:"), ids);
        CFRelease(idStr);
        CFRelease(ids);
    }

    handleEvent(self, d, timer->section, timer->row, type + 1);
}

#pragma mark - Modal Delegate

void workoutVC_finishedBottomSheet(id self, int index, int weight) {
    WorkoutVC *d = getIVVC(WorkoutVC, self);
    d->weights[index] = weight;
    handleEvent(self, d, 0, index, 0);
}
