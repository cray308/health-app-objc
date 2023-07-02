#include "WorkoutVC.h"
#include "AppDelegate.h"
#include "StatusView.h"
#include "UpdateMaxesVC.h"
#include "Views.h"

extern CFStringRef UIApplicationDidBecomeActiveNotification;
extern uint32_t UIAccessibilityLayoutChangedNotification;

Class WorkoutVCClass;

enum {
    ExerciseStateDisabled,
    ExerciseStateActive,
    ExerciseStateActiveMult,
    ExerciseStateResting,
    ExerciseStateCompleted
};

enum {
    TimerCircuit,
    TimerExercise
};

enum {
    EventFinishCircuit = 1,
    EventFinishExercise
};

enum {
    TransitionCompletedWorkout = 1,
    TransitionFinishedCircuitDeleteFirst,
    TransitionFinishedCircuit,
    TransitionFinishedExercise,
};

struct WorkoutData {
    const Class Content, Trigger, Request, Sound;
    const SEL ds, newTri, newReq;
    const SEL sct[2];
    id (*getSound)(Class,SEL);
    void (*sTitle)(id,SEL,CFStringRef);
    void (*setBody)(id,SEL,CFStringRef);
    void (*setSound)(id,SEL,id);
    id (*createTrigger)(Class,SEL,double,bool);
    id (*createReq)(Class,SEL,CFStringRef,id,id);
};

static CFStringRef notificationMessages[2];
static CFStringRef notifTitle;
static struct WorkoutData wkData;

static void handleEvent(id, WorkoutVC *, int, int, int);
static void restartTimers(CFNotificationCenterRef, void *,
                          CFStringRef, const void *, CFDictionaryRef);

void initWorkoutData(int week) {
    initExerciseData(week);
    fillStringArray(notificationMessages, CFSTR("notifications%d"), 2);
    notifTitle = localize(CFSTR("workoutNotificationTitle"));
    Class Content = objc_getClass("UNMutableNotificationContent");
    Class Trigger = objc_getClass("UNTimeIntervalNotificationTrigger");
    Class Req = objc_getClass("UNNotificationRequest"), Sound = objc_getClass("UNNotificationSound");
    SEL ds = sel_getUid("defaultSound");
    SEL newTri = sel_getUid("triggerWithTimeInterval:repeats:");
    SEL newReq = sel_getUid("requestWithIdentifier:content:trigger:");
    SEL sct[] = {sel_getUid("setBody:"), sel_getUid("setSound:")};
    memcpy(&wkData, &(struct WorkoutData){Content, Trigger, Req, Sound, ds, newTri,
        newReq, {sct[0], sct[1]},
        (id(*)(Class,SEL))getClassMethodImp(Sound, ds),
        (void(*)(id,SEL,CFStringRef))class_getMethodImplementation(Content, SetTitleSel),
        (void(*)(id,SEL,CFStringRef))class_getMethodImplementation(Content, sct[0]),
        (void(*)(id,SEL,id))class_getMethodImplementation(Content, sct[1]),
        (id(*)(Class,SEL,double,bool))getClassMethodImp(Trigger, newTri),
        (id(*)(Class,SEL,CFStringRef,id,id))getClassMethodImp(Req, newReq),
    }, sizeof(struct WorkoutData));
}

#pragma mark - Timers

static void workoutTimer_start(WorkoutTimer *t, int duration, bool scheduleNotification) {
    static int identifiers[] = {0, 1};
    t->refTime = time(NULL);
    t->duration = duration;
    t->active = 1;
    if (!scheduleNotification) return;

    identifiers[t->type] += 2;
    t->identifier = identifiers[t->type];
    CFStringRef idString = formatStr(NULL, CFSTR("%d"), t->identifier);
    id content = new(wkData.Content);
    wkData.sTitle(content, SetTitleSel, notifTitle);
    wkData.setBody(content, wkData.sct[0], notificationMessages[t->type]);
    wkData.setSound(content, wkData.sct[1], wkData.getSound(wkData.Sound, wkData.ds));
    id trigger = wkData.createTrigger(wkData.Trigger, wkData.newTri, duration, false);
    id req = wkData.createReq(wkData.Request, wkData.newReq, idString, content, trigger);
    addNotificationRequest(getNotificationCenter(), req);
    CFRelease(idString);
    releaseObject(content);
}

#pragma mark - View Configuration

