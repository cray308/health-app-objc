#include "WorkoutVC.h"
#include <dispatch/queue.h>
#include <signal.h>
#include "StatusView.h"
#include "ViewControllerHelpers.h"

extern id UIApplicationDidBecomeActiveNotification;
extern id UIApplicationWillResignActiveNotification;

Class WorkoutVCClass;
Ivar WorkoutVCDataRef;

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
static CFStringRef circuitHeaderRounds;
static CFStringRef circuitHeaderAMRAP;
static CFStringRef exerciseHeader;
static CFStringRef exerciseTitleRest;
static CFStringRef exerciseTitleReps;
static CFStringRef exerciseTitleRepsWithWeight;
static CFStringRef exerciseTitleDurationMinutes;
static CFStringRef exerciseTitleDurationSeconds;
static CFStringRef exerciseTitleDistance;
static pthread_t *exerciseTimerThread;
static pthread_mutex_t timerLock;

static void handleEvent(id self, unsigned gIdx, unsigned eIdx, unsigned char event);
static void restartTimers(id);
static void stopTimers(id);

void initWorkoutStrings(void) {
    fillStringArray(ExerciseStates, CFSTR("exerciseState%d"), 4);
    notifTitle = localize(CFSTR("workoutNotificationTitle"));
    notificationMessages[0] = localize(CFSTR("notifications0"));
    notificationMessages[1] = localize(CFSTR("notifications1"));
    circuitHeaderRounds = localize(CFSTR("circuitHeaderRounds"));
    circuitHeaderAMRAP = localize(CFSTR("circuitHeaderAMRAP"));
    exerciseHeader = localize(CFSTR("exerciseHeader"));
    exerciseTitleRest = localize(CFSTR("exerciseTitleRest"));
    exerciseTitleReps = localize(CFSTR("exerciseTitleReps"));
    exerciseTitleRepsWithWeight = localize(CFSTR("exerciseTitleRepsWithWeight"));
    exerciseTitleDurationMinutes = localize(CFSTR("exerciseTitleDurationMinutes"));
    exerciseTitleDurationSeconds = localize(CFSTR("exerciseTitleDurationSeconds"));
    exerciseTitleDistance = localize(CFSTR("exerciseTitleDistance"));
}

static void handle_exercise_timer_interrupt(int n _U_) {}

static void handle_group_timer_interrupt(int n _U_) {}

static void *timer_loop(void *arg) {
    WorkoutTimer *t = arg;
    int res = 0, duration = 0;
    unsigned char eventType = 0;
    unsigned container = ExerciseTagNA, exercise = ExerciseTagNA;
    while (!t->info.stop) {
        pthread_mutex_lock(&t->lock);
        while (t->info.active != 1)
            pthread_cond_wait(&t->cond, &t->lock);
        duration = t->duration;
        container = t->container;
        exercise = t->exercise;
        eventType = t->info.type ? 0 : EventFinishGroup;
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

static void startWorkoutTimer(WorkoutTimer *t, int duration) {
    pthread_mutex_lock(&t->lock);
    t->refTime = time(NULL);
    t->duration = duration;
    t->info.stop = !duration;
    t->info.active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

static void scheduleNotification(int secondsFromNow, unsigned char type) {
    static int identifier = 0;
    int currentId = identifier++;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), currentId);

    id _obj = allocClass(objc_getClass("UNMutableNotificationContent"));
    id content = getObject(_obj, sel_getUid("init"));
    setString(content, sel_getUid("setTitle:"), notifTitle);
    setString(content, sel_getUid("setBody:"), notificationMessages[type]);
    id sound = staticMethod(objc_getClass("UNNotificationSound"), sel_getUid("defaultSound"));
    setObject(content, sel_getUid("setSound:"), sound);

    id trigger = ((id(*)(Class,SEL,double,bool))objc_msgSend)
    (objc_getClass("UNTimeIntervalNotificationTrigger"),
     sel_getUid("triggerWithTimeInterval:repeats:"), secondsFromNow, false);

    id req = ((id(*)(Class,SEL,CFStringRef,id,id))objc_msgSend)
    (objc_getClass("UNNotificationRequest"), sel_getUid("requestWithIdentifier:content:trigger:"),
     idString, content, trigger);

    id center = getNotificationCenter();
    ((void(*)(id,SEL,id,void(^)(id)))objc_msgSend)
    (center, sel_getUid("addNotificationRequest:withCompletionHandler:"),
     req, ^(id error _U_) {});
    CFRelease(idString);
    releaseObj(content);
}

static void cleanupWorkoutNotifications(id *observers) {
    if (!observers[0]) return;
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

static bool cycleExerciseEntry(Workout *w, WorkoutTimer *timers) {
    bool completed = false;
    ExerciseEntry *e = w->entry;
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) {
                startWorkoutTimer(&timers[TimerExercise], e->reps);
                scheduleNotification(e->reps, TimerExercise);
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
                completed = true;
            } else {
                e->state = ExerciseStateActive;
            }
        default: ;
    }
    return completed;
}

