//
//  Workout.c
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#include "Workout.h"
#include "CocoaHelpers.h"
#include "Views.h"

const unsigned ExerciseTagNA = 255;

extern void exerciseView_configure(id v, ExerciseEntry *e);

static inline void startGroup(Workout *w, bool startTimer) {
    w->timers[TimerGroup].container = w->timers[TimerExercise].container = w->index;
    w->timers[TimerExercise].exercise = w->group->index = 0;
    w->entry = &w->group->exercises->arr[0];
    circuit_start(w->group, startTimer);
    exerciseEntry_cycle(w->entry);
}

WorkoutTransition workout_findTransitionForEvent(Workout *w, id view, id btn, CircuitEvent option) {
    WorkoutTransition t = TransitionNoChange;
    if (option) {
        t = TransitionFinishedCircuit;
        if (option == EventFinishGroup) {
            if (++w->index == w->activities->size) return TransitionCompletedWorkout;
            ++w->group;
            t = TransitionFinishedCircuitDeleteFirst;
        }
        startGroup(w, true);
        return t;
    }

    if (w->entry->type == ExerciseDuration && w->entry->state == ExerciseStateActive &&
        !getBool(btn, sel_getUid("isUserInteractionEnabled"))) {
        enableInteraction(btn, true);
        if (w->type == WorkoutEndurance) return TransitionNoChange;
    }

    bool exerciseDone = exerciseEntry_cycle(w->entry);
    exerciseView_configure(view, w->entry);

    if (exerciseDone) {
        t = TransitionFinishedExercise;
        ++w->entry;
        if (++w->group->index == w->group->exercises->size) {
            t = TransitionFinishedCircuit;
            if (circuit_didFinish(w->group)) {
                if (++w->index == w->activities->size) {
                    t = TransitionCompletedWorkout;
                } else {
                    t = TransitionFinishedCircuitDeleteFirst;
                    ++w->group;
                    startGroup(w, true);
                }
            } else {
                startGroup(w, false);
            }
        } else {
            exerciseEntry_cycle(w->entry);
        }
    }
    return t;
}

void workout_stopTimers(Workout *w) {
    if (w->timers[TimerGroup].info.active == 1) {
        w->savedInfo.groupTag = w->timers[TimerGroup].container;
        pthread_kill(w->threads[TimerGroup], SignalGroup);
    } else {
        w->savedInfo.groupTag = ExerciseTagNA;
    }

    if (w->timers[TimerExercise].info.active == 1) {
        w->savedInfo.exerciseInfo.group = w->timers[TimerExercise].container;
        w->savedInfo.exerciseInfo.tag = w->timers[TimerExercise].exercise;
        pthread_kill(w->threads[TimerExercise], SignalExercise);
    } else {
        w->savedInfo.exerciseInfo.group = w->savedInfo.exerciseInfo.tag = ExerciseTagNA;
    }
}

bool workout_restartExerciseTimer(Workout *w, time_t refTime) {
    unsigned group = w->savedInfo.exerciseInfo.group;
    bool endExercise = false, timerActive = group != ExerciseTagNA;
    unsigned index = w->savedInfo.exerciseInfo.tag;

    if (timerActive && w->index == group && w->group->index == index) {
        ExerciseEntry *e = &w->group->exercises->arr[index];
        if (e->type == ExerciseDuration) {
            int diff = (int) (refTime - w->timers[TimerExercise].refTime);
            if (diff >= w->timers[TimerExercise].duration) {
                endExercise = true;
            } else {
                startWorkoutTimer(&w->timers[TimerExercise],
                                  w->timers[TimerExercise].duration - diff);
            }
        }
    }
    return endExercise;
}

bool workout_restartGroupTimer(Workout *w, time_t refTime) {
    unsigned group = w->savedInfo.groupTag;
    bool endGroup = false, timerActive = group != ExerciseTagNA;

    if (timerActive && w->index == group && w->group->type == CircuitAMRAP) {
        int diff = (int) (refTime - w->timers[TimerGroup].refTime);
        if (diff >= w->timers[TimerGroup].duration) {
            endGroup = true;
        } else {
            startWorkoutTimer(&w->timers[TimerGroup], w->timers[TimerGroup].duration - diff);
        }
    }
    return endGroup;
}

void workout_setDuration(Workout *w) {
    w->duration = ((int16_t) ((time(NULL) - w->startTime) / 60.f)) + 1;
#if DEBUG
    w->duration *= 10;
#endif
}
