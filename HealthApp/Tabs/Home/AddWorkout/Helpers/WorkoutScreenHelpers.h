//
//  WorkoutScreenHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef WorkoutScreenHelpers_h
#define WorkoutScreenHelpers_h

#include "CocoaHelpers.h"
#include "Exercise.h"

typedef void (^ObserverCallback)(id);

typedef enum {
    TimerSignalGroup = SIGUSR2,
    TimerSignalExercise = SIGUSR1
} TimerSignal;

typedef enum {
    TimerTypeGroup,
    TimerTypeExercise
} TimerType;

typedef enum {
    TransitionCompletedWorkout,
    TransitionFinishedCircuitDeleteFirst,
    TransitionFinishedCircuit,
    TransitionFinishedExercise,
    TransitionNoChange
} WorkoutTransition;

typedef enum {
    EventOptionNone,
    EventOptionStartGroup,
    EventOptionFinishGroup,
} WorkoutEventOption;

extern pthread_mutex_t timerLock;
extern const uint ExerciseTagNA;

void setupTimers(Workout *w, id parent);
id createDeviceEventNotification(id name, ObserverCallback block);
void cleanupWorkoutNotifications(id *observers);
void startWorkoutTimer(WorkoutTimer *t, int duration, unsigned container, unsigned exercise);

WorkoutTransition workout_findTransitionForEvent(Workout *w, id view, id btn, uint option);
void workout_stopTimers(Workout *w);
bool workout_restartExerciseTimer(Workout *w, time_t refTime);
bool workout_restartGroupTimer(Workout *w, time_t refTime);

#endif /* WorkoutScreenHelpers_h */
