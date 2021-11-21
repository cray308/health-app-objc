#ifndef WorkoutVC_h
#define WorkoutVC_h

#include "ContainerView.h"
#include "AddWorkoutCoordinator.h"
#include <pthread.h>

extern Class WorkoutVCClass;
extern Ivar WorkoutVCDataRef;

typedef struct __savedWorkoutInfo {
    unsigned groupTag;
    struct __exerciseInfo {
        unsigned group;
        unsigned tag;
    } exerciseInfo;
} SavedWorkoutInfo;

typedef struct __workoutTimer {
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

typedef struct __workoutVCData {
    void *delegate;
    Workout *workout;
    ContainerViewData *first;
    id containers[10];
    id observers[2];
    pthread_t threads[2];
    SavedWorkoutInfo savedInfo;
    WorkoutTimer timers[2];
} WorkoutVCData;

id workoutVC_init(void *delegate);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_startEndWorkout(id self, SEL _cmd, id btn);
void workoutVC_handleTap(id self, SEL _cmd, id btn);

#endif /* WorkoutVC_h */