static bool cycleExerciseEntry(Exercise *e, WorkoutTimer *timers) {
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) {
                workoutTimer_start(&timers[TimerExercise], e->reps, true);
            }
            break;

        case ExerciseStateActive ... ExerciseStateActiveMult:
            if (e->rest) {
                e->state = ExerciseStateResting;
                break;
            }

        case ExerciseStateResting:
            if (++e->completedSets == e->sets) {
                e->state = ExerciseStateCompleted;
                ++timers[TimerExercise].row;
                return true;
            } else {
                e->state = ExerciseStateActiveMult;
                CFLocaleRef l = CFLocaleCopyCurrent();
                CFStringRef sets = formatStr(l, CFSTR("%d"), e->completedSets + 1);
                updateRange(e->header, &e->headerRange, sets, l);
                if (e->type == ExerciseDuration) {
                    workoutTimer_start(&timers[TimerExercise], e->reps, true);
                }
            }
        default:
            break;
    }
    return false;
}

static void startGroup(Circuit *c, WorkoutTimer *timers, bool startTimer) {
    timers[TimerExercise].row = c->index = 0;

    for (Exercise *e = c->exercises; e < &c->exercises[c->size]; ++e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer) {
        workoutTimer_start(&timers[TimerCircuit], 60 * c->reps, true);
    }

    cycleExerciseEntry(c->exercises, timers);
}

static void exerciseView_configure(StatusView *v) {
    Exercise *e = v->exercise;
    switch (e->state) {
        case ExerciseStateDisabled:
            setBackgroundColor(v->box, getColor(ColorGray));
            break;
        case ExerciseStateActive:
            setEnabled(v->button, true);
            setBackgroundColor(v->box, getColor(ColorOrange));
        case ExerciseStateActiveMult:
            if (e->type == ExerciseDuration) setUserInteractionEnabled(v->button, false);
            if (e->completedSets) {
                setTitle(v->button, e->title, ControlStateNormal);
                setText(v->header, e->header);
                statusView_updateAccessibility(v);
            }
            break;

        case ExerciseStateResting:
            setTitle(v->button, e->rest, ControlStateNormal);
            if (e->sets > 1) statusView_updateAccessibility(v);
            break;

        case ExerciseStateCompleted:
            setEnabled(v->button, false);
            setBackgroundColor(v->box, getColor(ColorGreen));
            if (e->rest) {
                setTitle(v->button, e->title, ControlStateNormal);
                if (e->sets > 1) statusView_updateAccessibility(v);
            }
    }
}

static bool didFinishCircuit(Circuit *c) {
    if (c->type == CircuitRounds) {
        return ++c->completedReps == c->reps;
    } else if (c->type == CircuitDecrement) {
        if (--c->completedReps == 0) return true;

        CFLocaleRef l = CFLocaleCopyCurrent();
        CFStringRef reps = formatStr(l, CFSTR("%d"), c->completedReps);
        Exercise *end = &c->exercises[c->size];
        for (Exercise *e = c->exercises; e < end; ++e) {
            if (e->type == ExerciseReps) {
                updateRange(e->title, &e->titleRange, CFRetain(reps), CFRetain(l));
            }
        }
        CFRelease(l);
        CFRelease(reps);
    }
    return false;
}

static int findTransition(Workout *w, WorkoutTimer *timers, StatusView *v) {
    int t = TransitionFinishedExercise;
    if (++w->group->index == w->group->size) {
        t = TransitionFinishedCircuit;
        if (didFinishCircuit(w->group)) {
            if (++w->index == w->size) {
                t = TransitionCompletedWorkout;
            } else {
                t = TransitionFinishedCircuitDeleteFirst;
                timers[0].section = timers[1].section = w->index;
                startGroup(++w->group, timers, true);
            }
        } else {
            startGroup(w->group, timers, false);
        }
    } else {
        cycleExerciseEntry(v->exercise + 1, timers);
    }
    return t;
}

static inline void setDuration(Workout *w) {
    w->duration = ((int)((time(NULL) - w->startTime) / 60.f)) + 1;
#if TARGET_OS_SIMULATOR
    w->duration *= 10;
#endif
}