static void startGroup(Workout *w, WorkoutTimer *timers, bool startTimer) {
    timers[TimerGroup].container = timers[TimerExercise].container = w->index;
    timers[TimerExercise].exercise = w->group->index = 0;
    w->entry = &w->group->exercises->arr[0];

    Circuit *c = w->group;
    ExerciseEntry *e;
    array_iter(c->exercises, e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer) {
        int duration = 60 * c->reps;
        startWorkoutTimer(&timers[TimerGroup], duration);
        scheduleNotification(duration, TimerGroup);
    }

    cycleExerciseEntry(w, timers);
}

static CFStringRef createCircuitHeader(Circuit *c) {
    if (c->type == CircuitRounds && c->reps > 1) {
        int completed = c->completedReps == c->reps ? c->reps : c->completedReps + 1;
        return CFStringCreateWithFormat(NULL, NULL, circuitHeaderRounds, completed, c->reps);
    } else if (c->type == CircuitAMRAP) {
        return CFStringCreateWithFormat(NULL, NULL, circuitHeaderAMRAP, c->reps);
    }
    return NULL;
}

static void exerciseView_configure(id v) {
    StatusViewData *ptr = ((StatusViewData *) object_getIvar(v, StatusViewDataRef));
    ExerciseEntry *e = ptr->entry;

    if (e->sets > 1) {
        int completed = e->completedSets == e->sets ? e->sets : e->completedSets + 1;
        CFStringRef label = CFStringCreateWithFormat(NULL, NULL, exerciseHeader,
                                                     completed, e->sets);
        setLabelText(ptr->headerLabel, label);
        CFRelease(label);
    } else {
        setLabelText(ptr->headerLabel, NULL);
    }

    CFStringRef title;
    if (e->state == ExerciseStateResting) {
        title = CFStringCreateWithFormat(NULL, NULL, exerciseTitleRest, e->rest);
    } else {
        switch (e->type) {
            case ExerciseReps:
                if (e->weight > 1) {
                    title = CFStringCreateWithFormat(NULL, NULL, exerciseTitleRepsWithWeight,
                                                     e->name, e->reps, e->weight);
                } else {
                    title = CFStringCreateWithFormat(NULL, NULL, exerciseTitleReps,
                                                     e->name, e->reps);
                }
                break;

            case ExerciseDuration:
                if (e->reps > 120) {
                    title = CFStringCreateWithFormat(NULL, NULL, exerciseTitleDurationMinutes,
                                                     e->name, e->reps / 60.f);
                } else {
                    title = CFStringCreateWithFormat(NULL, NULL, exerciseTitleDurationSeconds,
                                                     e->name, e->reps);
                }
                break;

            default:
                title = CFStringCreateWithFormat(NULL, NULL, exerciseTitleDistance,
                                                 e->reps, (5 * e->reps) >> 2);
        }
    }
    setButtonTitle(ptr->button, title, 0);
    CFRelease(title);

    statusView_updateAccessibility(v, ExerciseStates[e->state]);

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

id workoutVC_init(void *delegate) {
    id self = createVC(WorkoutVCClass);
    WorkoutVCData *data = calloc(1, sizeof(WorkoutVCData));
    data->delegate = delegate;
    data->workout = ((AddWorkoutCoordinator *) delegate)->workout;

    pthread_mutex_init(&timerLock, NULL);
    for (int i = 0; i < 2; ++i) {
        memcpy(&data->timers[i], &(WorkoutTimer){.info = {.type = i}}, sizeof(WorkoutTimer));
        pthread_mutex_init(&data->timers[i].lock, NULL);
        pthread_cond_init(&data->timers[i].cond, NULL);
    }
    data->timers[TimerGroup].exercise = ExerciseTagNA;

    struct sigaction sa = {.sa_flags = 0, .sa_handler = handle_exercise_timer_interrupt};
    sigemptyset(&sa.sa_mask);
    sigaction(SignalExercise, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SignalGroup, &sa, NULL);
    exerciseTimerThread = &data->threads[TimerExercise];
    data->timers[0].parent = data->timers[1].parent = self;
    pthread_create(&data->threads[1], NULL, timer_loop, &data->timers[1]);
    pthread_create(&data->threads[0], NULL, timer_loop, &data->timers[0]);

    object_setIvar(self, WorkoutVCDataRef, (id) data);
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVCData *data = (WorkoutVCData *) object_getIvar(self, WorkoutVCDataRef);
    struct objc_super super = {self, objc_getClass("UIViewController")};

    cleanupWorkoutNotifications(data->observers);
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

    for (int i = 0; i < 10; ++i) {
        if (data->containers[i])
            releaseObj(data->containers[i]);
    }
    free(data);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    WorkoutVCData *data = (WorkoutVCData *) object_getIvar(self, WorkoutVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, data->workout->title);

    id stack = createStackView(NULL, 0, 1, 20, (Padding){20, 8, 20, 8});
    id startBtn = createButton(localize(CFSTR("start")), ColorGreen, 0, 0,
                                      self, sel_getUid("startEndWorkout:"), -1);

    SEL btnTap = sel_getUid("buttonTapped:");
    CGRect frame;
    getRect(view, &frame, 0);
    setNavButton(self, false, startBtn, frame.size.width);

    for (unsigned i = 0; i < data->workout->activities->size; ++i) {
        Circuit *c = &data->workout->activities->arr[i];
        CFStringRef header = createCircuitHeader(c);
        data->containers[i] = containerView_init(header, false, 0, false);
        addArrangedSubview(stack, data->containers[i]);

        ContainerViewData *cvData =
        (ContainerViewData *) object_getIvar(data->containers[i], ContainerViewDataRef);
        hideView(cvData->headerLabel, !header);
        if (header)
            CFRelease(header);

        for (unsigned j = 0; j < c->exercises->size; ++j) {
            id v = statusView_init(NULL, false, (i << 8) | j, self, btnTap);
            StatusViewData *ptr = (StatusViewData *) object_getIvar(v, StatusViewDataRef);
            ptr->entry = &c->exercises->arr[j];
            exerciseView_configure(v);
            containerView_add(data->containers[i], v);
        }
    }

    id scrollView = createScrollView();
    addSubview(view, scrollView);
    id guide = getLayoutGuide(view);
    pin(scrollView, guide, (Padding){0}, 0);
    addVStackToScrollView(stack, scrollView);

    releaseObj(scrollView);
    releaseObj(stack);

    data->first = (ContainerViewData *) object_getIvar(data->containers[0], ContainerViewDataRef);
    hideView(data->first->divider, true);
    SEL obsSig = sel_getUid("addObserverForName:object:queue:usingBlock:");
    id weakSelf = self;
    id main = staticMethod(objc_getClass("NSOperationQueue"), sel_getUid("mainQueue"));
    id center = getDeviceNotificationCenter();
    data->observers[0] = ((id(*)(id,SEL,CFStringRef,id,id,void(^)(id)))objc_msgSend)
    (center, obsSig, (CFStringRef) UIApplicationDidBecomeActiveNotification,
     nil, main, ^(id note _U_){ restartTimers(weakSelf); });
    data->observers[1] = ((id(*)(id,SEL,CFStringRef,id,id,void(^)(id)))objc_msgSend)
    (center, obsSig, (CFStringRef) UIApplicationWillResignActiveNotification,
     nil, main, ^(id note _U_){ stopTimers(weakSelf); });
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id btn) {
    WorkoutVCData *data = (WorkoutVCData *) object_getIvar(self, WorkoutVCDataRef);
    if (!getTag(btn)) {
        setButtonTitle(btn, localize(CFSTR("end")), 0);
        setButtonColor(btn, createColor(ColorRed), 0);
        setTag(btn, 1);
        data->workout->startTime = time(NULL);
        handleEvent(self, 0, 0, EventStartGroup);
    } else {
        bool valid = false;
        pthread_mutex_lock(&timerLock);
        if (data->workout) {
            valid = true;
            data->workout = NULL;
            cleanupWorkoutNotifications(data->observers);
        }
        pthread_mutex_unlock(&timerLock);
        if (valid)
            addWorkoutCoordinator_stoppedWorkout(data->delegate);
    }
}

void workoutVC_handleTap(id self, SEL _cmd _U_, id btn) {
    unsigned tag = (unsigned) getTag(btn);
    unsigned groupIdx = (tag & 0xff00) >> 8, exerciseIdx = tag & 0xff;
    handleEvent(self, groupIdx, exerciseIdx, 0);
}

void handleEvent(id self, unsigned gIdx, unsigned eIdx, unsigned char event) {
    bool finishedWorkout = false;
    WorkoutVCData *data = (WorkoutVCData *) object_getIvar(self, WorkoutVCDataRef);
    pthread_mutex_lock(&timerLock);
    Workout *w = data->workout;
    if (!w) {
        goto cleanup;
    } else if (gIdx != w->index || eIdx != w->group->index) {
        if (event != EventFinishGroup || gIdx != w->index)
            goto cleanup;
    }

    id v = data->first->views->arr[w->group->index];
    StatusViewData *ptr = ((StatusViewData *) object_getIvar(v, StatusViewDataRef));
    unsigned char t = 0;
    if (event) {
        t = TransitionFinishedCircuit;
        if (event == EventFinishGroup) {
            if (++w->index == w->activities->size) {
                t = TransitionCompletedWorkout;
                goto foundTransition;
            }
            ++w->group;
            t = TransitionFinishedCircuitDeleteFirst;
        }
        startGroup(w, data->timers, true);
        goto foundTransition;
    }

    if (w->entry->type == ExerciseDuration && w->entry->state == ExerciseStateActive &&
        !getBool(ptr->button, sel_getUid("isUserInteractionEnabled"))) {
        toggleInteraction(ptr->button, true);
        if (w->type == WorkoutEndurance) {
            goto foundTransition;
        }
    }

    bool exerciseDone = cycleExerciseEntry(w, data->timers);
    exerciseView_configure(v);

    if (exerciseDone) {
        t = TransitionFinishedExercise;
        ++w->entry;
        if (++w->group->index == w->group->exercises->size) {
            t = TransitionFinishedCircuit;
            Circuit *c = w->group;
            bool finishedCircuit = false;
            switch (c->type) {
                case CircuitRounds:
                    if (++c->completedReps == c->reps)
                        finishedCircuit = true;
                    break;

                case CircuitDecrement:
                    if (--c->completedReps == 0) {
                        finishedCircuit = true;
                    } else {
                        ExerciseEntry *e;
                        array_iter(c->exercises, e) {
                            if (e->type == ExerciseReps)
                                e->reps -= 1;
                        }
                    }

                default: ;
            }

            if (finishedCircuit) {
                if (++w->index == w->activities->size) {
                    t = TransitionCompletedWorkout;
                } else {
                    t = TransitionFinishedCircuitDeleteFirst;
                    ++w->group;
                    startGroup(w, data->timers, true);
                }
            } else {
                startGroup(w, data->timers, false);
            }
        } else {
            cycleExerciseEntry(w, data->timers);
        }
    }
foundTransition: ;

    switch (t) {
        case TransitionCompletedWorkout:
            finishedWorkout = true;
            data->workout = NULL;
            cleanupWorkoutNotifications(data->observers);
            break;

        case TransitionFinishedCircuitDeleteFirst:
            data->first =
            (ContainerViewData *) object_getIvar(data->containers[w->index], ContainerViewDataRef);
            removeView(data->containers[w->index - 1]);
            releaseObj(data->containers[w->index - 1]);
            data->containers[w->index - 1] = nil;

        case TransitionFinishedCircuit: ;
            hideView(data->first->divider, true);
            CFStringRef header = createCircuitHeader(w->group);
            setLabelText(data->first->headerLabel, header);
            if (header)
                CFRelease(header);

            for (unsigned i = 0; i < w->group->exercises->size; ++i) {
                exerciseView_configure(data->first->views->arr[i]);
            }
            break;

        case TransitionFinishedExercise:
            v = data->first->views->arr[w->group->index];
            exerciseView_configure(v);
        default:
            break;
    }

cleanup:
    pthread_mutex_unlock(&timerLock);
    if (finishedWorkout)
        addWorkoutCoordinator_completedWorkout(data->delegate, false, true, NULL);
}

void stopTimers(id self) {
    WorkoutVCData *data = (WorkoutVCData *) object_getIvar(self, WorkoutVCDataRef);
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
        data->savedInfo.exerciseInfo.group = data->savedInfo.exerciseInfo.tag = ExerciseTagNA;
    }
    pthread_mutex_unlock(&timerLock);
}

