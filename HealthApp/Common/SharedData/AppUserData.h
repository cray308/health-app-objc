#ifndef AppUserData_h
#define AppUserData_h

#include <time.h>
#include <stdbool.h>

#define WeekSeconds 604800

typedef struct {
    time_t planStart;
    time_t weekStart;
    int tzOffset;
    signed char darkMode;
    signed char currentPlan;
    unsigned char completedWorkouts;
    short liftMaxes[4];
    short bodyweight;
} UserInfo;

extern UserInfo *userData;

void userInfo_create(bool darkMode);
int userInfo_initFromStorage(void);

int appUserData_getWeekInPlan(void);
bool appUserData_updateUserSettings(signed char plan, signed char darkMode, short *weights);
void appUserData_deleteSavedData(void);
unsigned char appUserData_addCompletedWorkout(unsigned char day);
bool appUserData_updateWeightMaxes(short *weights);

#endif /* AppUserData_h */
