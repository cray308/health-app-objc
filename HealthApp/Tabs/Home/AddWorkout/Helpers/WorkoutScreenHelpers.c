//
//  WorkoutScreenHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#include "WorkoutScreenHelpers.h"

pthread_mutex_t timerLock;
static pthread_t *exerciseTimerThread;

static void handle_exercise_timer_interrupt(int n _U_) {}
static void handle_group_timer_interrupt(int n _U_) {}

static void *timer_loop(void *arg) {
    WorkoutTimer *t = arg;
    int res = 0, duration = 0;
    unsigned container = 255, exercise = 255;
    while (!t->info.stop) {
        pthread_mutex_lock(&t->lock);
        while (t->info.active != 1)
            pthread_cond_wait(&t->cond, &t->lock);
        duration = t->duration;
        container = t->container;
        exercise = t->exercise;
        pthread_mutex_unlock(&t->lock);

        if (!duration) continue;
        res = sleep(duration);
        t->info.active = 2;
        if (!res) {
            id parent = t->parent;
            if (t->info.type == 0 && t[1].info.active == 1)
                pthread_kill(*exerciseTimerThread, TimerSignalExercise);
            dispatch_async(dispatch_get_main_queue(), ^(void) {
                ((void(*)(id,SEL,unsigned char,unsigned,unsigned))objc_msgSend)
                (parent, sel_getUid("finishedWorkoutTimerForType:group:entry:"),
                 t->info.type, container, exercise);
            });
        }
    }
    return NULL;
}

static void scheduleNotification(int secondsFromNow, CFStringRef message) {
    static int identifier = 0;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), identifier++);

    id content = ((id(*)(id,SEL))objc_msgSend)(allocClass("UNMutableNotificationContent"),
                                               sel_getUid("init"));
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(content, sel_getUid("setTitle:"),
                                                CFSTR("Workout Update"));
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(content, sel_getUid("setSubtitle:"), message);
    id sound = objc_staticMethod(objc_getClass("UNNotificationSound"), sel_getUid("defaultSound"));
    ((void(*)(id,SEL,id))objc_msgSend)(content, sel_getUid("setSound:"), sound);

    id trigger = ((id(*)(Class,SEL,double,bool))objc_msgSend)
    (objc_getClass("UNTimeIntervalNotificationTrigger"),
     sel_getUid("triggerWithTimeInterval:repeats:"), secondsFromNow, false);

    id req = ((id(*)(Class,SEL,CFStringRef,id,id))objc_msgSend)
    (objc_getClass("UNNotificationRequest"), sel_getUid("requestWithIdentifier:content:trigger:"),
     idString, content, trigger);

    ((void(*)(id,SEL,id,void(^)(id)))objc_msgSend)
    (getNotificationCenter(), sel_getUid("addNotificationRequest:withCompletionHandler:"),
     req, ^(id error _U_) {});
    CFRelease(idString);
    releaseObj(content);
}

static id getDeviceNotificationCenter(void) {
    return objc_staticMethod(objc_getClass("NSNotificationCenter"), sel_getUid("defaultCenter"));
}

static void removeNotificationObserver(id center, id observer) {
    ((void(*)(id,SEL,id))objc_msgSend)(center, sel_getUid("removeObserver:"), observer);
}

static bool cycleCurrentEntry(Workout *w) {
    ExerciseEntry *e = w->entry;
    bool completed = false;
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseTypeDuration) {
                startWorkoutTimer(&w->timers[TimerTypeExercise],
                                  e->reps, w->index, w->group->index);
                scheduleNotification(e->reps, CFSTR("Finished exercise!"));
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
                completed = true;
            } else {
                e->state = ExerciseStateActive;
            }

        default:
            break;
    }
    return completed;
}

static bool finishedExerciseGroup(ExerciseGroup *g) {
    bool isDone = false;
    switch (g->type) {
        case ExerciseContainerTypeRounds:
            if (++g->completedReps == g->reps)
                isDone = true;
            break;

        case ExerciseContainerTypeDecrement:
            if (--g->completedReps == 0) {
                isDone = true;
            } else {
                ExerciseEntry *e;
                array_iter(g->exercises, e) {
                    if (e->type == ExerciseTypeReps)
                        e->reps -= 1;
                }
            }

        default:
            break;
    }
    return isDone;
}

