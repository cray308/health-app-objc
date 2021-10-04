//
//  Exercise.h
//  HealthApp
//
//  Created by Christopher Ray on 6/6/21.
//

#ifndef Exercise_h
#define Exercise_h

#include "CocoaHelpers.h"
#include "array.h"
#include <pthread.h>

typedef struct {
    id parent;
    struct info {
        const unsigned char type : 2;
        unsigned char active : 2;
        unsigned char stop : 4;
    } info;
    unsigned container;
    unsigned exercise;
    int duration;
    time_t refTime;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WorkoutTimer;

enum {
    LiftSquat,
    LiftPullup,
    LiftBench,
    LiftDeadlift
};

enum {
    ExerciseReps,
    ExerciseDuration,
    ExerciseDistance
};

enum {
    CircuitRounds,
    CircuitAMRAP,
    CircuitDecrement
};

enum {
    WorkoutStrength,
    WorkoutSE,
    WorkoutEndurance,
    WorkoutHIC,
    WorkoutRest
};

gen_array_headers(str, CFStringRef)

typedef struct {
    unsigned char type;
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
    unsigned char type;
    int reps;
    int completedReps;
    unsigned index;
    Array_exEntry *exercises;
} ExerciseGroup;

gen_array_headers(exGroup, ExerciseGroup)

typedef struct {
    unsigned char type;
    signed char day;
    unsigned index;
    time_t startTime;
    int16_t duration;
    short *newLifts;
    CFStringRef title;
    ExerciseGroup *group;
    ExerciseEntry *entry;
    Array_exGroup *activities;
    WorkoutTimer timers[2];
    pthread_t threads[2];
    struct savedInfo {
        unsigned groupTag;
        struct exerciseInfo {
            unsigned group;
            unsigned tag;
        } exerciseInfo;
    } savedInfo;
} Workout;

typedef struct {
    const schar day;
    uchar type;
    int index;
    int sets;
    int reps;
    int weight;
} WorkoutParams;

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index);
Array_str *exerciseManager_getWorkoutNamesForType(unsigned char type);
Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params);

CFStringRef exerciseGroup_createHeader(ExerciseGroup *g);
CFStringRef exerciseEntry_createSetsTitle(ExerciseEntry *e);
CFStringRef exerciseEntry_createTitle(ExerciseEntry *e);

void workoutParams_init(WorkoutParams *this, schar day);
void workout_setDuration(Workout *w);
void initWorkoutStrings(void);

#endif /* Exercise_h */
