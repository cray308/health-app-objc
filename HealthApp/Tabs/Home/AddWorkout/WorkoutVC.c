#include "WorkoutVC.h"
#include <CoreFoundation/CFNotificationCenter.h>
#include <dispatch/queue.h>
#include <signal.h>
#include "AppDelegate.h"
#include "StatusView.h"
#include "Views.h"

extern CFStringRef UIApplicationDidBecomeActiveNotification;
extern CFStringRef UIApplicationWillResignActiveNotification;
extern uint32_t UIAccessibilityLayoutChangedNotification;
id updateMaxesVC_init(void *parent, int index, short bodyweight, VCacheRef tbl, CCacheRef clr);
unsigned char addWorkoutData(unsigned char day, int type, int16_t duration, short *weights);
void homeVC_handleFinishedWorkout(id self, unsigned char completed);

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
    const Class NotifCenter, Content, Trigger, Request, Sound;
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

static void handleEvent(WorkoutVC *data, int gIdx, int eIdx, int event);

void initWorkoutStrings(CFBundleRef bundle) {
    fillStringArray(bundle, notificationMessages, CFSTR("notifications%d"), 2);
    notifTitle = localize(bundle, CFSTR("workoutNotificationTitle"));
    exerciseProgressFmt = localize(bundle, CFSTR("exerciseProgressHint"));
    SEL btnEn = sel_getUid("setUserInteractionEnabled:");
    Class NotifCenter = objc_getClass("UNUserNotificationCenter");
    Class Content = objc_getClass("UNMutableNotificationContent");
    Class Trigger = objc_getClass("UNTimeIntervalNotificationTrigger");
    Class Request = objc_getClass("UNNotificationRequest"), Sound = objc_getClass("UNNotificationSound");
    SEL ds = sel_getUid("defaultSound"), dc = sel_getUid("currentNotificationCenter");
    SEL newTri = sel_getUid("triggerWithTimeInterval:repeats:");
    SEL newReq = sel_getUid("requestWithIdentifier:content:trigger:");
    SEL addReq = sel_getUid("addNotificationRequest:withCompletionHandler:");
    SEL sct[] = {sel_getUid("setTitle:"), sel_getUid("setBody:"), sel_getUid("setSound:")};
    memcpy(&wkData, &(struct WorkoutData){NotifCenter, Content, Trigger, Request, Sound, dc, ds, newTri,
        newReq, addReq, btnEn, {sct[0], sct[1], sct[2]}, (id(*)(Class,SEL))getImpC(NotifCenter, dc),
        (id(*)(Class,SEL))getImpC(Sound, ds), (void(*)(id,SEL,bool))getImpO(View, btnEn),
        (void(*)(id,SEL,CFStringRef))getImpO(Content, sct[0]),
        (void(*)(id,SEL,CFStringRef))getImpO(Content, sct[1]),
        (void(*)(id,SEL,id))getImpO(Content, sct[2]),
        (id(*)(Class,SEL,double,bool))getImpC(Trigger, newTri),
        (id(*)(Class,SEL,CFStringRef,id,id))getImpC(Request, newReq),
        (void(*)(id,SEL,id,void(^)(id)))getImpO(NotifCenter, addReq)
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
            dispatch_async(dispatch_get_main_queue(), ^(void) {
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

static void stopTimers(CFNotificationCenterRef ctr _U_, void *self,
                       CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    if (data->timers[TimerGroup].info.active == 1) {
        data->savedInfo.groupTag = data->timers[TimerGroup].container;
        pthread_kill(data->threads[TimerGroup], SignalGroup);
    } else {
        data->savedInfo.groupTag = -1;
    }

    if (data->timers[TimerExercise].info.active == 1) {
        data->savedInfo.exerciseInfo.group = data->timers[TimerExercise].container;
        data->savedInfo.exerciseInfo.tag = data->timers[TimerExercise].exercise;
        pthread_kill(data->threads[TimerExercise], SignalExercise);
    } else {
        data->savedInfo.exerciseInfo.group = -1;
    }
    pthread_mutex_unlock(&timerLock);
}

static void restartTimers(CFNotificationCenterRef ctr _U_, void *self,
                          CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    bool endExercise = false, endGroup = false;
    int groupIdx = 0, exerciseIdx = 0;
    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    if (data->done) {
        pthread_mutex_unlock(&timerLock);
        return;
    }
    time_t now = time(NULL);
    int group = data->savedInfo.exerciseInfo.group;
    exerciseIdx = data->savedInfo.exerciseInfo.tag;
    if (group >= 0) {
        unsigned diff = (unsigned)(now - data->timers[TimerExercise].refTime);
        if (diff >= data->timers[TimerExercise].duration) {
            endExercise = true;
            groupIdx = group;
        } else {
            startWorkoutTimer(&data->timers[TimerExercise],
                              data->timers[TimerExercise].duration - diff);
        }
    }

    group = data->savedInfo.groupTag;
    if (group >= 0) {
        unsigned diff = (unsigned)(now - data->timers[TimerGroup].refTime);
        if (diff >= data->timers[TimerGroup].duration) {
            endGroup = true;
            groupIdx = group;
        } else {
            startWorkoutTimer(&data->timers[TimerGroup], data->timers[TimerGroup].duration - diff);
        }
    }
    pthread_mutex_unlock(&timerLock);

    if (endExercise)
        handleEvent(data, groupIdx, exerciseIdx, EventFinishExercise);
    if (endGroup)
        handleEvent(data, groupIdx, 0, EventFinishGroup);
}

#pragma mark - Notifications

static void scheduleNotification(unsigned secondsFromNow, int type) {
    static int identifier = 0;
    int currentId = identifier++;
    CFStringRef idString = formatStr(CFSTR("%d"), currentId);
    id content = Sels.new(wkData.Content, Sels.nw);
    wkData.setTitle(content, wkData.sct[0], notifTitle);
    wkData.setBody(content, wkData.sct[1], notificationMessages[type]);
    wkData.setSound(content, wkData.sct[2], wkData.getSound(wkData.Sound, wkData.ds));
    id trigger = wkData.createTrigger(wkData.Trigger, wkData.newTri, secondsFromNow, false);
    id req = wkData.createReq(wkData.Request, wkData.newReq, idString, content, trigger);
    wkData.addToCenter(wkData.getCenter(wkData.NotifCenter, wkData.dc), wkData.addReq, req, ^(id e _U_){});
    CFRelease(idString);
    Sels.objRel(content, Sels.rel);
}

static void cleanupWorkoutNotifications(WorkoutVC *data) {
    data->done = true;
    if (data->timers[TimerGroup].info.active == 1)
        pthread_kill(data->threads[TimerGroup], SignalGroup);
    if (data->timers[TimerExercise].info.active == 1)
        pthread_kill(data->threads[TimerExercise], SignalExercise);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), (char *)data - VCSize);
    id center = wkData.getCenter(wkData.NotifCenter, wkData.dc);
    msg0(void, center, sel_getUid("removeAllPendingNotificationRequests"));
    msg0(void, center, sel_getUid("removeAllDeliveredNotifications"));
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
            if (e->restStr) {
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
                CFStringRef sets = formatStr(CFSTR("%d"), e->completedSets + 1);
                CFStringReplace(e->headerStr, e->hRange, sets);
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

static void exerciseView_configure(StatusView *ptr, VCacheRef tbl, CCacheRef clr) {
    ExerciseEntry *e = ptr->entry;

    tbl->label.setText(ptr->headerLabel, tbl->label.stxt, e->headerStr);
    if (e->state == ExerciseStateResting) {
        tbl->button.setTitle(ptr->button, tbl->button.sbtxt, e->restStr, 0);
    } else {
        tbl->button.setTitle(ptr->button, tbl->button.sbtxt, e->titleStr, 0);
    }

    statusView_updateAccessibility(ptr, tbl);

    switch (e->state) {
        case ExerciseStateDisabled:
            tbl->view.setBG(ptr->box, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorGray));
            tbl->button.setEnabled(ptr->button, tbl->button.en, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseDuration)
                wkData.setInteraction(ptr->button, wkData.btnEn, false);
        case ExerciseStateResting:
            tbl->button.setEnabled(ptr->button, tbl->button.en, true);
            tbl->view.setBG(ptr->box, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorOrange));
            break;
        case ExerciseStateCompleted:
            tbl->button.setEnabled(ptr->button, tbl->button.en, false);
            tbl->view.setBG(ptr->box, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorGreen));
    }
}

static bool didFinishCircuit(Circuit *c) {
    if (c->type == CircuitRounds) {
        return ++c->completedReps == c->reps;
    } else if (c->type == CircuitDecrement) {
        bool changeRange = c->completedReps-- == 10;
        if (c->completedReps == 0) return true;

        CFStringRef reps = formatStr(CFSTR("%d"), c->completedReps);
        ExerciseEntry *end = &c->exercises[c->size];
        for (ExerciseEntry *e = c->exercises; e < end; ++e) {
            if (e->type == ExerciseReps) {
                CFStringReplace(e->titleStr, e->tRange, reps);
                if (changeRange)
                    e->tRange.length -= 1;
            }
        }
        CFRelease(reps);
    }
    return false;
}

static int findTransition(Workout *w, WorkoutTimer *timers, StatusView *view) {
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
        cycleExerciseEntry(view->entry + 1, timers);
    }
    return t;
}

#pragma mark - VC init/free

id workoutVC_init(Workout *workout, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(WorkoutVCClass, Sels.nw);
    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
    data->tbl = tbl;
    data->clr = clr;
    data->workout = workout;
    data->containers = malloc((unsigned)workout->size * sizeof(CVPair));

    pthread_mutex_init(&timerLock, NULL);
    for (unsigned char i = 0; i < 2; ++i) {
        memcpy(&data->timers[i], &(WorkoutTimer){.info = {.type = i}}, sizeof(WorkoutTimer));
        pthread_mutex_init(&data->timers[i].lock, NULL);
        pthread_cond_init(&data->timers[i].cond, NULL);
    }

    struct sigaction sa = {.sa_flags = 0, .sa_handler = handle_exercise_timer_interrupt};
    sigemptyset(&sa.sa_mask);
    sigaction(SignalExercise, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SignalGroup, &sa, NULL);
    exerciseTimerThread = &data->threads[TimerExercise];
    data->timers[0].parent = data->timers[1].parent = data;
    pthread_create(&data->threads[1], NULL, timer_loop, &data->timers[1]);
    pthread_create(&data->threads[0], NULL, timer_loop, &data->timers[0]);
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);

    startWorkoutTimer(&data->timers[TimerGroup], 0);
    startWorkoutTimer(&data->timers[TimerExercise], 0);
    pthread_join(data->threads[1], NULL);
    pthread_join(data->threads[0], NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&data->timers[i].cond);
        pthread_mutex_destroy(&data->timers[i].lock);
    }
    pthread_mutex_destroy(&timerLock);
    exerciseTimerThread = NULL;
    int size = data->workout->size;
    Circuit *cEnd = &data->workout->activities[size];
    for (Circuit *c = &data->workout->activities[0]; c < cEnd; ++c) {
        if (c->headerStr) CFRelease(c->headerStr);
        for (ExerciseEntry *e = &c->exercises[0]; e < &c->exercises[c->size]; ++e) {
            CFRelease(e->titleStr);
            if (e->restStr) CFRelease(e->restStr);
            if (e->headerStr) CFRelease(e->headerStr);
        }
        free(c->exercises);
    }
    free(data->workout->activities);
    free(data->workout);

    for (int i = 0; i < size; ++i) {
        Sels.viewRel(data->containers[i].view, Sels.rel);
    }
    free(data->containers);
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);
}

