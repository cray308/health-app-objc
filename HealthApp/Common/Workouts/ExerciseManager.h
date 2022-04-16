#ifndef ExerciseManager_h
#define ExerciseManager_h

#include <CoreFoundation/CFBase.h>

enum {
    LiftPullup = 1, LiftBench, LiftDeadlift
};

enum {
    WorkoutStrength, WorkoutSE, WorkoutEndurance, WorkoutHIC
};

enum {
    CircuitRounds, CircuitAMRAP, CircuitDecrement
};

enum {
    ExerciseReps, ExerciseDuration
};

typedef struct {
    CFMutableStringRef titleStr;
    CFMutableStringRef headerStr;
    CFStringRef restStr;
    CFRange tRange;
    CFRange hRange;
    const short reps;
    const short sets;
    short completedSets;
    const unsigned char type;
    unsigned char state;
} ExerciseEntry;

typedef struct {
    ExerciseEntry *exercises;
    CFMutableStringRef headerStr;
    CFRange range;
    int index;
    const int size;
    const short reps;
    short completedReps;
    const unsigned char type;
} Circuit;

typedef struct {
    Circuit *activities;
    Circuit *group;
    CFStringRef title;
    time_t startTime;
    int index;
    const int size;
    int16_t duration;
    const short bodyweight;
    const unsigned char type;
    const unsigned char day;
    const bool testMax;
} Workout;

typedef struct {
    const int index;
    const short sets;
    const short reps;
    const short weight;
    const unsigned char type;
    const unsigned char day;
} WorkoutParams;

Workout *getWorkoutFromLibrary(WorkoutParams *params);

#endif /* ExerciseManager_h */
