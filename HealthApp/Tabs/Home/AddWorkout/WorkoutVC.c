#include "WorkoutVC.h"
#include "AppDelegate.h"
#include "StatusView.h"
#include "UpdateMaxesVC.h"
#include "Views.h"

extern CFStringRef UIApplicationDidBecomeActiveNotification;
extern CFStringRef UIApplicationWillResignActiveNotification;
extern uint32_t UIAccessibilityLayoutChangedNotification;

Class WorkoutVCClass;

enum {
    ExerciseStateDisabled, ExerciseStateActive, ExerciseStateResting, ExerciseStateCompleted
};

enum {
    SignalGroup = SIGUSR2,
    SignalExercise = SIGUSR1
};

enum {
    TimerGroup,
    TimerExercise
};

enum {
    EventFinishGroup = 1,
    EventFinishExercise
};

enum {
    TransitionCompletedWorkout = 1,
    TransitionFinishedCircuitDeleteFirst,
    TransitionFinishedCircuit,
    TransitionFinishedExercise,
};

struct WorkoutData {
    const Class Center, Content, Trigger, Request, Sound;
    const SEL dc, ds, newTri, newReq, addReq, btnEn;
    const SEL sct[3];
    id (*getCenter)(Class,SEL);
    id (*getSound)(Class,SEL);
    void (*setInteraction)(id,SEL,bool);
    void (*setTitle)(id,SEL,CFStringRef);
    void (*setBody)(id,SEL,CFStringRef);
    void (*setSound)(id,SEL,id);
    id (*createTrigger)(Class,SEL,double,bool);
    id (*createReq)(Class,SEL,CFStringRef,id,id);
    void (*addToCenter)(id,SEL,id,void(^)(id));
};

static CFStringRef notificationMessages[2];
static CFStringRef notifTitle;
static CFStringRef exerciseProgressFmt;
static pthread_t *exerciseTimerThread;
static pthread_mutex_t timerLock;
static struct WorkoutData wkData;

static void handleEvent(WorkoutVC *, int, int, int);
static void stopTimers(CFNotificationCenterRef, void *,
                       CFNotificationName, const void *, CFDictionaryRef);
static void restartTimers(CFNotificationCenterRef, void *,
                          CFNotificationName, const void *, CFDictionaryRef);
static void scheduleNotification(unsigned, int);

void initWorkoutStrings(void) {
    fillStringArray(notificationMessages, CFSTR("notifications%d"), 2);
    notifTitle = localize(CFSTR("workoutNotificationTitle"));
    exerciseProgressFmt = localize(CFSTR("exerciseProgressHint"));
    SEL btnEn = sel_getUid("setUserInteractionEnabled:");
    Class Center = objc_getClass("UNUserNotificationCenter");
    Class Content = objc_getClass("UNMutableNotificationContent");
    Class Trigger = objc_getClass("UNTimeIntervalNotificationTrigger");
    Class Req = objc_getClass("UNNotificationRequest"), Sound = objc_getClass("UNNotificationSound");
    SEL ds = sel_getUid("defaultSound"), dc = sel_getUid("currentNotificationCenter");
    SEL newTri = sel_getUid("triggerWithTimeInterval:repeats:");
    SEL newReq = sel_getUid("requestWithIdentifier:content:trigger:");
    SEL addReq = sel_getUid("addNotificationRequest:withCompletionHandler:");
    SEL sct[] = {sel_getUid("setTitle:"), sel_getUid("setBody:"), sel_getUid("setSound:")};
    memcpy(&wkData, &(struct WorkoutData){Center, Content, Trigger, Req, Sound, dc, ds, newTri,
        newReq, addReq, btnEn, {sct[0], sct[1], sct[2]}, (id(*)(Class,SEL))getImpC(Center, dc),
        (id(*)(Class,SEL))getImpC(Sound, ds), (void(*)(id,SEL,bool))getImpO(View, btnEn),
        (void(*)(id,SEL,CFStringRef))getImpO(Content, sct[0]),
        (void(*)(id,SEL,CFStringRef))getImpO(Content, sct[1]),
        (void(*)(id,SEL,id))getImpO(Content, sct[2]),
        (id(*)(Class,SEL,double,bool))getImpC(Trigger, newTri),
        (id(*)(Class,SEL,CFStringRef,id,id))getImpC(Req, newReq),
        (void(*)(id,SEL,id,void(^)(id)))getImpO(Center, addReq)
    }, sizeof(struct WorkoutData));
}