static bool isCompleted(Workout *w) {
    Circuit *group = w->group;
    int groupIndex = group->index;
    if (w->index != w->size - 1 || groupIndex != group->size - 1) return false;
    if (w->type == WorkoutEndurance)
        return w->duration >= (int16_t)(group->exercises[0].reps / 60);

    if (group->type == CircuitRounds && group->completedReps == group->reps - 1) {
        ExerciseEntry *e = &group->exercises[groupIndex];
        return e->state == ExerciseStateResting && e->completedSets == e->sets - 1;
    }
    return false;
}

static inline bool setDuration(Workout *w) {
    w->duration = ((int16_t)((time(NULL) - w->startTime) / 60.f)) + 1;
#if TARGET_OS_SIMULATOR
    w->duration *= 10;
#endif
    return w->duration >= 15;
}

static void workoutVC_handleFinishedWorkout(WorkoutVC *data, bool longEnough) {
    short *lifts = NULL;
    if (data->weights[0]) {
        lifts = malloc(sizeof(short) << 2);
        memcpy(lifts, data->weights, sizeof(short) << 2);
        longEnough = true;
    }

    unsigned char totalCompleted = 0;
    if (longEnough) {
        totalCompleted = addWorkoutData(data->workout->day,
                                        data->workout->type, data->workout->duration, lifts);
    }

    id navVC = msg0(id, (id)((char *)data - VCSize), sel_getUid("navigationController"));
    if (totalCompleted) {
        CFArrayRef ctrls = msg0(CFArrayRef, navVC, sel_getUid("viewControllers"));
        homeVC_handleFinishedWorkout((id)CFArrayGetValueAtIndex(ctrls, 0), totalCompleted);
    }
    msg1(id, bool, navVC, sel_getUid("popViewControllerAnimated:"), true);
}

