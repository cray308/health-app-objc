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
    WorkoutHIC,
    WorkoutRest
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
    CFRange hRange;
    int type;
    int reps;
    int sets;
    int completedSets;
    int state;
} ExerciseEntry;

typedef struct {
    ExerciseEntry *exercises;
    CFMutableStringRef headerStr;
    CFRange numberRange;
    int type;
    int index;
    int size;
    int reps;
    int completedReps;
} Circuit;

typedef struct {
    Circuit *activities;
    Circuit *group;
    CFStringRef title;
    time_t startTime;
    int index;
    int size;
    int type;
    int16_t duration;
    short bodyweight;
    unsigned char day;
    bool testMax;
} Workout;

typedef struct {
    int type;
    int index;
    int sets;
    int reps;
    int weight;
    const unsigned char day;
} WorkoutParams;

void initExerciseData(int week);

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkout(unsigned char plan, int index);
CFArrayRef exerciseManager_createWorkoutNames(int type);
Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params);

#endif /* ExerciseManager_h */
