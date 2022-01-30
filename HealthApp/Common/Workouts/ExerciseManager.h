#ifndef ExerciseManager_h
#define ExerciseManager_h

#include <CoreFoundation/CFArray.h>

#define getBodyWeight() (userData->bodyweight < 0 ? 145 : userData->bodyweight)

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
    unsigned reps;
    unsigned sets;
    unsigned completedSets;
    enum {
        ExerciseStateDisabled,
        ExerciseStateActive,
        ExerciseStateResting,
        ExerciseStateCompleted
    } state;
    CFRange hRange;
    CFRange tRange;
    CFStringRef restStr;
    CFMutableStringRef titleStr;
    CFMutableStringRef headerStr;
} ExerciseEntry;

typedef struct {
    enum {
        CircuitRounds,
        CircuitAMRAP,
        CircuitDecrement
    } type;
    unsigned reps;
    unsigned completedReps;
    unsigned index;
    unsigned size;
    ExerciseEntry *exercises;
    CFRange numberRange;
    CFMutableStringRef headerStr;
} Circuit;

typedef struct {
    unsigned char type;
    signed char day;
    bool testMax;
    unsigned index;
    unsigned size;
    time_t startTime;
    int16_t duration;
    CFStringRef title;
    Circuit *group;
    ExerciseEntry *entry;
    Circuit *activities;
} Workout;

typedef struct {
    const signed char day;
    unsigned char type;
    int index;
    unsigned sets;
    unsigned reps;
    int weight;
} WorkoutParams;

void initExerciseStrings(void);

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkout(unsigned char plan, int week, int index);
CFArrayRef exerciseManager_createWorkoutNames(unsigned char type);
Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params);

#endif /* ExerciseManager_h */