#pragma mark - Main VC Functions

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
    VCacheRef tbl = data->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg,
                    data->clr->getColor(data->clr->cls, data->clr->sc, ColorPrimaryBGGrouped));
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    setVCTitle(navItem, data->workout->title);

    id stack = createVStack(NULL, 0);
    tbl->stack.setSpace(stack, tbl->stack.ssp, 20);
    tbl->stack.setMargins(stack, tbl->stack.smr, (HAInsets){16, 8, 16, 8});
    id startBtn = createButton(tbl, data->clr, localize(CFBundleGetMainBundle(), CFSTR("start")),
                               ColorGreen, UIFontTextStyleBody, self, sel_getUid("startEndWorkout:"));
    setNavButtons(navItem, (id []){nil, startBtn});

    SEL btnTap = sel_getUid("buttonTapped:");
    ContainerView *container;
    StatusView *sv;
    for (int i = 0; i < data->workout->size; ++i) {
        Circuit *c = &data->workout->activities[i];
        data->containers[i].view = containerView_init(tbl, data->clr, c->headerStr, &container);
        data->containers[i].data = container;
        if (!i) {
            data->first = container;
            tbl->view.hide(container->divider, tbl->view.shd, true);
        }
        tbl->stack.addSub(stack, tbl->stack.asv, data->containers[i].view);

        bool addHint = c->size > 1;
        for (int j = 0; j < c->size; ++j) {
            id v = statusView_init(tbl, data->clr, NULL, &sv, (int)((i << 8) | j), self, btnTap);
            sv->entry = &c->exercises[j];
            if (addHint) {
                CFStringRef exerciseHint = formatStr(exerciseProgressFmt, j + 1, c->size);
                tbl->view.setHint(sv->button, tbl->view.shn, exerciseHint);
                CFRelease(exerciseHint);
            }
            exerciseView_configure(sv, tbl, data->clr);
            tbl->stack.addSub(container->stack, tbl->stack.asv, v);
            Sels.viewRel(v, Sels.rel);
        }
    }

    id scrollView = createScrollView();
    addVStackToScrollView(tbl, view, stack, scrollView);
    Sels.viewRel(scrollView, Sels.rel);
    Sels.viewRel(stack, Sels.rel);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, restartTimers, UIApplicationDidBecomeActiveNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, stopTimers, UIApplicationWillResignActiveNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
    VCacheRef tbl = data->tbl;
    if (!tbl->view.getTag(btn, tbl->view.gtg)) {
        CFStringRef str = localize(CFBundleGetMainBundle(), CFSTR("end"));
        tbl->button.setTitle(btn, tbl->button.sbtxt, str, 0);
        CFRelease(str);
        tbl->button.setColor(btn, tbl->button.sbc,
                             data->clr->getColor(data->clr->cls, data->clr->sc, ColorRed), 0);
        tbl->view.setTag(btn, tbl->view.stg, 1);
        data->workout->startTime = time(NULL);
        startGroup(data->workout->group, data->timers, true);
        CFArrayRef views = tbl->stack.getSub(data->first->stack, tbl->stack.gsv);
        for (int i = 0; i < data->workout->group->size; ++i) {
            exerciseView_configure(
              (StatusView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize), tbl, data->clr);
        }
        id nextView;
        if (data->workout->group->headerStr) {
            nextView = data->first->headerLabel;
        } else {
            nextView = ((StatusView *)((char *)CFArrayGetValueAtIndex(views, 0) + ViewSize))->button;
        }
        UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
    } else {
        pthread_mutex_lock(&timerLock);
        if (data->done) {
            pthread_mutex_unlock(&timerLock);
            return;
        }
        cleanupWorkoutNotifications(data);
        pthread_mutex_unlock(&timerLock);
        bool longEnough = setDuration(data->workout);
        if (isCompleted(data->workout)) {
            workoutVC_handleFinishedWorkout(data, longEnough);
        } else {
            if (longEnough)
                addWorkoutData(0xff, data->workout->type, data->workout->duration, NULL);
            msg1(id, bool, msg0(id, self, sel_getUid("navigationController")),
                 sel_getUid("popViewControllerAnimated:"), true);
        }
    }
}

