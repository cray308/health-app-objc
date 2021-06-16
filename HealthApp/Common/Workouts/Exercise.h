//
//  Exercise.h
//  HealthApp
//
//  Created by Christopher Ray on 6/6/21.
//

#ifndef Exercise_h
#define Exercise_h

#include <CoreFoundation/CoreFoundation.h>

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

typedef struct ExerciseEntry ExerciseEntry;
typedef struct Array_exEntry Array_exEntry;
typedef struct ExerciseGroup ExerciseGroup;
typedef struct Array_exGroup Array_exGroup;
typedef struct Workout Workout;

struct ExerciseEntry {
    unsigned char type;
    unsigned int weight;
    unsigned int reps;
    unsigned int sets;
    unsigned int rest;
    unsigned int completedSets;
    CFStringRef name;
};

struct ExerciseGroup {
    unsigned char type;
    unsigned int reps;
    unsigned int completedReps;
    Array_exEntry *exercises;
};

struct Workout {
    unsigned char type;
    signed char day;
    CFStringRef title;
    Array_exGroup *activities;
};

void workout_buildFromDictionary(CFDictionaryRef dict, unsigned int index, unsigned char type, unsigned int sets, unsigned int reps, unsigned int weight, Workout *w);

void workout_free(Workout *w);
int workout_getNumberOfActivities(Workout *w);
ExerciseGroup *workout_getExerciseGroup(Workout *w, int i);

int exerciseGroup_getNumberOfExercises(ExerciseGroup *g);
ExerciseEntry *exerciseGroup_getExercise(ExerciseGroup *g, int i);

CFStringRef exercise_createTitleString(ExerciseEntry *e);
CFStringRef exercise_createSetsString(ExerciseEntry *e);
CFStringRef exerciseGroup_createHeaderText(ExerciseGroup *g);

#endif /* Exercise_h */
