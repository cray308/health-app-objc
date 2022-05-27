#include "WorkoutVC.h"
#include "AppDelegate.h"
#include "Views.h"
#include "WorkoutVC_Ext.h"

extern CFStringRef UIApplicationDidBecomeActiveNotification;
extern uint32_t UIAccessibilityLayoutChangedNotification;

Class WorkoutVCClass;

enum {
    EventFinishGroup = 1,
    EventFinishExercise
};

static CFStringRef exerciseProgressFmt;

static void handleEvent(id, WorkoutVC *, int, int, int);
static void restartTimers(CFNotificationCenterRef, void *,
                          CFNotificationName, const void *, CFDictionaryRef);

void initWorkoutData(int week, float *multiplier) {
    exerciseProgressFmt = localize(CFSTR("exerciseProgressHint"));
    initNotificationCache();
    initExerciseData(week, multiplier);
}

#pragma mark - VC init/free

id workoutVC_init(Workout *workout) {
    id self = new(WorkoutVCClass);
    WorkoutVC *d = (WorkoutVC *)getIVVC(self);
    d->workout = workout;
    d->containers = malloc((unsigned)workout->size * sizeof(CVPair));
    for (unsigned char i = 0; i < 2; ++i) {
        memcpy(&d->timers[i], &(WorkoutTimer){.type = i}, sizeof(WorkoutTimer));
    }
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVC *d = (WorkoutVC *)getIVVC(self);

    int size = d->workout->size;
    Circuit *cEnd = &d->workout->activities[size];
    for (Circuit *c = d->workout->activities; c < cEnd; ++c) {
        if (c->header) CFRelease(c->header);
        ExerciseEntry *eEnd = &c->exercises[c->size];
        for (ExerciseEntry *e = c->exercises; e < eEnd; ++e) {
            CFRelease(e->title);
            if (e->rest) CFRelease(e->rest);
            if (e->header) CFRelease(e->header);
        }
        free(c->exercises);
    }
    free(d->workout->activities);
    free(d->workout);

    for (int i = 0; i < size; ++i) {
        releaseV(d->containers[i].view);
    }
    free(d->containers);
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);
}

static void handleFinishedWorkout(id self, WorkoutVC *d, bool pop) {
    short *lifts = NULL;
    if (d->weights[3]) {
        lifts = malloc(sizeof(short) << 2);
        memcpy(lifts, d->weights, sizeof(short) << 2);
        d->workout->duration = max(d->workout->duration, MinWorkoutDuration);
    }
    addWorkoutData(self, d->workout, d->workout->day, lifts, pop);
}

static void cleanupWorkoutNotifications(id self, WorkoutVC *d) {
    id uc = clsF0(id, objc_getClass("UNUserNotificationCenter"), sel_getUid("currentNotificationCenter"));
    msg0(void, uc, sel_getUid("removeAllPendingNotificationRequests"));
    msg0(void, uc, sel_getUid("removeAllDeliveredNotifications"));
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    d->done = true;
    d->timers[0].active = 0;
    d->timers[1].active = 0;
}

