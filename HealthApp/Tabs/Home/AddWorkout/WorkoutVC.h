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
    int section;
    int row;
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
    CVPair *containers;
    struct {
        int groupTag;
        struct {
            int group;
            int tag;
        } exerciseInfo;
    } savedInfo;
    int weights[4];
    bool done;
} WorkoutVC;

void initWorkoutData(int week);

id workoutVC_init(Workout *workout);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_startEndWorkout(id self, SEL _cmd, id button);
void workoutVC_willDisappear(id self, SEL _cmd, bool animated);
void workoutVC_handleTap(id self, SEL _cmd, id button);

#endif /* WorkoutVC_h */
