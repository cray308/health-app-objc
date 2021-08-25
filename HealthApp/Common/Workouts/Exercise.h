//
//  Exercise.h
//  HealthApp
//
//  Created by Christopher Ray on 6/6/21.
//

#ifndef Exercise_h
#define Exercise_h

#include <CoreFoundation/CoreFoundation.h>
#include "array.h"

typedef enum {
    LiftTypeSquat,
    LiftTypePullup,
    LiftTypeBench,
    LiftTypeDeadlift
} LiftType;

typedef enum {
    ExerciseTypeReps,
    ExerciseTypeDuration,
    ExerciseTypeDistance
} ExerciseType;

typedef enum {
    ExerciseContainerTypeRounds,
    ExerciseContainerTypeAMRAP,
    ExerciseContainerTypeDecrement
} ExerciseContainerType;

typedef enum {
    WorkoutTypeStrength,
    WorkoutTypeSE,
    WorkoutTypeEndurance,
    WorkoutTypeHIC,
    WorkoutTypeRest
} WorkoutType;

gen_array_headers(str, CFStringRef)

typedef struct {
    unsigned char type;
    int weight;
    int reps;
    int sets;
    int rest;
    int completedSets;
    CFStringRef name;
} ExerciseEntry;

gen_array_headers(exEntry, ExerciseEntry)

typedef struct {
    unsigned char type;
    int reps;
    int completedReps;
    Array_exEntry *exercises;
} ExerciseGroup;

gen_array_headers(exGroup, ExerciseGroup)

typedef struct {
    unsigned char type;
    signed char day;
    CFStringRef title;
    Array_exGroup *activities;
} Workout;

static inline CFStringRef exerciseEntry_createSetsTitle(ExerciseEntry *e) {
    return CFStringCreateWithFormat(NULL, NULL, CFSTR("Set %d of %d"),
                                    e->completedSets + 1, e->sets);
}

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index);
Array_str *exerciseManager_getWorkoutNamesForType(unsigned char type);
Workout *exerciseManager_getWorkoutFromLibrary(unsigned char type,
                                               int index, int sets, int reps, int weight);

CFStringRef exerciseGroup_createHeader(ExerciseGroup *g);
CFStringRef exerciseEntry_createTitle(ExerciseEntry *e);

#endif /* Exercise_h */