#pragma mark - Lifecycle

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    WorkoutVC *d = (WorkoutVC *)getIVVC(self);
    CFStringRef title = formatStr(NULL, CFSTR("wkNames%d%02d"), d->workout->type, d->workout->jIdx);
    id startBtn = createButton(localize(CFSTR("start")), ColorGreen,
                               UIFontTextStyleBody, self, sel_getUid("startEndWorkout:"));
    setupNavItem(self, title, (id []){nil, startBtn});
    CFRelease(title);

    id stack = createVStack(NULL, 0);
    setSpacing(stack, GroupSpacing);
    setLayoutMargins(stack, VCMargins);

    SEL btnTap = sel_getUid("buttonTapped:");
    ContainerView *cv;
    StatusView *sv;
    CFLocaleRef locale = CFLocaleCopyCurrent();
    for (int i = 0; i < d->workout->size; ++i) {
        Circuit *c = &d->workout->activities[i];
        d->containers[i].view = containerView_init(&cv, c->header);
        d->containers[i].data = cv;
        if (!i) {
            d->first = cv;
            setHidden(cv->divider, true);
        }
        addArrangedSubview(stack, d->containers[i].view);

        bool addHint = c->size > 1;
        for (int j = 0; j < c->size; ++j) {
            ExerciseEntry *e = &c->exercises[j];
            id btn = statusView_init(&sv, e->header, e->title, (int)((i << 8) | j), self, btnTap);
            sv->entry = e;
            if (addHint) {
                CFStringRef exerciseHint = formatStr(locale, exerciseProgressFmt, j + 1, c->size);
                setAccessibilityHint(sv->button, exerciseHint);
                CFRelease(exerciseHint);
            }
            exerciseView_configure(sv);
            addArrangedSubview(cv->stack, btn);
            releaseV(btn);
        }
    }
    CFRelease(locale);

    setupHierarchy(self, stack, createScrollView(), ColorPrimaryBGGrouped);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), self, restartTimers,
                                    UIApplicationDidBecomeActiveNotification, NULL,
                                    CFNotificationSuspensionBehaviorDrop);
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *d = (WorkoutVC *)getIVVC(self);
    if (!getTag(btn)) {
        CFStringRef str = localize(CFSTR("end"));
        setBtnTitle(btn, str);
        CFRelease(str);
        setTitleColor(btn, getColor(ColorRed), UIControlStateNormal);
        setTag(btn, 1);
        d->workout->startTime = time(NULL);
        circuit_start(d->workout->group, d->timers, true);
        CFArrayRef views = getArrangedSubviews(d->first->stack);
        StatusView *sv = (StatusView *)getIVV(CFArrayGetValueAtIndex(views, 0));
        exerciseView_configure(sv);
        id nextView = d->workout->group->header ? d->first->header : sv->button;
        UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
    } else {
        cleanupWorkoutNotifications(self, d);
        if (workout_isCompleted(d->workout)) {
            handleFinishedWorkout(self, d, true);
        } else {
            addWorkoutData(self, d->workout, UCHAR_MAX, NULL, true);
        }
    }
}

void workoutVC_willDisappear(id self, SEL _cmd, bool animated) {
    (((void(*)(struct objc_super*,SEL,bool))objc_msgSendSuper)
     (&(struct objc_super){self, VC}, _cmd, animated));

    if (msg0(bool, self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *d = (WorkoutVC *)getIVVC(self);
        if (!d->done) {
            cleanupWorkoutNotifications(self, d);
            if (d->workout->startTime) {
                if (workout_isCompleted(d->workout)) {
                    handleFinishedWorkout(self, d, false);
                } else {
                    addWorkoutData(self, d->workout, UCHAR_MAX, NULL, false);
                }
            }
        }
    }
}

#pragma mark - Event Handling

void workoutVC_handleTap(id self, SEL _cmd _U_, id btn) {
    int tag = (int)getTag(btn);
    int groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & UCHAR_MAX;
    handleEvent(self, (WorkoutVC *)getIVVC(self), groupIdx, exerciseIdx, 0);
}

void handleEvent(id self, WorkoutVC *d, int gIdx, int eIdx, int event) {
    Workout *w = d->workout;
    if (gIdx != w->index || (eIdx != w->group->index && event != EventFinishGroup)) return;

    CFArrayRef views = getArrangedSubviews(d->first->stack);
    StatusView *v = (StatusView *)getIVV(CFArrayGetValueAtIndex(views, eIdx));
    id next = nil;
    bool exerciseDone;
    int transition = 0;
    switch (event) {
        case EventFinishGroup:
            transition = workout_increment(w, d->timers);
            break;

        case EventFinishExercise:
            msg1(void, bool, v->button, sel_getUid("setUserInteractionEnabled:"), true);
            if (w->type == WorkoutEndurance) {
                setTextColor(v->header, getColor(ColorGreen));
                CFStringRef msg = localize(CFSTR("exerciseDurationMessage"));
                setText(v->header, msg);
                CFRelease(msg);
                statusView_updateAccessibility(v);
                next = v->button;
                break;
            }
        default:
            exerciseDone = exerciseEntry_cycle(v->entry, d->timers);
            exerciseView_configure(v);
            if (exerciseDone) {
                transition = circuit_increment(w->group, d->timers, v->entry);
                if (transition == TransitionFinishedCircuitDeleteFirst)
                    transition = workout_increment(w, d->timers);
            }
            break;
    }

    switch (transition) {
        case TransitionCompletedWorkout:
            cleanupWorkoutNotifications(self, d);
            workout_setDuration(w);
            if (UIAccessibilityIsVoiceOverRunning()) {
                CFStringRef msg = localize(CFSTR("workoutCompleteMsg"));
                dispatch_after(dispatch_time(0, 4000000000), dispatch_get_main_queue(), ^{
                    UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)msg);
                    CFRelease(msg);
                });
            }
            handleFinishedWorkout(self, d, true);
            return;

        case TransitionFinishedCircuitDeleteFirst:
            d->first = d->containers[w->index].data;
            views = getArrangedSubviews(d->first->stack);
            removeFromSuperview(d->containers[w->index - 1].view);
            setHidden(d->first->divider, true);
            next = d->first->header;
        case TransitionFinishedCircuit:
            if (w->group->reps > 1 && w->group->type == CircuitRounds) {
                CFLocaleRef locale = CFLocaleCopyCurrent();
                CFStringRef newNumber = formatStr(locale, CFSTR("%d"), w->group->completedReps + 1);
                CFRelease(locale);
                updateRange(w->group->header, &w->group->range, newNumber);
                setText(d->first->header, w->group->header);
                next = d->first->header;
            }

            for (int i = 0; i < w->group->size; ++i) {
                v = (StatusView *)getIVV(CFArrayGetValueAtIndex(views, i));
                setEnabled(v->button, true);
                exerciseView_configure(v);
            }
            if (!next) next = ((StatusView *)getIVV(CFArrayGetValueAtIndex(views, 0)))->button;
            break;

        case TransitionFinishedExercise:
            v = (StatusView *)getIVV(CFArrayGetValueAtIndex(views, w->group->index));
            exerciseView_configure(v);
            next = v->button;
            break;

        default:
            if (w->testMax) {
                presentModalVC(self, updateMaxesVC_init(self, eIdx));
                return;
            }
            break;
    }
    if (next) UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, next);
}

