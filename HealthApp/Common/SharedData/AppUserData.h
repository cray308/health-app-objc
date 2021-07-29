//
//  AppUserData.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppUserData_h
#define AppUserData_h

#include <CoreFoundation/CoreFoundation.h>

typedef struct {
    long planStart;
    long weekStart;
    int tzOffset;
    signed char currentPlan;
    unsigned char completedWorkouts;
    short liftMaxes[4];
} UserInfo;

extern UserInfo *appUserDataShared;

UserInfo *userInfo_initFromStorage(void);
void userInfo_saveData(UserInfo *info);

int appUserData_checkTimezone(CFTimeZoneRef tz, long time);
void appUserData_setWorkoutPlan(signed char plan);
void appUserData_deleteSavedData(void);
void appUserData_handleNewWeek(long weekStart);
unsigned char appUserData_addCompletedWorkout(unsigned char day);
int appUserData_getWeekInPlan(void);
void appUserData_updateWeightMaxes(short *weights);

#endif /* AppUserData_h */
