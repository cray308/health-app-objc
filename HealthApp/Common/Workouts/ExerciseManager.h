#ifndef ExerciseManager_h
#define ExerciseManager_h

#include <CoreFoundation/CFArray.h>
#include "array.h"

enum {
    LiftSquat,
    LiftPullup,
    LiftBench,
    LiftDeadlift
};

enum {
    WorkoutStrength,
    WorkoutSE,
    WorkoutEndurance,
    WorkoutHIC,
    WorkoutRest
};

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
} ExerciseEntry;

gen_array_headers(exEntry, ExerciseEntry)

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
} Circuit;

gen_array_headers(circuit, Circuit)

typedef struct {
    unsigned char type;
    signed char day;
    unsigned index;
    time_t startTime;
    int16_t duration;
    CFStringRef title;
    Circuit *group;
    ExerciseEntry *entry;
    Array_circuit *activities;
} Workout;

typedef struct __workoutParams {
    const signed char day;
    unsigned char type;
    int index;
    int sets;
    int reps;
    int weight;
} WorkoutParams;

void workoutParams_init(WorkoutParams *this, signed char day);

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index);
CFArrayRef exerciseManager_createWorkoutNames(unsigned char type);
Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params);

#endif /* ExerciseManager_h */
