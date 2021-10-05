//
//  Circuit.h
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#ifndef Circuit_h
#define Circuit_h

#include "ExerciseEntry.h"

typedef struct {
    enum {
        CircuitRounds,
        CircuitAMRAP,
        CircuitDecrement
    } type;
    int reps;
    int completedReps;
    unsigned index;
    Array_exEntry *exercises;
    WorkoutTimer *timer;
} Circuit;

gen_array_headers(circuit, Circuit)

CFStringRef circuit_createHeader(Circuit *c);
void circuit_start(Circuit *c, bool startTimer);
bool circuit_didFinish(Circuit *c);

#endif /* Circuit_h */
