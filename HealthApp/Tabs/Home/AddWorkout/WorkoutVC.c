#include "WorkoutVC.h"
#include <CoreFoundation/CFNotificationCenter.h>
#include <dispatch/queue.h>
#include <signal.h>
#include "AppDelegate.h"
#include "AppUserData.h"
#include "HomeVC.h"
#include "PersistenceService.h"
#include "StatusView.h"
#include "UpdateMaxesVC.h"
#include "ViewControllerHelpers.h"

#define toggleInteraction(v, e) msg1(void, bool, v, sel_getUid("setUserInteractionEnabled:"), e)

#define popVC(_navVC) msg1(id, bool, _navVC, sel_getUid("popViewControllerAnimated:"), true)

extern id UIApplicationDidBecomeActiveNotification;
extern id UIApplicationWillResignActiveNotification;

Class WorkoutVCClass;

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

static CFStringRef notificationMessages[2];
static CFStringRef ExerciseStates[4];
static CFStringRef notifTitle;
static pthread_t *exerciseTimerThread;
static pthread_mutex_t timerLock;

static void handleEvent(id self, int gIdx, int eIdx, int event);

void initWorkoutStrings(CFBundleRef bundle) {
    fillStringArray(bundle, ExerciseStates, CFSTR("exerciseState%d"), 4);
    fillStringArray(bundle, notificationMessages, CFSTR("notifications%d"), 2);
    notifTitle = CFBundleCopyLocalizedString(bundle, CFSTR("workoutNotificationTitle"), NULL, NULL);
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
            id parent = t->parent;
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

static void stopTimers(CFNotificationCenterRef center _U_,
                        void *observer, CFNotificationName name _U_,
                        const void *object _U_, CFDictionaryRef userinfo _U_) {
    WorkoutVC *data = (WorkoutVC *) ((char *)observer + VCSize);
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

static void restartTimers(CFNotificationCenterRef center _U_,
                           void *observer, CFNotificationName name _U_,
                           const void *object _U_, CFDictionaryRef userinfo _U_) {
    bool endExercise = false, endGroup = false;
    int groupIdx = 0, exerciseIdx = 0;
    WorkoutVC *data = (WorkoutVC *) ((char *)observer + VCSize);
    pthread_mutex_lock(&timerLock);
    if (data->done) {
        pthread_mutex_unlock(&timerLock);
        return;
    }
    time_t now = time(NULL);
    int group = data->savedInfo.exerciseInfo.group;
    exerciseIdx = data->savedInfo.exerciseInfo.tag;
    if (group >= 0) {
        unsigned diff = (unsigned) (now - data->timers[TimerExercise].refTime);
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
        unsigned diff = (unsigned) (now - data->timers[TimerGroup].refTime);
        if (diff >= data->timers[TimerGroup].duration) {
            endGroup = true;
            groupIdx = group;
        } else {
            startWorkoutTimer(&data->timers[TimerGroup], data->timers[TimerGroup].duration - diff);
        }
    }
    pthread_mutex_unlock(&timerLock);

    if (endExercise)
        handleEvent(observer, groupIdx, exerciseIdx, EventFinishExercise);
    if (endGroup)
        handleEvent(observer, groupIdx, 0, EventFinishGroup);
}

#pragma mark - Notifications

static void scheduleNotification(unsigned secondsFromNow, int type) {
    static int identifier = 0;
    int currentId = identifier++;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), currentId);

    id content = createNew(objc_getClass("UNMutableNotificationContent"));
    msg1(void, CFStringRef, content, sel_getUid("setTitle:"), notifTitle);
    msg1(void, CFStringRef, content, sel_getUid("setBody:"), notificationMessages[type]);

    id sound = clsF0(id, objc_getClass("UNNotificationSound"), sel_getUid("defaultSound"));
    msg1(void, id, content, sel_getUid("setSound:"), sound);

    id trigger = clsF2(id, double, bool, objc_getClass("UNTimeIntervalNotificationTrigger"),
                       sel_getUid("triggerWithTimeInterval:repeats:"), secondsFromNow, false);
    id req = clsF3(id, CFStringRef, id, id, objc_getClass("UNNotificationRequest"),
                   sel_getUid("requestWithIdentifier:content:trigger:"), idString, content, trigger);
    msg2(void, id, void(^)(id), getUserNotificationCenter(),
         sel_getUid("addNotificationRequest:withCompletionHandler:"), req, ^(id error _U_) {});
    CFRelease(idString);
    releaseObj(content);
}

