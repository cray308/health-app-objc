//
//  NotificationHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 6/16/21.
//

#ifndef NotificationHelpers_h
#define NotificationHelpers_h

#include <CoreFoundation/CoreFoundation.h>

typedef enum {
    WorkoutNotificationExerciseCompleted,
    WorkoutNotificationAMRAPCompleted
} WorkoutNotification;

void notifications_requestAccess(void);
void notifications_cleanup(void);
void notifications_schedule(unsigned int secondsFromNow, unsigned char messageType);

#endif /* NotificationHelpers_h */