#pragma mark - Notifications

void restartTimers(CFNotificationCenterRef ctr _U_, void *self,
                   CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    WorkoutVC *d = (WorkoutVC *)getIVVC(self);
    if (d->done) return;

    time_t now = time(NULL);
    for (int i = 1; i >= 0; --i) {
        WorkoutTimer *t = &d->timers[i];
        if (t->active) {
            int diff = (int)(now - t->refTime);
            if (diff >= t->duration) {
                t->active = 0;
                handleEvent(self, d, t->container, t->exercise, i + 1);
            } else {
                workoutTimer_start(t, t->duration - diff, false);
            }
        }
    }
}

void workoutVC_receivedNotif(id self, int identifier) {
    WorkoutVC *d = (WorkoutVC *)getIVVC(self);
    int type = identifier % 2;
    WorkoutTimer *timer = &d->timers[type];
    if (d->done || !timer->active || timer->identifier != identifier) return;

    timer->active = 0;
    if (type == TimerGroup && d->timers[TimerEx].active) {
        d->timers[TimerEx].active = 0;
        CFStringRef exId = formatStr(NULL, CFSTR("%d"), d->timers[TimerEx].identifier);
        CFArrayRef arr = CFArrayCreate(NULL, (const void *[]){exId}, 1, &kCFTypeArrayCallBacks);
        CFRelease(exId);
        id uc = clsF0(id, objc_getClass("UNUserNotificationCenter"), sel_getUid("currentNotificationCenter"));
        msg1(void, CFArrayRef, uc,
             sel_getUid("removePendingNotificationRequestsWithIdentifiers:"), arr);
        CFRelease(arr);
    }

    handleEvent(self, d, timer->container, timer->exercise, type + 1);
}

#pragma mark - Modal Delegate

void workoutVC_finishedBottomSheet(id self, int index, short weight) {
    WorkoutVC *d = (WorkoutVC *)getIVVC(self);
    d->weights[index] = weight;
    handleEvent(self, d, 0, index, 0);
}
