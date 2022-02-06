#include "WorkoutVC.h"
#include <dispatch/queue.h>
#include <signal.h>
#include "AppDelegate.h"
#include "AppUserData.h"
#include "HomeVC.h"
#include "PersistenceService.h"
#include "StatusView.h"
#include "UpdateMaxesVC.h"
#include "ViewControllerHelpers.h"

#define popVC(_navVC) ((id(*)(id,SEL,bool))objc_msgSend)\
((_navVC), sel_getUid("popViewControllerAnimated:"), true)

#define isWorkoutLongEnough(w) (w)->duration >= 15

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
    EventStartGroup = 1,
    EventFinishGroup,
    EventFinishExercise
};

enum {
    TransitionCompletedWorkout = 1,
    TransitionFinishedCircuitDeleteFirst,
    TransitionFinishedCircuit,
    TransitionFinishedExercise,
};

static const unsigned ExerciseTagNA = 255;
static CFStringRef notificationMessages[2];
static CFStringRef ExerciseStates[4];
static CFStringRef notifTitle;
static pthread_t *exerciseTimerThread;
static pthread_mutex_t timerLock;

static void handleEvent(id self, unsigned gIdx, unsigned eIdx, unsigned char event);

void initWorkoutStrings(void) {
    fillStringArray(ExerciseStates, CFSTR("exerciseState%d"), 4);
    notifTitle = localize(CFSTR("workoutNotificationTitle"));
    notificationMessages[0] = localize(CFSTR("notifications0"));
    notificationMessages[1] = localize(CFSTR("notifications1"));
}

#pragma mark - Timers

static void handle_exercise_timer_interrupt(int n _U_) {}

static void handle_group_timer_interrupt(int n _U_) {}

static void *timer_loop(void *arg) {
    WorkoutTimer *t = arg;
    unsigned char eventType = 0;
    unsigned res = 0, duration = 0, container = ExerciseTagNA, exercise = ExerciseTagNA;
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

void stopTimers(id self) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    if (data->timers[TimerGroup].info.active == 1) {
        data->savedInfo.groupTag = data->timers[TimerGroup].container;
        pthread_kill(data->threads[TimerGroup], SignalGroup);
    } else {
        data->savedInfo.groupTag = ExerciseTagNA;
    }

    if (data->timers[TimerExercise].info.active == 1) {
        data->savedInfo.exerciseInfo.group = data->timers[TimerExercise].container;
        data->savedInfo.exerciseInfo.tag = data->timers[TimerExercise].exercise;
        pthread_kill(data->threads[TimerExercise], SignalExercise);
    } else {
        data->savedInfo.exerciseInfo.group = ExerciseTagNA;
    }
    pthread_mutex_unlock(&timerLock);
}

void restartTimers(id self) {
    bool endExercise = false, endGroup = false;
    unsigned groupIdx = 0, exerciseIdx = 0;
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    if (data->done) {
        pthread_mutex_unlock(&timerLock);
        return;
    }
    time_t now = time(NULL);
    unsigned group = data->savedInfo.exerciseInfo.group;
    exerciseIdx = data->savedInfo.exerciseInfo.tag;
    if (group != ExerciseTagNA) {
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
    if (group != ExerciseTagNA) {
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
        handleEvent(self, groupIdx, exerciseIdx, EventFinishExercise);
    if (endGroup)
        handleEvent(self, groupIdx, 0, EventFinishGroup);
}

#pragma mark - Notifications

static void scheduleNotification(unsigned secondsFromNow, unsigned char type) {
    static int identifier = 0;
    int currentId = identifier++;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), currentId);

    id content = createNew(objc_getClass("UNMutableNotificationContent"));
    setString(content, sel_getUid("setTitle:"), notifTitle);
    setString(content, sel_getUid("setBody:"), notificationMessages[type]);
    id sound = staticMethod(objc_getClass("UNNotificationSound"), sel_getUid("defaultSound"));
    setObject(content, sel_getUid("setSound:"), sound);

    id trigger = (((id(*)(Class,SEL,double,bool))objc_msgSend)
                  (objc_getClass("UNTimeIntervalNotificationTrigger"),
                   sel_getUid("triggerWithTimeInterval:repeats:"), secondsFromNow, false));
    id req = (((id(*)(Class,SEL,CFStringRef,id,id))objc_msgSend)
              (objc_getClass("UNNotificationRequest"),
               sel_getUid("requestWithIdentifier:content:trigger:"),
               idString, content, trigger));

    id center = getNotificationCenter();
    (((void(*)(id,SEL,id,void(^)(id)))objc_msgSend)
     (center, sel_getUid("addNotificationRequest:withCompletionHandler:"),
      req, ^(id error _U_) {}));
    CFRelease(idString);
    releaseObj(content);
}