void restartTimers(id self) {
    bool endExercise = false, endGroup = false;
    unsigned groupIdx = 0, exerciseIdx = 0;
    WorkoutVCData *data = (WorkoutVCData *) object_getIvar(self, WorkoutVCDataRef);
    pthread_mutex_lock(&timerLock);
    Workout *w = data->workout;
    if (w) {
        time_t now = time(NULL);
        unsigned group = data->savedInfo.exerciseInfo.group;
        exerciseIdx = data->savedInfo.exerciseInfo.tag;
        if (group != ExerciseTagNA && w->index == group && w->group->index == exerciseIdx) {
            ExerciseEntry *e = &w->group->exercises->arr[exerciseIdx];
            if (e->type == ExerciseDuration) {
                int diff = (int) (now - data->timers[TimerExercise].refTime);
                if (diff >= data->timers[TimerExercise].duration) {
                    endExercise = true;
                    groupIdx = group;
                } else {
                    startWorkoutTimer(&data->timers[TimerExercise],
                                      data->timers[TimerExercise].duration - diff);
                }
            }
        }

        group = data->savedInfo.groupTag;
        if (group != ExerciseTagNA && w->index == group && w->group->type == CircuitAMRAP) {
            int diff = (int) (now - data->timers[TimerGroup].refTime);
            if (diff >= data->timers[TimerGroup].duration) {
                endGroup = true;
                groupIdx = group;
            } else {
                startWorkoutTimer(&data->timers[TimerGroup],
                                  data->timers[TimerGroup].duration - diff);
            }
        }
    }
    pthread_mutex_unlock(&timerLock);

    if (endExercise)
        handleEvent(self, groupIdx, exerciseIdx, 0);
    if (endGroup)
        handleEvent(self, groupIdx, 0, EventFinishGroup);
}