void setupTimers(Workout *w, id parent) {
    struct sigaction sa = {.sa_flags = 0, .sa_handler = handle_exercise_timer_interrupt};
    sigemptyset(&sa.sa_mask);
    sigaction(TimerSignalExercise, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(TimerSignalGroup, &sa, NULL);
    exerciseTimerThread = &w->threads[TimerTypeExercise];
    w->timers[0].parent = w->timers[1].parent = parent;
    pthread_create(&w->threads[1], NULL, timer_loop, &w->timers[1]);
    pthread_create(&w->threads[0], NULL, timer_loop, &w->timers[0]);
}

id createDeviceEventNotification(id name, ObserverCallback block) {
    id main = objc_staticMethod(objc_getClass("NSOperationQueue"), sel_getUid("mainQueue"));
    return ((id(*)(id,SEL,CFStringRef,id,id,void(^)(id)))objc_msgSend)
    (getDeviceNotificationCenter(), sel_getUid("addObserverForName:object:queue:usingBlock:"),
     (CFStringRef) name, nil, main, block);
}

void cleanupWorkoutNotifications(id *observers) {
    id center = getDeviceNotificationCenter();
    removeNotificationObserver(center, observers[0]);
    removeNotificationObserver(center, observers[1]);
    center = getNotificationCenter();
    objc_singleArg(center, sel_getUid("removeAllPendingNotificationRequests"));
    objc_singleArg(center, sel_getUid("removeAllDeliveredNotifications"));
    exerciseTimerThread = NULL;
}

void startWorkoutTimer(WorkoutTimer *t, int duration, unsigned container, unsigned exercise) {
    pthread_mutex_lock(&t->lock);
    t->refTime = time(NULL);
    t->duration = duration;
    t->container = container;
    t->exercise = exercise;
    t->info.stop = !duration;
    t->info.active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

static void startGroup(Workout *w, bool startTimer) {
    w->timers[TimerTypeExercise].container = w->index;
    w->group->index = 0;
    w->entry = &w->group->exercises->arr[0];
    ExerciseEntry *e;
    array_iter(w->group->exercises, e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (w->group->type == ExerciseContainerTypeAMRAP && startTimer) {
        int duration = 60 * w->group->reps;
        startWorkoutTimer(&w->timers[TimerTypeGroup], duration, w->index, 255);
        scheduleNotification(duration, CFSTR("Finished AMRAP circuit!"));
    }
}

WorkoutTransition workout_findTransitionForEvent(Workout *w, id view, id btn, uint option) {
    WorkoutTransition t = TransitionNoChange;
    if (option) {
        t = TransitionFinishedCircuit;
        if (option == EventOptionFinishGroup) {
            if (++w->index == w->activities->size)
                return TransitionCompletedWorkout;
            ++w->group;
            t = TransitionFinishedCircuitDeleteFirst;
        }
        startGroup(w, true);
        cycleCurrentEntry(w);
        return t;
    }

    if (w->entry->type == ExerciseTypeDuration && w->entry->state == ExerciseStateActive &&
        !((bool(*)(id,SEL))objc_msgSend)(btn, sel_getUid("isUserInteractionEnabled"))) {
        ((void(*)(id,SEL,bool))objc_msgSend)(btn, sel_getUid("setUserInteractionEnabled:"), true);
        if (w->type == WorkoutTypeEndurance)
            return TransitionNoChange;
    }

    bool exerciseDone = cycleCurrentEntry(w);
    ((void(*)(id,SEL,ExerciseEntry*))objc_msgSend)(view,
                                                   sel_getUid("configureWithEntry:"), w->entry);

    if (exerciseDone) {
        t = TransitionFinishedExercise;
        ++w->entry;
        if (++w->group->index == w->group->exercises->size) {
            t = TransitionFinishedCircuit;
            if (finishedExerciseGroup(w->group)) {
                if (++w->index == w->activities->size) {
                    t = TransitionCompletedWorkout;
                } else {
                    t = TransitionFinishedCircuitDeleteFirst;
                    ++w->group;
                    startGroup(w, true);
                    cycleCurrentEntry(w);
                }
            } else {
                startGroup(w, false);
                cycleCurrentEntry(w);
            }
        } else {
            cycleCurrentEntry(w);
        }
    }

    return t;
}

void workout_stopTimers(Workout *w) {
    if (w->timers[TimerTypeGroup].info.active == 1) {
        w->savedInfo.groupTag = w->timers[TimerTypeGroup].container;
        pthread_kill(w->threads[TimerTypeGroup], TimerSignalGroup);
    } else {
        w->savedInfo.groupTag = 255;
    }

    if (w->timers[TimerTypeExercise].info.active == 1) {
        w->savedInfo.exerciseInfo.group = w->timers[TimerTypeExercise].container;
        w->savedInfo.exerciseInfo.tag = w->timers[TimerTypeExercise].exercise;
        pthread_kill(w->threads[TimerTypeExercise], TimerSignalExercise);
    } else {
        w->savedInfo.exerciseInfo.group = w->savedInfo.exerciseInfo.tag = 255;
    }
}

bool workout_restartExerciseTimer(Workout *w, time_t refTime) {
    unsigned group = w->savedInfo.exerciseInfo.group;
    bool endExercise = false, timerActive = group != 255;
    unsigned index = w->savedInfo.exerciseInfo.tag;

    if (timerActive && w->index == group && w->group->index == index) {
        ExerciseEntry *e = &w->group->exercises->arr[index];
        if (e->type == ExerciseTypeDuration) {
            int diff = (int) (refTime - w->timers[TimerTypeExercise].refTime);
            if (diff >= w->timers[TimerTypeExercise].duration) {
                endExercise = true;
            } else {
                startWorkoutTimer(&w->timers[TimerTypeExercise],
                                  w->timers[TimerTypeExercise].duration - diff, group, index);
            }
        }
    }
    return endExercise;
}

bool workout_restartGroupTimer(Workout *w, time_t refTime) {
    unsigned group = w->savedInfo.groupTag;
    bool endGroup = false, timerActive = group != 255;
    
    if (timerActive && w->index == group && w->group->type == ExerciseContainerTypeAMRAP) {
        int diff = (int) (refTime - w->timers[TimerTypeGroup].refTime);
        if (diff >= w->timers[TimerTypeGroup].duration) {
            endGroup = true;
        } else {
            startWorkoutTimer(&w->timers[TimerTypeGroup],
                              w->timers[TimerTypeGroup].duration - diff, group, 255);
        }
    }
    return endGroup;
}