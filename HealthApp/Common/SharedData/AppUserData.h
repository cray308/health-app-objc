//
//  AppUserData.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppUserData_h
#define AppUserData_h

#include <time.h>

typedef enum {
    WorkoutPlanNone = -1,
    WorkoutPlanBaseBuilding = 0,
    WorkoutPlanContinuation = 1
} WorkoutPlan;

typedef struct {
    time_t planStart;
    time_t weekStart;
    int tzOffset;
    signed char currentPlan;
    unsigned char completedWorkouts;
    short liftMaxes[4];
} UserInfo;

extern UserInfo *userData;

int appUserData_getWeekInPlan(void);
void appUserData_updateWeightMaxes(short *weights);

#endif /* AppUserData_h */
