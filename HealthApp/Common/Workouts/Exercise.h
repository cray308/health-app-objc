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

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index);
CFStringRef *exerciseManager_getWorkoutNamesForType(unsigned char type, int *size);
Workout *exerciseManager_getWorkoutFromLibrary(unsigned char type,
                                               int index, int reps, int sets, int weight);

#endif /* Exercise_h */
