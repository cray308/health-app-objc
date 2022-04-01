#ifndef WorkoutVC_h
#define WorkoutVC_h

#include <pthread.h>
#include "ColorCache.h"
#include "ContainerView.h"
#include "ExerciseManager.h"
#include "ViewCache.h"

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

#endif /* WorkoutVC_h */
