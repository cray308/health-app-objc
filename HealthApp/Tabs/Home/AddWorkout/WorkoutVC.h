#ifndef WorkoutVC_h
#define WorkoutVC_h

#include "ContainerView.h"
#include "ExerciseManager.h"
#include "UpdateMaxesVC.h"
#include "WorkoutTimer.h"

extern Class WorkoutVCClass;

typedef struct {
    WorkoutTimer timers[2];
    Workout *workout;
    ContainerView *first;
    CVPair *containers;
    short weights[4];
    bool done;
} WorkoutVC;

void initWorkoutData(int week, float *multiplier);

id workoutVC_init(Workout *workout);
void workoutVC_deinit(id self, SEL _cmd);

void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_startEndWorkout(id self, SEL _cmd, id btn);
void workoutVC_willDisappear(id self, SEL _cmd, bool animated);
void workoutVC_handleTap(id self, SEL _cmd, id btn);

void workoutVC_receivedNotif(id self, int identifier);

#endif /* WorkoutVC_h */
