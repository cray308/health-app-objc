#ifndef WorkoutVC_Ext_h
#define WorkoutVC_Ext_h

#include "ExerciseManager.h"
#include "StatusCell.h"
#include "WorkoutTimer.h"

enum {
    ExerciseStateDisabled,
    ExerciseStateActive,
    ExerciseStateResting,
    ExerciseStateCompleted = 4
};

enum {
    TimerCircuit,
    TimerExercise
};

enum {
    TransitionCompletedWorkout = 1,
    TransitionFinishedCircuit,
    TransitionFinishedCircuitDeleteFirst,
    TransitionFinishedExercise
};

void initNotificationCache(void);

void workoutTimer_start(WorkoutTimer *t, int duration, bool scheduleNotification);
bool exercise_cycle(Exercise *e, WorkoutTimer *timers);
void statusCell_configure(StatusCell *v, Exercise *exercise);
void circuit_start(Circuit *c, WorkoutTimer *timers, bool startTimer);
int circuit_increment(Circuit *c, WorkoutTimer *timers, Exercise *exercise);
int workout_increment(Workout *w, WorkoutTimer *timers);
bool workout_isCompleted(Workout *w);

static inline void workout_setDuration(Workout *w) {
    w->duration = ((int)((time(NULL) - w->startTime) / 60.f)) + 1;
#if TARGET_OS_SIMULATOR
    w->duration += 1;
    w->duration *= 10;
#endif
}

#endif /* WorkoutVC_Ext_h */
