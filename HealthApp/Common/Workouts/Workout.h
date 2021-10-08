//
//  Workout.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef Workout_h
#define Workout_h

#include "Circuit.h"
#include <signal.h>

enum {
    LiftSquat,
    LiftPullup,
    LiftBench,
    LiftDeadlift
};

enum {
    SignalGroup = SIGUSR2,
    SignalExercise = SIGUSR1
};

typedef enum {
    WorkoutStrength,
    WorkoutSE,
    WorkoutEndurance,
    WorkoutHIC,
    WorkoutRest
} WorkoutType;

typedef struct {
    WorkoutType type;
    signed char day;
    unsigned index;
    time_t startTime;
    int16_t duration;
    short *newLifts;
    CFStringRef title;
    Circuit *group;
    ExerciseEntry *entry;
    Array_circuit *activities;
    WorkoutTimer timers[2];
    pthread_t threads[2];
    struct savedInfo {
        unsigned groupTag;
        struct exerciseInfo {
            unsigned group;
            unsigned tag;
        } exerciseInfo;
    } savedInfo;
} Workout;

typedef enum {
    TransitionCompletedWorkout,
    TransitionFinishedCircuitDeleteFirst,
    TransitionFinishedCircuit,
    TransitionFinishedExercise,
    TransitionNoChange
} WorkoutTransition;

typedef enum {
    EventNone,
    EventStartGroup,
    EventFinishGroup,
} CircuitEvent;

extern const unsigned ExerciseTagNA;

void workout_setDuration(Workout *w);

WorkoutTransition workout_findTransitionForEvent(Workout *w, id view, id btn, CircuitEvent option);
void workout_stopTimers(Workout *w);
bool workout_restartExerciseTimer(Workout *w, time_t refTime);
bool workout_restartGroupTimer(Workout *w, time_t refTime);

#endif /* Workout_h */
