//
//  WorkoutScreenHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef WorkoutScreenHelpers_h
#define WorkoutScreenHelpers_h

#include "CocoaHelpers.h"
#include "Workout.h"

extern pthread_mutex_t timerLock;

void setupTimers(Workout *w, id parent);
void setupDeviceEventNotifications(id *observers, ObjectBlock active, ObjectBlock resign);
void cleanupWorkoutNotifications(id *observers);

#endif /* WorkoutScreenHelpers_h */
