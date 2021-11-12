//
//  WorkoutScreenHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef WorkoutScreenHelpers_h
#define WorkoutScreenHelpers_h

#include "Workout.h"

typedef void (^ObjectBlock)(id);

extern pthread_mutex_t timerLock;

void setupTimers(Workout *w, id parent);
void setupDeviceEventNotifications(id *observers, ObjectBlock active, ObjectBlock resign);
void cleanupWorkoutNotifications(id *observers, bool *removed);

#endif /* WorkoutScreenHelpers_h */
