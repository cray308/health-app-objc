#ifndef AppUserData_h
#define AppUserData_h

#include <time.h>
#include <stdbool.h>

#define WeekSeconds 604800

typedef struct {
    time_t planStart;
    time_t weekStart;
    int tzOffset;
    bool darkMode;
    signed char currentPlan;
    unsigned char completedWorkouts;
    short liftMaxes[4];
} UserInfo;

extern UserInfo *userData;

void userInfo_create(void);
int userInfo_initFromStorage(void);

int appUserData_getWeekInPlan(void);
bool appUserData_updateUserSettings(signed char plan, bool darkMode, short *weights);
void appUserData_deleteSavedData(void);
unsigned char appUserData_addCompletedWorkout(unsigned char day);
void appUserData_updateWeightMaxes(short *weights);

#endif /* AppUserData_h */
