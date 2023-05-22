#ifndef WorkoutVC_h
#define WorkoutVC_h

#include <pthread.h>
#include "ContainerView.h"
#include "ExerciseManager.h"

extern Class WorkoutVCClass;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    void *parent;
    time_t refTime;
    int container;
    int exercise;
    unsigned duration;
    struct {
        const unsigned char type;
        unsigned char active;
        unsigned char stop;
    } info;
} WorkoutTimer;

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    WorkoutTimer timers[2];
    pthread_t threads[2];
    Workout *workout;
    ContainerView *first;
    CVPair *containers;
    struct {
        int groupTag;
        struct {
            int group;
            int tag;
        } exerciseInfo;
    } savedInfo;
    short weights[4];
    bool done;
} WorkoutVC;

void initWorkoutStrings(void);

id workoutVC_init(Workout *workout, VCacheRef tbl, CCacheRef clr);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_startEndWorkout(id self, SEL _cmd, id btn);
void workoutVC_willDisappear(id self, SEL _cmd, bool animated);
void workoutVC_handleTap(id self, SEL _cmd, id btn);

#endif /* WorkoutVC_h */
