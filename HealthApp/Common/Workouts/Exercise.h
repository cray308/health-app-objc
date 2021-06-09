//
//  Exercise.h
//  HealthApp
//
//  Created by Christopher Ray on 6/6/21.
//

#ifndef Exercise_h
#define Exercise_h

#include "array.h"
#import <Foundation/Foundation.h>

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

typedef enum {
    FitnessPlanBaseBuilding,
    FitnessPlanContinuation
} FitnessPlan;



#define freeExerciseEntry(x) [(x).name release]
#define freeExerciseGroup(x) array_free(exEntry, (x).exercises)
#define freeWorkout(x) do { array_free(exGroup, (x).activities); [(x).title release]; } while (0)

typedef struct {
    unsigned char type;
    unsigned int weight;
    unsigned int reps;
    unsigned int sets;
    unsigned int rest;
    unsigned int completedSets;
    NSString *name;
} ExerciseEntry;

gen_array(exEntry, ExerciseEntry, DSDefault_shallowCopy, freeExerciseEntry)

typedef struct {
    unsigned char type;
    unsigned int reps;
    unsigned int completedReps;
    Array_exEntry *exercises;
} ExerciseGroup;

gen_array(exGroup, ExerciseGroup, DSDefault_shallowCopy, freeExerciseGroup)

typedef struct {
    unsigned char type;
    unsigned int weight;
    unsigned int sets;
    unsigned int reps;
    unsigned int completedReps;
    unsigned int day;
    NSString *title;
    Array_exGroup *activities;
} Workout;

gen_array(workout, Workout, DSDefault_shallowCopy, freeWorkout)

#endif /* Exercise_h */