static void cleanupWorkoutNotifications(id self, WorkoutVC *data) {
    data->done = true;
    if (data->timers[TimerGroup].info.active == 1)
        pthread_kill(data->threads[TimerGroup], SignalGroup);
    if (data->timers[TimerExercise].info.active == 1)
        pthread_kill(data->threads[TimerExercise], SignalExercise);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    id center = getUserNotificationCenter();
    msg0(void, center, sel_getUid("removeAllPendingNotificationRequests"));
    msg0(void, center, sel_getUid("removeAllDeliveredNotifications"));
}

#pragma mark - View Configuration

static bool cycleExerciseEntry(ExerciseEntry *e, WorkoutTimer *timers) {
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) {
                startWorkoutTimer(&timers[TimerExercise], (unsigned) e->reps);
                scheduleNotification((unsigned) e->reps, TimerExercise);
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
                CFStringRef sets = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"),
                                                            e->completedSets + 1);
                CFStringReplace(e->headerStr, e->hRange, sets);
                CFRelease(sets);
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
        unsigned duration = (unsigned) (60 * c->reps);
        startWorkoutTimer(&timers[TimerGroup], duration);
        scheduleNotification(duration, TimerGroup);
    }

    cycleExerciseEntry(c->exercises, timers);
}

static void exerciseView_configure(StatusView *ptr) {
    ExerciseEntry *e = ptr->entry;

    setLabelText(ptr->headerLabel, e->headerStr);
    if (e->state == ExerciseStateResting) {
        setButtonTitle(ptr->button, e->restStr, 0);
    } else {
        setButtonTitle(ptr->button, e->titleStr, 0);
    }

    statusView_updateAccessibility(ptr, ExerciseStates[e->state]);

    switch (e->state) {
        case ExerciseStateDisabled:
            setBackground(ptr->box, createColor(ColorGray));
            enableButton(ptr->button, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseDuration)
                toggleInteraction(ptr->button, false);
        case ExerciseStateResting:
            enableButton(ptr->button, true);
            setBackground(ptr->box, createColor(ColorOrange));
            break;
        case ExerciseStateCompleted:
            enableButton(ptr->button, false);
            setBackground(ptr->box, createColor(ColorGreen));
    }
}

