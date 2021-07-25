//
//  AppUserData.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppUserData_h
#define AppUserData_h

typedef struct {
    double planStart;
    double weekStart;
    signed char currentPlan;
    unsigned char completedWorkouts;
    short liftMaxes[4];
} UserInfo;

extern UserInfo *appUserDataShared;

UserInfo *userInfo_initFromStorage(void);
void userInfo_saveData(UserInfo *info);

void appUserData_setWorkoutPlan(signed char plan);
void appUserData_deleteSavedData(void);
void appUserData_handleNewWeek(double weekStart);
unsigned char appUserData_addCompletedWorkout(unsigned char day);
int appUserData_getWeekInPlan(void);
void appUserData_updateWeightMaxes(short *weights);

#endif /* AppUserData_h */
