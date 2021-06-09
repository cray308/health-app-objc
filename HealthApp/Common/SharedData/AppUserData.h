//
//  AppUserData.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppUserData_h
#define AppUserData_h

#include <stdint.h>
#include <stdbool.h>

typedef struct UserInfo UserInfo;

struct UserInfo {
    double planStart;
    signed char currentPlan;
    uint8_t completedWorkouts;
    unsigned short squatMax, pullUpMax, benchMax, deadliftMax;
};

extern UserInfo *appUserDataShared;


UserInfo *userInfo_initFromStorage(void);
void userInfo_saveData(UserInfo *info);
void appUserData_free(void);

void appUserData_setWorkoutPlan(signed char plan);
bool appUserData_hasWorkoutPlan(void);




#endif /* AppUserData_h */
