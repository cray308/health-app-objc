//
//  ExerciseEntry.h
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#ifndef ExerciseEntry_h
#define ExerciseEntry_h

#include "CocoaHelpers.h"
#include "WorkoutTimer.h"

typedef struct {
    enum {
        ExerciseReps,
        ExerciseDuration,
        ExerciseDistance
    } type;
    int weight;
    int reps;
    int sets;
    int rest;
    int completedSets;
    enum {
        ExerciseStateDisabled,
        ExerciseStateActive,
        ExerciseStateResting,
        ExerciseStateCompleted
    } state;
    CFStringRef name;
    WorkoutTimer *timer;
} ExerciseEntry;

gen_array_headers(exEntry, ExerciseEntry)

CFStringRef exerciseEntry_createSetsTitle(ExerciseEntry *e);
CFStringRef exerciseEntry_createTitle(ExerciseEntry *e);
bool exerciseEntry_cycle(ExerciseEntry *e);

#endif /* ExerciseEntry_h */