static bool isCompleted(Workout *w) {
    setDuration(w);
    Circuit *group = w->group;
    int groupIndex = group->index;
    if (w->index != w->size - 1 || groupIndex != group->size - 1) return false;
    if (w->type == WorkoutEndurance) return w->duration >= (int)(group->exercises[0].reps / 60);

    if (group->type == CircuitRounds && group->completedReps == group->reps - 1) {
        Exercise *e = &group->exercises[groupIndex];
        return e->state == ExerciseStateResting && e->completedSets == e->sets - 1;
    }
    return false;
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
        startGroup(d->workout->group, d->timers, true);
        CFArrayRef views = getArrangedSubviews(d->containers[0].data->stack);
        StatusView *sv = getIVV(StatusView, CFArrayGetValueAtIndex(views, 0));
        exerciseView_configure(sv);
        id nextView = d->workout->circuits[0].header ? d->containers[0].data->header : sv->button;
        UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
    } else {
        cleanupNotifications(self, d);
        if (isCompleted(d->workout)) {
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
                if (isCompleted(d->workout)) {
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
    Workout *w = d->workout;
    CVPair *pair = &d->containers[section];
    CFArrayRef views = getArrangedSubviews(pair->data->stack);
    StatusView *v = getIVV(StatusView, CFArrayGetValueAtIndex(views, row));
    id nextView = nil;
    bool exerciseDone;
    int t = 0;
    switch (event) {
        case EventFinishCircuit:
            t = TransitionFinishedCircuitDeleteFirst;
            if (++w->index == w->size) {
                t = TransitionCompletedWorkout;
                goto foundTransition;
            }
            ++w->group;
            d->timers[0].section = d->timers[1].section = w->index;
            startGroup(w->group, d->timers, true);
            goto foundTransition;

        case EventFinishExercise:
            setUserInteractionEnabled(v->button, true);
            if (w->type == WorkoutEndurance) {
                setTextColor(v->header, getColor(ColorGreen));
                CFStringRef msg = localize(CFSTR("exerciseDurationMessage"));
                setText(v->header, msg);
                CFRelease(msg);
                statusView_updateAccessibility(v);
                nextView = v->button;
                goto foundTransition;
            }
        default:
            exerciseDone = cycleExerciseEntry(v->exercise, d->timers);
            exerciseView_configure(v);
            if (exerciseDone) t = findTransition(w, d->timers, v);
    }
foundTransition:

    switch (t) {
        case TransitionCompletedWorkout:
            cleanupNotifications(self, d);
            setDuration(w);
            if (UIAccessibilityIsVoiceOverRunning()) {
                CFStringRef msg = localize(CFSTR("workoutCompleteMsg"));
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 4000000000), dispatch_get_main_queue(), ^{
                    UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)msg);
                    CFRelease(msg);
                });
            }
            handleFinishedWorkout(d, true);
            return;

        case TransitionFinishedCircuitDeleteFirst:
            removeFromSuperview(pair->view);
            releaseView(pair->view);
            (pair++)->view = nil;
            views = getArrangedSubviews(pair->data->stack);
            setHidden(pair->data->divider, true);
            exerciseView_configure(getIVV(StatusView, CFArrayGetValueAtIndex(views, 0)));
            nextView = pair->data->header;
            break;

        case TransitionFinishedCircuit:
            if (w->group->reps > 1 && w->group->type == CircuitRounds) {
                CFLocaleRef locale = CFLocaleCopyCurrent();
                CFStringRef newNumber = formatStr(locale, CFSTR("%d"), w->group->completedReps + 1);
                updateRange(w->group->header, &w->group->range, newNumber, locale);
                setText(pair->data->header, w->group->header);
                nextView = pair->data->header;
            }

            bool refreshButtons = w->group->type == CircuitDecrement;
            for (int i = 0; i < w->group->size; ++i) {
                v = getIVV(StatusView, CFArrayGetValueAtIndex(views, i));
                if (refreshButtons && v->exercise->type == ExerciseReps) {
                    setEnabled(v->button, true);
                    setTitle(v->button,
                             v->exercise->title, ControlStateNormal | ControlStateDisabled);
                    setEnabled(v->button, false);
                }
                exerciseView_configure(v);
            }

            if (!nextView)
                nextView = getIVV(StatusView, CFArrayGetValueAtIndex(views, 0))->button;
            break;

        case TransitionFinishedExercise:
            v = getIVV(StatusView, CFArrayGetValueAtIndex(views, w->group->index));
            exerciseView_configure(v);
            nextView = v->button;
            break;

        default:
            if (w->testMax) {
                presentModalVC(self, updateMaxesVC_init(self, row));
                return;
            }
            break;
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
