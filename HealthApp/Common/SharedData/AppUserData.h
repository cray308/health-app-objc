//
//  AppUserData.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppUserData_h
#define AppUserData_h

#include <time.h>
#include "AppTypes.h"
#include <stdbool.h>

typedef struct {
    time_t planStart;
    time_t weekStart;
    int tzOffset;
    bool darkMode;
    signed char currentPlan;
    byte completedWorkouts;
    short liftMaxes[4];
} UserInfo;

extern UserInfo *userData;

int appUserData_getWeekInPlan(void);

#endif /* AppUserData_h */