void workoutVC_willDisappear(id self, SEL _cmd, bool animated) {
    msgSup1(void, bool, (&(struct objc_super){self, VC}), _cmd, animated);

    if (msg0(bool, self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
        if (!data->done) {
            cleanupWorkoutNotifications(data);
            if (data->workout->startTime) {
                bool longEnough = setDuration(data->workout);
                if (isCompleted(data->workout)) {
                    workoutVC_handleFinishedWorkout(data, longEnough);
                } else if (longEnough) {
                    addWorkoutData(0xff, data->workout->type, data->workout->duration, NULL);
                }
            }
        }
    }
}

void workoutVC_handleTap(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *data = (WorkoutVC *)((char *)self + VCSize);
    int tag = (int)data->tbl->view.getTag(btn, data->tbl->view.gtg);
    int groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & 0xff;
    handleEvent(data, groupIdx, exerciseIdx, 0);
}

void handleEvent(WorkoutVC *data, int gIdx, int eIdx, int event) {
    pthread_mutex_lock(&timerLock);
    Workout *w = data->workout;
    VCacheRef tbl = data->tbl;
    if (data->done || gIdx != w->index || (eIdx != w->group->index && event != EventFinishGroup)) {
        pthread_mutex_unlock(&timerLock);
        return;
    }

    CFArrayRef views = tbl->stack.getSub(data->first->stack, tbl->stack.gsv);
    StatusView *ptr = (StatusView *)((char *)CFArrayGetValueAtIndex(views, eIdx) + ViewSize);
    id nextView = nil;
    bool longEnough, exerciseDone;
    int t = 0;
    switch (event) {
        case EventFinishGroup:
            t = TransitionFinishedCircuitDeleteFirst;
            if (++w->index == w->size) {
                t = TransitionCompletedWorkout;
                goto foundTransition;
            }
            ++w->group;
            data->timers[0].container = data->timers[1].container = w->index;
            startGroup(w->group, data->timers, true);
            goto foundTransition;

        case EventFinishExercise:
            wkData.setInteraction(ptr->button, wkData.btnEn, true);
            if (w->type == WorkoutEndurance) {
                tbl->label.setColor(ptr->headerLabel, tbl->label.stc,
                                    data->clr->getColor(data->clr->cls, data->clr->sc, ColorGreen));
                CFStringRef message = localize(CFBundleGetMainBundle(), CFSTR("exerciseDurationMessage"));
                tbl->label.setText(ptr->headerLabel, tbl->label.stxt, message);
                CFRelease(message);
                statusView_updateAccessibility(ptr, tbl);
                nextView = ptr->button;
                goto foundTransition;
            }
        default:
            exerciseDone = cycleExerciseEntry(ptr->entry, data->timers);
            exerciseView_configure(ptr, tbl, data->clr);
            if (exerciseDone)
                t = findTransition(w, data->timers, ptr);
    }
foundTransition:

    switch (t) {
        case TransitionCompletedWorkout:
            cleanupWorkoutNotifications(data);
            longEnough = setDuration(w);
            pthread_mutex_unlock(&timerLock);
            if (UIAccessibilityIsVoiceOverRunning()) {
                CFStringRef message = localize(CFBundleGetMainBundle(), CFSTR("workoutCompleteMsg"));
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 4000000000), dispatch_get_main_queue(), ^{
                    UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)message);
                    CFRelease(message);
                });
            }
            workoutVC_handleFinishedWorkout(data, longEnough);
            return;

        case TransitionFinishedCircuitDeleteFirst:
            data->first = data->containers[w->index].data;
            views = tbl->stack.getSub(data->first->stack, tbl->stack.gsv);
            tbl->view.rmSub(data->containers[w->index - 1].view, tbl->view.rsv);
            tbl->view.hide(data->first->divider, tbl->view.shd, true);
            nextView = data->first->headerLabel;

        case TransitionFinishedCircuit:
            if (w->group->reps > 1 && w->group->type == CircuitRounds) {
                CFStringRef newNumber = formatStr(CFSTR("%d"), w->group->completedReps + 1);
                CFStringReplace(w->group->headerStr, w->group->numberRange, newNumber);
                CFRelease(newNumber);
                tbl->label.setText(data->first->headerLabel, tbl->label.stxt, w->group->headerStr);
                nextView = data->first->headerLabel;
            }

            for (int i = 0; i < w->group->size; ++i) {
                ptr = (StatusView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize);
                tbl->button.setEnabled(ptr->button, tbl->button.en, true);
                exerciseView_configure(ptr, tbl, data->clr);
            }
            if (!nextView)
                nextView = ((StatusView *)((char *)CFArrayGetValueAtIndex(views, 0) + ViewSize))->button;
            break;

        case TransitionFinishedExercise:
            ptr = (StatusView *)((char *)CFArrayGetValueAtIndex(views, w->group->index) + ViewSize);
            exerciseView_configure(ptr, tbl, data->clr);
            nextView = ptr->button;
            break;

        default:
            if (w->testMax) {
                pthread_mutex_unlock(&timerLock);
                presentModalVC(updateMaxesVC_init(data, eIdx, w->bodyweight, tbl, data->clr));
                return;
            }
            break;
    }
    pthread_mutex_unlock(&timerLock);
    if (nextView)
        UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
}

void workoutVC_finishedBottomSheet(void *self, int index, short weight) {
    WorkoutVC *data = (WorkoutVC *)self;
    data->weights[index] = weight;
    handleEvent(data, 0, index, 0);
}
