#ifndef ExerciseManager_h
#define ExerciseManager_h

#include <CoreFoundation/CFArray.h>

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
    WorkoutHIC
};

enum {
    CircuitRounds,
    CircuitAMRAP,
    CircuitDecrement
};

enum {
    ExerciseReps,
    ExerciseDuration,
    ExerciseDistance
};

enum {
    ExerciseStateDisabled,
    ExerciseStateActive,
    ExerciseStateResting,
    ExerciseStateCompleted
};

typedef struct {
    CFMutableStringRef titleStr;
    CFMutableStringRef headerStr;
    CFStringRef restStr;
    CFRange tRange;
    const CFRange hRange;
    const short reps;
    const short sets;
    short completedSets;
    const unsigned char type;
    unsigned char state;
} ExerciseEntry;

typedef struct {
    ExerciseEntry *exercises;
    CFMutableStringRef headerStr;
    const CFRange numberRange;
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

void initExerciseData(int week);

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkout(unsigned char plan, int index);
CFArrayRef exerciseManager_createWorkoutNames(unsigned char type);
Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params);

#endif /* ExerciseManager_h */