static void cleanupWorkoutNotifications(id *observers) {
    SEL remove = sel_getUid("removeObserver:");
    id center = getDeviceNotificationCenter();
    for (int i = 0; i < 2; ++i) {
        setObject(center, remove, observers[i]);
        observers[i] = nil;
    }
    center = getNotificationCenter();
    voidFunc(center, sel_getUid("removeAllPendingNotificationRequests"));
    voidFunc(center, sel_getUid("removeAllDeliveredNotifications"));
}

#pragma mark - View Configuration

static bool cycleExerciseEntry(ExerciseEntry *e, WorkoutTimer *timers) {
    bool completed = false;
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) {
                startWorkoutTimer(&timers[TimerExercise], e->reps);
                scheduleNotification(e->reps, TimerExercise);
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
                completed = true;
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
    return completed;
}

static void startGroup(Circuit *c, WorkoutTimer *timers, bool startTimer) {
    timers[TimerExercise].exercise = c->index = 0;

    for (ExerciseEntry *e = c->exercises; e < &c->exercises[c->size]; ++e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer) {
        unsigned duration = 60 * c->reps;
        startWorkoutTimer(&timers[TimerGroup], duration);
        scheduleNotification(duration, TimerGroup);
    }

    cycleExerciseEntry(c->exercises, timers);
}

