#ifndef WorkoutVC_h
#define WorkoutVC_h

#include <objc/objc.h>
#include "ExerciseManager.h"
#include "WorkoutTimer.h"

extern Class WorkoutVCClass;

typedef struct {
    WorkoutTimer timers[2];
    Workout *workout;
    int weights[4];
    bool done;
} WorkoutVC;

void initWorkoutData(int week);

id workoutVC_init(Workout *workout, CFMutableStringRef *headers);
void workoutVC_deinit(id self, SEL _cmd);
void workoutVC_viewDidLoad(id self, SEL _cmd);
void workoutVC_startEndWorkout(id self, SEL _cmd, id button);
void workoutVC_viewWillDisappear(id self, SEL _cmd, bool animated);
id workoutVC_cellForItemAtIndexPath(id self, SEL _cmd, id collectionView, id indexPath);
bool workoutVC_shouldSelectItem(id self, SEL _cmd, id collectionView, id indexPath);
void workoutVC_didSelectItemAtIndexPath(id self, SEL _cmd, id collectionView, id indexPath);

void workoutVC_receivedNotification(id self, int identifier);

#endif /* WorkoutVC_h */
