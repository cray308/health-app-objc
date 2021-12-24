#ifndef WorkoutVC_h
#define WorkoutVC_h

#include <objc/runtime.h>
#include <pthread.h>
#include "ContainerView.h"
#include "ExerciseManager.h"

extern Class WorkoutVCClass;
extern Ivar WorkoutVCDataRef;

typedef struct {
    id parent;
    struct __timerInfo {
        const unsigned char type;
        unsigned char active;
        unsigned char stop;
    } info;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    unsigned container;
    unsigned exercise;
    int duration;
    time_t refTime;
} WorkoutTimer;

typedef struct {
    Workout *workout;
    ContainerViewData *first;
    id containers[10];
    id observers[2];
    pthread_t threads[2];
    struct __savedWorkoutInfo {
        unsigned groupTag;
        struct __exerciseInfo {
            unsigned group;
            unsigned tag;
        } exerciseInfo;
    } savedInfo;
    WorkoutTimer timers[2];
    bool done;
} WorkoutVCData;

void initWorkoutStrings(void);
id workoutVC_init(Workout *workout);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_willDisappear(id self, SEL _cmd, bool animated);
void workoutVC_startEndWorkout(id self, SEL _cmd, id btn);
void workoutVC_handleTap(id self, SEL _cmd, id btn);

void workoutVC_handleFinishedWorkout(id self, short *lifts);

#endif /* WorkoutVC_h */