#pragma mark - Timers

static void handle_exercise_timer_interrupt(int n _U_) {}

static void handle_group_timer_interrupt(int n _U_) {}

static void *timer_loop(void *arg) {
    WorkoutTimer *t = arg;
    unsigned res = 0, duration = 0;
    int container = -1, exercise = -1, eventType = 0;
    while (!t->info.stop) {
        pthread_mutex_lock(&t->lock);
        while (t->info.active != 1)
            pthread_cond_wait(&t->cond, &t->lock);
        duration = t->duration;
        container = t->container;
        exercise = t->exercise;
        eventType = t->info.type ? EventFinishExercise : EventFinishGroup;
        pthread_mutex_unlock(&t->lock);

        if (!duration) continue;
        res = sleep(duration);
        t->info.active = 2;
        if (!res) {
            void *parent = t->parent;
            if (t->info.type == 0 && t[1].info.active == 1)
                pthread_kill(*exerciseTimerThread, SignalExercise);
            dispatch_async(dispatch_get_main_queue(), ^{
                handleEvent(parent, container, exercise, eventType);
            });
        }
    }
    return NULL;
}

static void startWorkoutTimer(WorkoutTimer *t, unsigned duration) {
    pthread_mutex_lock(&t->lock);
    t->refTime = time(NULL);
    t->duration = duration;
    t->info.stop = !duration;
    t->info.active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

#pragma mark - View Configuration

static bool cycleExerciseEntry(ExerciseEntry *e, WorkoutTimer *timers) {
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) {
                startWorkoutTimer(&timers[TimerExercise], (unsigned)e->reps);
                scheduleNotification((unsigned)e->reps, TimerExercise);
            }
            break;

        case ExerciseStateActive:
            if (e->rest) {
                e->state = ExerciseStateResting;
                break;
            }

        case ExerciseStateResting:
            if (++e->completedSets == e->sets) {
                e->state = ExerciseStateCompleted;
                ++timers[TimerExercise].exercise;
                return true;
            } else {
                e->state = ExerciseStateActive;
                CFLocaleRef l = CFLocaleCopyCurrent();
                CFStringRef sets = formatStr(l, CFSTR("%d"), e->completedSets + 1);
                CFRelease(l);
                CFStringReplace(e->header, e->hRange, sets);
                e->hRange.length = CFStringGetLength(sets);
                CFRelease(sets);
                if (e->type == ExerciseDuration) {
                    startWorkoutTimer(&timers[TimerExercise], (unsigned)e->reps);
                    scheduleNotification((unsigned)e->reps, TimerExercise);
                }
            }
        default:
            break;
    }
    return false;
}

static void startGroup(Circuit *c, WorkoutTimer *timers, bool startTimer) {
    timers[TimerExercise].exercise = c->index = 0;

    for (ExerciseEntry *e = c->exercises; e < &c->exercises[c->size]; ++e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer) {
        unsigned duration = (unsigned)(60 * c->reps);
        startWorkoutTimer(&timers[TimerGroup], duration);
        scheduleNotification(duration, TimerGroup);
    }

    cycleExerciseEntry(c->exercises, timers);
}

static void exerciseView_configure(StatusView *v, VCacheRef tbl, CCacheRef clr) {
    ExerciseEntry *e = v->entry;

    if (e->state == ExerciseStateResting) {
        tbl->button.setTitle(v->button, tbl->button.sbtxt, e->rest, 0);
    } else {
        tbl->button.setTitle(v->button, tbl->button.sbtxt, e->title, 0);
    }

    switch (e->state) {
        case ExerciseStateDisabled:
            tbl->view.setBG(v->box, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorGray));
            tbl->button.setEnabled(v->button, tbl->button.en, false);
            break;
        case ExerciseStateActive:
            tbl->label.setText(v->header, tbl->label.stxt, e->header);
            if (e->type == ExerciseDuration)
                wkData.setInteraction(v->button, wkData.btnEn, false);
        case ExerciseStateResting:
            tbl->button.setEnabled(v->button, tbl->button.en, true);
            tbl->view.setBG(v->box, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorOrange));
            break;
        case ExerciseStateCompleted:
            tbl->button.setEnabled(v->button, tbl->button.en, false);
            tbl->view.setBG(v->box, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorGreen));
    }
    statusView_updateAccessibility(v, tbl);
}

