#ifndef WorkoutVC_h
#define WorkoutVC_h

#include "ContainerView.h"
#include "ExerciseManager.h"
#include "WorkoutTimer.h"

extern Class WorkoutVCClass;

typedef struct {
    WorkoutTimer timers[2];
    Workout *workout;
    CVPair *containers;
    int weights[4];
    bool done;
} WorkoutVC;

void initWorkoutData(int week);

id workoutVC_init(Workout *workout);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_startEndWorkout(id self, SEL _cmd, id button);
void workoutVC_viewWillDisappear(id self, SEL _cmd, bool animated);
void workoutVC_handleTap(id self, SEL _cmd, id button);

void workoutVC_receivedNotification(id self, int identifier);

#endif /* WorkoutVC_h */