static bool didFinishCircuit(Circuit *c) {
    if (c->type == CircuitRounds) {
        return ++c->completedReps == c->reps;
    } else if (c->type == CircuitDecrement) {
        bool changeRange = c->completedReps-- == 10;
        if (c->completedReps == 0) return true;

        CFStringRef reps = CFStringCreateWithFormat(NULL, NULL,
                                                    CFSTR("%d"), c->completedReps);
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

id workoutVC_init(Workout *workout) {
    id self = createNew(WorkoutVCClass);
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    data->workout = workout;
    data->containers = malloc((unsigned) workout->size * sizeof(id));

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
    data->timers[0].parent = data->timers[1].parent = self;
    pthread_create(&data->threads[1], NULL, timer_loop, &data->timers[1]);
    pthread_create(&data->threads[0], NULL, timer_loop, &data->timers[0]);
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    struct objc_super super = {self, VCClass};

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
        releaseObj(data->containers[i]);
    }
    free(data->containers);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

static bool isCompleted(Workout *w) {
    Circuit *group = w->group;
    int groupIndex = group->index;
    if (w->index != w->size - 1 || groupIndex != group->size - 1) return false;
    if (w->type == WorkoutEndurance)
        return w->duration >= (int16_t) (group->exercises[0].reps / 60);

    if (group->type == CircuitRounds && group->completedReps == group->reps - 1) {
        ExerciseEntry *e = &group->exercises[groupIndex];
        return e->state == ExerciseStateResting && e->completedSets == e->sets - 1;
    }
    return false;
}

static inline bool setDuration(Workout *w) {
    w->duration = ((int16_t) ((time(NULL) - w->startTime) / 60.f)) + 1;
#if TARGET_OS_SIMULATOR
    w->duration *= 10;
#endif
    return w->duration >= 15;
}

static void updateStoredData(int type, int16_t duration, short *lifts) {
    id context = backgroundContext;
    msg1(void, void(^)(void), context, sel_getUid("performBlock:"), ^{
        CFArrayRef currentWeeks = persistenceService_fetchData(context, 2, &(int){0});
        id data = (id) CFArrayGetValueAtIndex(currentWeeks, 0);
        int16_t newDuration = duration + weekData_getWorkoutTimeForType(data, type);
        weekData_setWorkoutTimeForType(data, type, newDuration);
        int16_t totalWorkouts = weekData_getTotalWorkouts(data) + 1;
        weekData_setTotalWorkouts(data, totalWorkouts);
        if (lifts) {
            weekData_setLiftingMaxArray(data, lifts);
            free(lifts);
        }
        persistenceService_saveContext(context);
    });
}

static void workoutVC_handleFinishedWorkout(id self, WorkoutVC *data, bool longEnough) {
    unsigned char totalCompleted = 0;
    short *lifts = NULL;

    if (data->weights[0]) {
        appDel_updateMaxWeights(data->weights);
        lifts = malloc(sizeof(short) << 2);
        memcpy(lifts, data->weights, sizeof(short) << 2);
        if (!longEnough) {
            data->workout->duration = 15;
            longEnough = true;
        }
    }

    if (longEnough) {
        if (data->workout->day != 0xff)
            totalCompleted = appUserData_addCompletedWorkout(data->workout->day);
        updateStoredData(data->workout->type, data->workout->duration, lifts);
    }

    id navVC = getNavVC(self);
    if (totalCompleted) {
        CFArrayRef ctrls = getViewControllers(navVC);
        id parent = (id) CFArrayGetValueAtIndex(ctrls, 0);
        homeVC_handleFinishedWorkout(parent, totalCompleted);
    }
    popVC(navVC);
}

#pragma mark - Main VC Functions

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, data->workout->title);

    id stack = createStackView(NULL, 0, 1, 20, (Padding){20, 8, 20, 8});
    id startBtn = createButton(CFBundleCopyLocalizedString(CFBundleGetMainBundle(), CFSTR("start"),
                                                           NULL, NULL),
                               ColorGreen, 0, self, sel_getUid("startEndWorkout:"));
    setNavButtons(self, (id []){nil, startBtn});

    SEL btnTap = sel_getUid("buttonTapped:");
    ContainerView *container;
    StatusView *sv;
    for (int i = 0; i < data->workout->size; ++i) {
        Circuit *c = &data->workout->activities[i];
        if (c->headerStr)
            CFRetain(c->headerStr);
        data->containers[i] = containerView_init(c->headerStr, &container, 0, false);
        if (!i) {
            data->first = container;
            hideView(container->divider, true);
        }
        addArrangedSubview(stack, data->containers[i]);

        for (int j = 0; j < c->size; ++j) {
            id v = statusView_init(NULL, &sv, (int) ((i << 8) | j), self, btnTap);
            sv->entry = &c->exercises[j];
            exerciseView_configure(sv);
            addArrangedSubview(container->stack, v);
            releaseObj(v);
        }
    }

    id scrollView = createScrollView();
    addVStackToScrollView(view, stack, scrollView);
    releaseObj(scrollView);
    releaseObj(stack);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, restartTimers,
                                    (CFStringRef) UIApplicationDidBecomeActiveNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, stopTimers,
                                    (CFStringRef) UIApplicationWillResignActiveNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    if (!getTag(btn)) {
        CFStringRef str = CFBundleCopyLocalizedString(CFBundleGetMainBundle(),
                                                      CFSTR("end"), NULL, NULL);
        setButtonTitle(btn, str, 0);
        CFRelease(str);
        setButtonColor(btn, createColor(ColorRed), 0);
        setTag(btn, 1);
        data->workout->startTime = time(NULL);
        startGroup(data->workout->group, data->timers, true);
        CFArrayRef views = getArrangedSubviews(data->first->stack);
        for (int i = 0; i < data->workout->group->size; ++i) {
            StatusView *v = (StatusView *) ((char *)CFArrayGetValueAtIndex(views, i) + ViewSize);
            exerciseView_configure(v);
        }
    } else {
        pthread_mutex_lock(&timerLock);
        if (data->done) {
            pthread_mutex_unlock(&timerLock);
            return;
        }
        cleanupWorkoutNotifications(self, data);
        pthread_mutex_unlock(&timerLock);
        bool longEnough = setDuration(data->workout);
        if (isCompleted(data->workout)) {
            workoutVC_handleFinishedWorkout(self, data, longEnough);
        } else {
            if (longEnough)
                updateStoredData(data->workout->type, data->workout->duration, NULL);
            popVC(getNavVC(self));
        }
    }
}