static bool didFinishCircuit(Circuit *c) {
    if (c->type == CircuitRounds) {
        return ++c->completedReps == c->reps;
    } else if (c->type == CircuitDecrement) {
        if (--c->completedReps == 0) return true;

        CFLocaleRef l = CFLocaleCopyCurrent();
        CFStringRef reps = formatStr(l, CFSTR("%d"), c->completedReps);
        CFRelease(l);
        long len = CFStringGetLength(reps);
        ExerciseEntry *end = &c->exercises[c->size];
        for (ExerciseEntry *e = c->exercises; e < end; ++e) {
            if (e->type == ExerciseReps) {
                CFStringReplace(e->title, e->tRange, reps);
                e->tRange.length = len;
            }
        }
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
                timers[0].container = timers[1].container = w->index;
                startGroup(++w->group, timers, true);
            }
        } else {
            startGroup(w->group, timers, false);
        }
    } else {
        cycleExerciseEntry(v->entry + 1, timers);
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
        ExerciseEntry *e = &group->exercises[groupIndex];
        return e->state == ExerciseStateResting && e->completedSets == e->sets - 1;
    }
    return false;
}

#pragma mark - VC init/free

id workoutVC_init(Workout *workout, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(WorkoutVCClass, Sels.nw);
    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
    d->tbl = tbl;
    d->clr = clr;
    d->workout = workout;
    d->containers = malloc((unsigned)workout->size * sizeof(CVPair));

    pthread_mutex_init(&timerLock, NULL);
    for (uint8_t i = 0; i < 2; ++i) {
        memcpy(&d->timers[i], &(WorkoutTimer){.info = {.type = i}}, sizeof(WorkoutTimer));
        pthread_mutex_init(&d->timers[i].lock, NULL);
        pthread_cond_init(&d->timers[i].cond, NULL);
    }

    struct sigaction sa = {.sa_flags = 0, .sa_handler = handle_exercise_timer_interrupt};
    sigemptyset(&sa.sa_mask);
    sigaction(SignalExercise, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SignalGroup, &sa, NULL);
    exerciseTimerThread = &d->threads[TimerExercise];
    d->timers[0].parent = d->timers[1].parent = d;
    pthread_create(&d->threads[1], NULL, timer_loop, &d->timers[1]);
    pthread_create(&d->threads[0], NULL, timer_loop, &d->timers[0]);
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);

    startWorkoutTimer(&d->timers[TimerGroup], 0);
    startWorkoutTimer(&d->timers[TimerExercise], 0);
    pthread_join(d->threads[1], NULL);
    pthread_join(d->threads[0], NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&d->timers[i].cond);
        pthread_mutex_destroy(&d->timers[i].lock);
    }
    pthread_mutex_destroy(&timerLock);
    exerciseTimerThread = NULL;
    int size = d->workout->size;
    Circuit *cEnd = &d->workout->activities[size];
    for (Circuit *c = &d->workout->activities[0]; c < cEnd; ++c) {
        if (c->header) CFRelease(c->header);
        for (ExerciseEntry *e = &c->exercises[0]; e < &c->exercises[c->size]; ++e) {
            CFRelease(e->title);
            if (e->rest) CFRelease(e->rest);
            if (e->header) CFRelease(e->header);
        }
        free(c->exercises);
    }
    free(d->workout->activities);
    free(d->workout);

    for (int i = 0; i < size; ++i) {
        Sels.viewRel(d->containers[i].view, Sels.rel);
    }
    free(d->containers);
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);
}

static void handleFinishedWorkout(WorkoutVC *d, bool pop) {
    int *lifts = NULL;
    if (d->weights[3]) {
        lifts = malloc(sizeof(int) << 2);
        memcpy(lifts, d->weights, sizeof(int) << 2);
        d->workout->duration = max(d->workout->duration, MinWorkoutDuration);
    }
    addWorkoutData(d->workout, d->workout->day, lifts, pop);
}