static void exerciseView_configure(id v) {
    StatusView *ptr = (StatusView *) ((char *)v + ViewSize);
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

#pragma mark - VC init/free

id workoutVC_init(Workout *workout) {
    id self = createNew(WorkoutVCClass);
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    data->workout = workout;
    data->containers = malloc(workout->size * sizeof(id));

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

    if (data->timers[TimerGroup].info.active == 1)
        pthread_kill(data->threads[TimerGroup], SignalGroup);
    if (data->timers[TimerExercise].info.active == 1)
        pthread_kill(data->threads[TimerExercise], SignalExercise);
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
    CFRelease(data->workout->title);
    unsigned size = data->workout->size;
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

    for (unsigned i = 0; i < size; ++i) {
        releaseObj(data->containers[i]);
    }
    free(data->containers);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

static bool checkEnduranceDuration(Workout *w) {
    if (w->type != WorkoutEndurance) return false;
    int planDuration = w->activities[0].exercises[0].reps / 60;
    return w->duration >= planDuration;
}

static inline void setDuration(Workout *w) {
    w->duration = ((int16_t) ((time(NULL) - w->startTime) / 60.f)) + 1;
#if TARGET_OS_SIMULATOR
    w->duration *= 10;
#endif
}

static void updateStoredData(unsigned char type, int16_t duration, short *lifts) {
    runInBackground((^{
        id data = persistenceService_getCurrentWeek();
        int16_t newDuration = duration + weekData_getWorkoutTimeForType(data, type);
        weekData_setWorkoutTimeForType(data, type, newDuration);
        int16_t totalWorkouts = weekData_getTotalWorkouts(data) + 1;
        weekData_setTotalWorkouts(data, totalWorkouts);
        if (lifts) {
            weekData_setLiftingMaxArray(data, lifts);
            free(lifts);
        }
        persistenceService_saveContext();
    }));
}

static void workoutVC_handleFinishedWorkout(id self) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    Workout *w = data->workout;
    unsigned char totalCompleted = 0;
    short *lifts = NULL;

    if (data->weights[0]) {
        appDel_updateMaxWeights(data->weights);
        lifts = malloc(sizeof(short) << 2);
        memcpy(lifts, data->weights, sizeof(short) << 2);
        if (data->workout->duration < 15)
            data->workout->duration = 15;
    }

    if (isWorkoutLongEnough(data->workout)) {
        if (w->day >= 0)
            totalCompleted = appUserData_addCompletedWorkout((unsigned char) w->day);
        updateStoredData(w->type, w->duration, lifts);
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
    id startBtn = createButton(localize(CFSTR("start")), ColorGreen, 0, 0,
                                      self, sel_getUid("startEndWorkout:"), -1);

    SEL btnTap = sel_getUid("buttonTapped:");
    CGRect frame;
    getRect(view, &frame, 0);
    setNavButton(self, false, startBtn, (int) frame.size.width);

    for (unsigned i = 0; i < data->workout->size; ++i) {
        Circuit *c = &data->workout->activities[i];
        data->containers[i] = containerView_init(c->headerStr, 0, false);
        ContainerView *container = (ContainerView*) ((char *)data->containers[i] + ViewSize);
        addArrangedSubview(stack, data->containers[i]);

        for (unsigned j = 0; j < c->size; ++j) {
            id v = statusView_init(NULL, (int) ((i << 8) | j), self, btnTap);
            StatusView *ptr = (StatusView *) ((char *)v + ViewSize);
            ptr->entry = &c->exercises[j];
            exerciseView_configure(v);
            addArrangedSubview(container->stack, v);
            releaseObj(v);
        }
    }

    id scrollView = createScrollView();
    addSubview(view, scrollView);
    id guide = getLayoutGuide(view);
    pin(scrollView, guide, (Padding){0}, 0);
    addVStackToScrollView(stack, scrollView);

    releaseObj(scrollView);
    releaseObj(stack);

    data->first = (ContainerView *) ((char *)data->containers[0] + ViewSize);
    hideView(data->first->divider, true);
    SEL obsSig = sel_getUid("addObserverForName:object:queue:usingBlock:");
    id weakSelf = self;
    id main = staticMethod(objc_getClass("NSOperationQueue"), sel_getUid("mainQueue"));
    id center = getDeviceNotificationCenter();
    data->observers[0] = (((id(*)(id,SEL,CFStringRef,id,id,void(^)(id)))objc_msgSend)
                          (center, obsSig, (CFStringRef) UIApplicationDidBecomeActiveNotification,
                           nil, main, ^(id note _U_){ restartTimers(weakSelf); }));
    data->observers[1] = (((id(*)(id,SEL,CFStringRef,id,id,void(^)(id)))objc_msgSend)
                          (center, obsSig, (CFStringRef) UIApplicationWillResignActiveNotification,
                           nil, main, ^(id note _U_){ stopTimers(weakSelf); }));
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id btn) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    if (!getTag(btn)) {
        setButtonTitle(btn, localize(CFSTR("end")), 0);
        setButtonColor(btn, createColor(ColorRed), 0);
        setTag(btn, 1);
        data->workout->startTime = time(NULL);
        handleEvent(self, 0, 0, EventStartGroup);
    } else {
        bool valid = false;
        pthread_mutex_lock(&timerLock);
        if (!data->done) {
            data->done = valid = true;
            cleanupWorkoutNotifications(data->observers);
        }
        pthread_mutex_unlock(&timerLock);
        if (valid) {
            Workout *w = data->workout;
            setDuration(w);
            if (checkEnduranceDuration(w)) {
                workoutVC_handleFinishedWorkout(self);
            } else {
                if (isWorkoutLongEnough(w))
                    updateStoredData(w->type, w->duration, NULL);
                id navVC = getNavVC(self);
                popVC(navVC);
            }
        }
    }
}

void workoutVC_willDisappear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    if (getBool(self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
        if (!data->done) {
            cleanupWorkoutNotifications(data->observers);
            Workout *w = data->workout;
            if (w->startTime) {
                setDuration(w);
                if (checkEnduranceDuration(w)) {
                    workoutVC_handleFinishedWorkout(self);
                } else if (isWorkoutLongEnough(w)) {
                    updateStoredData(w->type, w->duration, NULL);
                }
            }
        }
    }
}

void workoutVC_handleTap(id self, SEL _cmd _U_, id btn) {
    unsigned tag = (unsigned) getTag(btn);
    unsigned groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & 0xff;
    handleEvent(self, groupIdx, exerciseIdx, 0);
}

void handleEvent(id self, unsigned gIdx, unsigned eIdx, unsigned char event) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    pthread_mutex_lock(&timerLock);
    Workout *w = data->workout;
    if (data->done || gIdx != w->index || (eIdx != w->group->index && event != EventFinishGroup)) {
        pthread_mutex_unlock(&timerLock);
        return;
    }

    CFArrayRef views = getArrangedSubviews(data->first->stack);
    id currView = (id) CFArrayGetValueAtIndex(views, eIdx);
    ExerciseEntry *entry = &w->group->exercises[eIdx];
    StatusView *ptr = (StatusView *) ((char *)currView + ViewSize);
    unsigned char t = 0;
    switch (event) {
        case EventFinishExercise:
            toggleInteraction(ptr->button, true);
            if (w->type == WorkoutEndurance)
                goto foundTransition;
            break;

        case EventStartGroup:
            t = TransitionFinishedCircuit;
            startGroup(w->group, data->timers, true);
            goto foundTransition;

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

        default:
            break;
    }

    bool exerciseDone = cycleExerciseEntry(entry, data->timers);
    exerciseView_configure(currView);

    if (exerciseDone) {
        t = TransitionFinishedExercise;
        if (++w->group->index == w->group->size) {
            t = TransitionFinishedCircuit;
            Circuit *c = w->group;
            bool finishedCircuit = false, changeRange;
            switch (c->type) {
                case CircuitRounds:
                    if (++c->completedReps == c->reps)
                        finishedCircuit = true;
                    break;

                case CircuitDecrement:
                    changeRange = c->completedReps == 10;
                    if (--c->completedReps == 0) {
                        finishedCircuit = true;
                    } else {
                        CFStringRef reps = CFStringCreateWithFormat(NULL, NULL,
                                                                    CFSTR("%d"), c->completedReps);
                        ExerciseEntry *e;
                        for (e = &c->exercises[0]; e < &c->exercises[c->size]; ++e) {
                            if (e->type == ExerciseReps) {
                                CFStringReplace(e->titleStr, e->tRange, reps);
                                if (changeRange)
                                    e->tRange.length -= 1;
                            }
                        }
                        CFRelease(reps);
                    }
                default:
                    break;
            }

            if (finishedCircuit) {
                if (++w->index == w->size) {
                    t = TransitionCompletedWorkout;
                } else {
                    t = TransitionFinishedCircuitDeleteFirst;
                    data->timers[0].container = data->timers[1].container = w->index;
                    startGroup(++w->group, data->timers, true);
                }
            } else {
                startGroup(w->group, data->timers, false);
            }
        } else {
            cycleExerciseEntry(entry + 1, data->timers);
        }
    }
foundTransition:

    switch (t) {
        case TransitionCompletedWorkout:
            data->done = true;
            cleanupWorkoutNotifications(data->observers);
            setDuration(w);
            pthread_mutex_unlock(&timerLock);
            workoutVC_handleFinishedWorkout(self);
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
            }
            setLabelText(data->first->headerLabel, w->group->headerStr);

            for (unsigned i = 0; i < w->group->size; ++i) {
                exerciseView_configure((id) CFArrayGetValueAtIndex(views, i));
            }
            break;

        case TransitionFinishedExercise:
            exerciseView_configure((id) CFArrayGetValueAtIndex(views, w->group->index));
            break;

        default:
            if (w->testMax) {
                pthread_mutex_unlock(&timerLock);
                presentModalVC(self, updateMaxesVC_init(self, eIdx));
                return;
            }
            break;
    }
    pthread_mutex_unlock(&timerLock);
}

void workoutVC_finishedBottomSheet(id self, unsigned index, short weight) {
    WorkoutVC *data = (WorkoutVC *) ((char *)self + VCSize);
    data->weights[index] = weight;
    handleEvent(self, 0, index, 0);
}
