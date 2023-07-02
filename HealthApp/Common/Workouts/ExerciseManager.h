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
    CFRange titleRange;
    CFRange headerRange;
    short reps;
    short sets;
    short completedSets;
    uint8_t type;
    uint8_t state;
} Exercise;

typedef struct {
    Exercise *exercises;
    CFMutableStringRef header;
    CFRange range;
    int index;
    int size;
    short reps;
    short completedReps;
    uint8_t type;
} Circuit;

typedef struct {
    Circuit *circuits;
    Circuit *group;
    time_t startTime;
    int index;
    int nameIdx;
    int size;
    int duration;
    uint8_t type;
    uint8_t day;
    bool testMax;
} Workout;

typedef struct {
    int index;
    short sets;
    short reps;
    short weight;
    uint8_t type;
    uint8_t day;
} WorkoutParams;

extern int Bodyweight;

#define createWorkoutTitleKey(t, i) formatStr(NULL, CFSTR("wkNames%d%02d"), (t), (i))

void initExerciseData(int week);

void getWeeklyWorkoutNames(CFStringRef *names, uint8_t plan);
Workout *getWeeklyWorkout(int index, uint8_t plan, int const *lifts);
CFArrayRef createWorkoutNames(uint8_t type);
Workout *getWorkoutFromLibrary(WorkoutParams const *params, int const *lifts);

#endif /* ExerciseManager_h */
