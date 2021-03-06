#ifndef ExerciseManager_h
#define ExerciseManager_h

#include <CoreFoundation/CoreFoundation.h>

#define MinWorkoutDuration 15

enum {
    StrengthIndexMain,
    StrengthIndexTestMax = 2
};

enum {
    LiftPullup = 1,
    LiftBench,
    LiftDeadlift
};

enum {
    WorkoutStrength,
    WorkoutSE,
    WorkoutEndurance,
    WorkoutHIC
};

enum {
    CircuitRounds,
    CircuitAMRAP,
    CircuitDecrement
};

enum {
    ExerciseReps,
    ExerciseDuration
};

typedef struct {
    CFMutableStringRef title;
    CFMutableStringRef header;
    CFStringRef rest;
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
    CFMutableStringRef header;
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
    time_t startTime;
    int index;
    const int jIdx;
    const int size;
    int16_t duration;
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

extern short bodyweight;

void initExerciseData(int week, float *multiplierRef);

void setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names);
Workout *getWeeklyWorkout(int index, unsigned char plan, const short *lifts);
CFArrayRef createWorkoutNames(unsigned char type);
Workout *getWorkoutFromLibrary(WorkoutParams *params, const short *lifts);

#endif /* ExerciseManager_h */