void workoutVC_willDisappear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    if (msg0(bool, self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
        if (!data->done) {
            cleanupWorkoutNotifications(self, data);
            if (data->workout->startTime) {
                bool longEnough = setDuration(data->workout);
                if (isCompleted(data->workout)) {
                    workoutVC_handleFinishedWorkout(self, data, longEnough);
                } else if (longEnough) {
                    updateStoredData(data->workout->type, data->workout->duration, NULL);
                }
            }
        }
    }
}

void workoutVC_handleTap(id self, SEL _cmd _U_, id btn) {
    int tag = (int) getTag(btn);
    int groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & 0xff;
    handleEvent(self, groupIdx, exerciseIdx, 0);
}

void handleEvent(id self, int gIdx, int eIdx, int event) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    Workout *w = data->workout;
    if (data->done || gIdx != w->index || (eIdx != w->group->index && event != EventFinishGroup)) {
        pthread_mutex_unlock(&timerLock);
        return;
    }

    CFArrayRef views = getArrangedSubviews(data->first->stack);
    id currView = (id) CFArrayGetValueAtIndex(views, eIdx);
    StatusView *ptr = (StatusView *) ((char *)currView + ViewSize);
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
            toggleInteraction(ptr->button, true);
            if (w->type == WorkoutEndurance)
                goto foundTransition;
        default:
            exerciseDone = cycleExerciseEntry(ptr->entry, data->timers);
            exerciseView_configure(ptr);
            if (exerciseDone)
                t = findTransition(w, data->timers, ptr);
    }
foundTransition:

    switch (t) {
        case TransitionCompletedWorkout:
            cleanupWorkoutNotifications(self, data);
            longEnough = setDuration(w);
            pthread_mutex_unlock(&timerLock);
            workoutVC_handleFinishedWorkout(self, data, longEnough);
            return;

        case TransitionFinishedCircuitDeleteFirst:
            data->first = (ContainerView *) ((char *)data->containers[w->index] + ViewSize);
            views = getArrangedSubviews(data->first->stack);
            removeView(data->containers[w->index - 1]);
            hideView(data->first->divider, true);

        case TransitionFinishedCircuit:
            if (w->group->reps > 1 && w->group->type == CircuitRounds) {
                CFStringRef newNumber = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"),
                                                                 w->group->completedReps + 1);
                CFStringReplace(w->group->headerStr, w->group->numberRange, newNumber);
                CFRelease(newNumber);
                setLabelText(data->first->headerLabel, w->group->headerStr);
            }

            for (int i = 0; i < w->group->size; ++i) {
                StatusView *v = (StatusView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize);
                exerciseView_configure(v);
            }
            break;

        case TransitionFinishedExercise:
            ptr = (StatusView *)((char *)CFArrayGetValueAtIndex(views, w->group->index) + ViewSize);
            exerciseView_configure(ptr);
            break;

        default:
            if (w->testMax) {
                pthread_mutex_unlock(&timerLock);
                presentModalVC(updateMaxesVC_init(self, eIdx, w->bodyweight));
                return;
            }
            break;
    }
    pthread_mutex_unlock(&timerLock);
}

void workoutVC_finishedBottomSheet(id self, int index, short weight) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    data->weights[index] = weight;
    handleEvent(self, 0, index, 0);
}
