//
//  Circuit.c
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#include "Circuit.h"

#define freeCircuit(x) array_free(exEntry, (x).exercises)

static CFStringRef circuitHeaderRounds;
static CFStringRef circuitHeaderAMRAP;

gen_array_source(circuit, Circuit, DSDefault_shallowCopy, freeCircuit)

void initCircuitStrings(void) {
    circuitHeaderRounds = localize(CFSTR("circuitHeaderRounds"));
    circuitHeaderAMRAP = localize(CFSTR("circuitHeaderAMRAP"));
}

CFStringRef circuit_createHeader(Circuit *c) {
    if (c->type == CircuitRounds && c->reps > 1) {
        int completed = c->completedReps == c->reps ? c->reps : c->completedReps + 1;
        return CFStringCreateWithFormat(NULL, NULL, circuitHeaderRounds, completed, c->reps);
    } else if (c->type == CircuitAMRAP) {
        return CFStringCreateWithFormat(NULL, NULL, circuitHeaderAMRAP, c->reps);
    }
    return NULL;
}

void circuit_start(Circuit *c, bool startTimer) {
    ExerciseEntry *e;
    array_iter(c->exercises, e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer) {
        int duration = 60 * c->reps;
        startWorkoutTimer(c->timer, duration);
        scheduleNotification(duration, TimerGroup);
    }
}

bool circuit_didFinish(Circuit *c) {
    bool isDone = false;
    switch (c->type) {
        case CircuitRounds:
            if (++c->completedReps == c->reps)
                isDone = true;
            break;

        case CircuitDecrement:
            if (--c->completedReps == 0) {
                isDone = true;
            } else {
                ExerciseEntry *e;
                array_iter(c->exercises, e) {
                    if (e->type == ExerciseReps)
                        e->reps -= 1;
                }
            }

        default:
            break;
    }
    return isDone;
}
