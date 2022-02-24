#ifndef WorkoutVC_h
#define WorkoutVC_h

#include <pthread.h>
#include "ContainerView.h"
#include "ExerciseManager.h"

extern Class WorkoutVCClass;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    id parent;
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
    WorkoutTimer timers[2];
    pthread_t threads[2];
    Workout *workout;
    ContainerView *first;
    id *containers;
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

void initWorkoutStrings(CFBundleRef bundle);
id workoutVC_init(Workout *workout);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_willDisappear(id self, SEL _cmd, bool animated);
void workoutVC_startEndWorkout(id self, SEL _cmd, id btn);
void workoutVC_handleTap(id self, SEL _cmd, id btn);

void workoutVC_finishedBottomSheet(id self, int index, short weight);

#endif /* WorkoutVC_h */