static void cleanupWorkoutNotifications(WorkoutVC *d) {
    d->done = true;
    if (d->timers[TimerGroup].info.active == 1)
        pthread_kill(d->threads[TimerGroup], SignalGroup);
    if (d->timers[TimerExercise].info.active == 1)
        pthread_kill(d->threads[TimerExercise], SignalExercise);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), (char *)d - VCSize);
    id center = wkData.getCenter(wkData.Center, wkData.dc);
    msg0(void, center, sel_getUid("removeAllPendingNotificationRequests"));
    msg0(void, center, sel_getUid("removeAllDeliveredNotifications"));
}

#pragma mark - Lifecycle

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg,
                    d->clr->getColor(d->clr->cls, d->clr->sc, ColorPrimaryBGGrouped));
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    setVCTitle(navItem, d->workout->title);

    id stack = createVStack(NULL, 0);
    tbl->stack.setSpace(stack, tbl->stack.ssp, 20);
    tbl->stack.setMargins(stack, tbl->stack.smr, (HAInsets){16, 8, 16, 8});
    id startBtn = createButton(tbl, d->clr, localize(CFSTR("start")), ColorGreen,
                               UIFontTextStyleBody, self, sel_getUid("startEndWorkout:"));
    setNavButtons(navItem, (id []){nil, startBtn});

    SEL btnTap = sel_getUid("buttonTapped:");
    ContainerView *cv;
    StatusView *sv; SVArgs args = {d->tbl, d->clr, &sv};
    CFLocaleRef l = CFLocaleCopyCurrent();
    for (int i = 0; i < d->workout->size; ++i) {
        Circuit *c = &d->workout->activities[i];
        d->containers[i].view = containerView_init(tbl, d->clr, &cv, c->header);
        d->containers[i].data = cv;
        if (!i) {
            d->first = cv;
            tbl->view.hide(cv->divider, tbl->view.shd, true);
        }
        tbl->stack.addSub(stack, tbl->stack.asv, d->containers[i].view);

        bool addHint = c->size > 1;
        for (int j = 0; j < c->size; ++j) {
            ExerciseEntry *e = &c->exercises[j];
            id v = statusView_init(&args, e->header, e->title, (int)((i << 8) | j), self, btnTap);
            sv->entry = e;
            if (addHint) {
                CFStringRef exerciseHint = formatStr(l, exerciseProgressFmt, j + 1, c->size);
                tbl->view.setHint(sv->button, tbl->view.shn, exerciseHint);
                CFRelease(exerciseHint);
            }
            exerciseView_configure(sv, tbl, d->clr);
            tbl->stack.addSub(cv->stack, tbl->stack.asv, v);
            Sels.viewRel(v, Sels.rel);
        }
    }
    CFRelease(l);

    id scrollView = createScrollView();
    addVStackToScrollView(tbl, view, stack, scrollView);
    Sels.viewRel(scrollView, Sels.rel);
    Sels.viewRel(stack, Sels.rel);

    CFNotificationCenterRef c = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(c, self, restartTimers, UIApplicationDidBecomeActiveNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(c, self, stopTimers, UIApplicationWillResignActiveNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;
    if (!tbl->view.getTag(btn, tbl->view.gtg)) {
        CFStringRef str = localize(CFSTR("end"));
        tbl->button.setTitle(btn, tbl->button.sbtxt, str, 0);
        CFRelease(str);
        tbl->button.setColor(btn, tbl->button.sbc,
                             d->clr->getColor(d->clr->cls, d->clr->sc, ColorRed), 0);
        tbl->view.setTag(btn, tbl->view.stg, 1);
        d->workout->startTime = time(NULL);
        startGroup(d->workout->group, d->timers, true);
        CFArrayRef views = tbl->stack.getSub(d->first->stack, tbl->stack.gsv);
        StatusView *sv = (StatusView *)((char *)CFArrayGetValueAtIndex(views, 0) + ViewSize);
        exerciseView_configure(sv, tbl, d->clr);
        id nextView = d->workout->group->header ? d->first->header : sv->button;
        UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
    } else {
        pthread_mutex_lock(&timerLock);
        if (d->done) {
            pthread_mutex_unlock(&timerLock);
            return;
        }
        cleanupWorkoutNotifications(d);
        pthread_mutex_unlock(&timerLock);
        if (isCompleted(d->workout)) {
            handleFinishedWorkout(d, true);
        } else {
            addWorkoutData(d->workout, UCHAR_MAX, NULL, true);
        }
    }
}

void workoutVC_willDisappear(id self, SEL _cmd, bool animated) {
    msgSup1(void, bool, (&(struct objc_super){self, VC}), _cmd, animated);

    if (msg0(bool, self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
        if (!d->done) {
            cleanupWorkoutNotifications(d);
            if (d->workout->startTime) {
                if (isCompleted(d->workout)) {
                    handleFinishedWorkout(d, false);
                } else {
                    addWorkoutData(d->workout, UCHAR_MAX, NULL, false);
                }
            }
        }
    }
}

#pragma mark - Event Handling

void workoutVC_handleTap(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
    int tag = (int)d->tbl->view.getTag(btn, d->tbl->view.gtg);
    int groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & 0xff;
    handleEvent(d, groupIdx, exerciseIdx, 0);
}

void handleEvent(WorkoutVC *d, int gIdx, int eIdx, int event) {
    pthread_mutex_lock(&timerLock);
    Workout *w = d->workout;
    VCacheRef tbl = d->tbl;
    if (d->done || gIdx != w->index || (eIdx != w->group->index && event != EventFinishGroup)) {
        pthread_mutex_unlock(&timerLock);
        return;
    }

    CFArrayRef views = tbl->stack.getSub(d->first->stack, tbl->stack.gsv);
    StatusView *v = (StatusView *)((char *)CFArrayGetValueAtIndex(views, eIdx) + ViewSize);
    id nextView = nil;
    bool exerciseDone;
    int t = 0;
    switch (event) {
        case EventFinishGroup:
            t = TransitionFinishedCircuitDeleteFirst;
            if (++w->index == w->size) {
                t = TransitionCompletedWorkout;
                goto foundTransition;
            }
            ++w->group;
            d->timers[0].container = d->timers[1].container = w->index;
            startGroup(w->group, d->timers, true);
            goto foundTransition;

        case EventFinishExercise:
            wkData.setInteraction(v->button, wkData.btnEn, true);
            if (w->type == WorkoutEndurance) {
                tbl->label.setColor(v->header, tbl->label.stc,
                                    d->clr->getColor(d->clr->cls, d->clr->sc, ColorGreen));
                CFStringRef msg = localize(CFSTR("exerciseDurationMessage"));
                tbl->label.setText(v->header, tbl->label.stxt, msg);
                CFRelease(msg);
                statusView_updateAccessibility(v, tbl);
                nextView = v->button;
                goto foundTransition;
            }
        default:
            exerciseDone = cycleExerciseEntry(v->entry, d->timers);
            exerciseView_configure(v, tbl, d->clr);
            if (exerciseDone) t = findTransition(w, d->timers, v);
    }
foundTransition:

    switch (t) {
        case TransitionCompletedWorkout:
            cleanupWorkoutNotifications(d);
            setDuration(w);
            pthread_mutex_unlock(&timerLock);
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
            d->first = d->containers[w->index].data;
            views = tbl->stack.getSub(d->first->stack, tbl->stack.gsv);
            tbl->view.rmSub(d->containers[w->index - 1].view, tbl->view.rsv);
            tbl->view.hide(d->first->divider, tbl->view.shd, true);
            nextView = d->first->header;

        case TransitionFinishedCircuit:
            if (w->group->reps > 1 && w->group->type == CircuitRounds) {
                CFLocaleRef l = CFLocaleCopyCurrent();
                CFStringRef newNumber = formatStr(l, CFSTR("%d"), w->group->completedReps + 1);
                CFRelease(l);
                CFStringReplace(w->group->header, w->group->range, newNumber);
                w->group->range.length = CFStringGetLength(newNumber);
                CFRelease(newNumber);
                tbl->label.setText(d->first->header, tbl->label.stxt, w->group->header);
                nextView = d->first->header;
            }

            for (int i = 0; i < w->group->size; ++i) {
                v = (StatusView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize);
                tbl->button.setEnabled(v->button, tbl->button.en, true);
                exerciseView_configure(v, tbl, d->clr);
            }
            if (!nextView)
                nextView = ((StatusView *)((char *)CFArrayGetValueAtIndex(views, 0) + ViewSize))->button;
            break;

        case TransitionFinishedExercise:
            v = (StatusView *)((char *)CFArrayGetValueAtIndex(views, w->group->index) + ViewSize);
            exerciseView_configure(v, tbl, d->clr);
            nextView = v->button;
            break;

        default:
            if (w->testMax) {
                pthread_mutex_unlock(&timerLock);
                presentModalVC(updateMaxesVC_init(d, eIdx, w->bodyweight, tbl, d->clr));
                return;
            }
            break;
    }
    pthread_mutex_unlock(&timerLock);
    if (nextView) UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
}

#pragma mark - Notifications

void stopTimers(CFNotificationCenterRef ctr _U_, void *self,
                CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    if (d->timers[TimerGroup].info.active == 1) {
        d->savedInfo.groupTag = d->timers[TimerGroup].container;
        pthread_kill(d->threads[TimerGroup], SignalGroup);
    } else {
        d->savedInfo.groupTag = -1;
    }

    if (d->timers[TimerExercise].info.active == 1) {
        d->savedInfo.exerciseInfo.group = d->timers[TimerExercise].container;
        d->savedInfo.exerciseInfo.tag = d->timers[TimerExercise].exercise;
        pthread_kill(d->threads[TimerExercise], SignalExercise);
    } else {
        d->savedInfo.exerciseInfo.group = -1;
    }
    pthread_mutex_unlock(&timerLock);
}

void restartTimers(CFNotificationCenterRef ctr _U_, void *self,
                   CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    bool endExercise = false, endGroup = false;
    int groupIdx = 0, exerciseIdx = 0;
    WorkoutVC *d = (WorkoutVC *)((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    if (d->done) {
        pthread_mutex_unlock(&timerLock);
        return;
    }
    time_t now = time(NULL);
    int group = d->savedInfo.exerciseInfo.group;
    exerciseIdx = d->savedInfo.exerciseInfo.tag;
    if (group >= 0) {
        unsigned diff = (unsigned)(now - d->timers[TimerExercise].refTime);
        if (diff >= d->timers[TimerExercise].duration) {
            endExercise = true;
            groupIdx = group;
        } else {
            startWorkoutTimer(&d->timers[TimerExercise], d->timers[TimerExercise].duration - diff);
        }
    }

    group = d->savedInfo.groupTag;
    if (group >= 0) {
        unsigned diff = (unsigned)(now - d->timers[TimerGroup].refTime);
        if (diff >= d->timers[TimerGroup].duration) {
            endGroup = true;
            groupIdx = group;
        } else {
            startWorkoutTimer(&d->timers[TimerGroup], d->timers[TimerGroup].duration - diff);
        }
    }
    pthread_mutex_unlock(&timerLock);

    if (endExercise) handleEvent(d, groupIdx, exerciseIdx, EventFinishExercise);
    if (endGroup) handleEvent(d, groupIdx, 0, EventFinishGroup);
}

static void scheduleNotification(unsigned secondsFromNow, int type) {
    static int identifier = 0;
    int currentId = identifier++;
    CFStringRef idString = formatStr(NULL, CFSTR("%d"), currentId);
    id content = Sels.new(wkData.Content, Sels.nw);
    wkData.setTitle(content, wkData.sct[0], notifTitle);
    wkData.setBody(content, wkData.sct[1], notificationMessages[type]);
    wkData.setSound(content, wkData.sct[2], wkData.getSound(wkData.Sound, wkData.ds));
    id trigger = wkData.createTrigger(wkData.Trigger, wkData.newTri, secondsFromNow, false);
    id req = wkData.createReq(wkData.Request, wkData.newReq, idString, content, trigger);
    wkData.addToCenter(wkData.getCenter(wkData.Center, wkData.dc), wkData.addReq, req, ^(id e _U_){});
    CFRelease(idString);
    Sels.objRel(content, Sels.rel);
}

#pragma mark - Modal Delegate

void workoutVC_finishedBottomSheet(void *self, int index, int weight) {
    WorkoutVC *d = (WorkoutVC *)self;
    d->weights[index] = weight;
    handleEvent(d, 0, index, 0);
}
