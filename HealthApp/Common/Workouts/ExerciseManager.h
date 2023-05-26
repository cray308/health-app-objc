#ifndef ExerciseManager_h
#define ExerciseManager_h

#include <CoreFoundation/CoreFoundation.h>

#define MinWorkoutDuration 15

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
    CFMutableStringRef title;
    CFMutableStringRef header;
    CFStringRef rest;
    CFRange tRange;
    CFRange hRange;
    const short reps;
    const short sets;
    short completedSets;
    const uint8_t type;
    uint8_t state;
} ExerciseEntry;

typedef struct {
    ExerciseEntry *exercises;
    CFMutableStringRef header;
    CFRange range;
    int index;
    const int size;
    const short reps;
    short completedReps;
    const uint8_t type;
} Circuit;

typedef struct {
    Circuit *activities;
    Circuit *group;
    time_t startTime;
    int nameIdx;
    int index;
    const int size;
    int duration;
    const int bodyweight;
    const uint8_t type;
    const uint8_t day;
    const bool testMax;
} Workout;

typedef struct {
    const int index;
    const short sets;
    const short reps;
    const short weight;
    const uint8_t type;
    const uint8_t day;
} WorkoutParams;

#define createWorkoutTitleKey(t, i) formatStr(NULL, CFSTR("wkNames%d%02d"), (t), (i))

void initExerciseData(int week);

void setWeeklyWorkoutNames(uint8_t plan, CFStringRef *names);
Workout *getWeeklyWorkout(uint8_t plan, int i, int const *lifts);
CFArrayRef createWorkoutNames(uint8_t type);
Workout *getWorkoutFromLibrary(WorkoutParams *params, int const *lifts);

#endif /* ExerciseManager_h */
