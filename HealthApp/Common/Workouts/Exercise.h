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

void workout_setup_activities(Workout *w);
void workout_add_activity(Workout *w, ExerciseGroup *g);
void workout_free(Workout *w);
int workout_getNumberOfActivities(Workout *w);
void workout_setSetsAndRepsForExercises(Workout *w, unsigned int sets, unsigned int reps);
void workout_setWeightsForExercises(Workout *w, unsigned int *weights, int size);
ExerciseGroup *workout_getExerciseGroup(Workout *w, int i);

void exerciseGroup_setup_exercises(ExerciseGroup *g);
void exerciseGroup_add_exercise(ExerciseGroup *g, ExerciseEntry *e);
int exerciseGroup_getNumberOfExercises(ExerciseGroup *g);
ExerciseEntry *exerciseGroup_getExercise(ExerciseGroup *g, int i);

#endif /* Exercise_h */
