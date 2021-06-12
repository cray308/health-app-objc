//
//  AppUserData.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppUserData_h
#define AppUserData_h

#import <Foundation/Foundation.h>
//#include <CoreFoundation/CoreFoundation.h>

typedef struct UserInfo UserInfo;

struct UserInfo {
    double planStart;
    double weekStart;
    signed char currentPlan;
    unsigned char completedWorkouts;
    unsigned short squatMax, pullUpMax, benchMax, deadliftMax;
};

extern UserInfo *appUserDataShared;

UserInfo *userInfo_initFromStorage(void);
void userInfo_saveData(UserInfo *info);
void appUserData_free(void);

void appUserData_setWorkoutPlan(signed char plan);
void appUserData_deleteSavedData(void);
void appUserData_handleNewWeek(double weekStart);
unsigned char appUserData_addCompletedWorkout(unsigned char day);
unsigned int appUserData_getWeekInPlan(void);
unsigned char appUserData_hasWorkoutPlan(void);
void appUserData_updateWeightMaxes(unsigned short *weights);




#endif /* AppUserData_h */
