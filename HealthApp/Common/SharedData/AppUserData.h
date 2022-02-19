#ifndef AppUserData_h
#define AppUserData_h

#include <time.h>
#include <stdbool.h>

#define WeekSeconds 604800

typedef struct {
    time_t planStart;
    const time_t weekStart;
    short liftMaxes[4];
    unsigned char darkMode;
    unsigned char currentPlan;
    unsigned char completedWorkouts;
} UserInfo;

extern UserInfo *userData;

void userInfo_create(bool legacy, UserInfo const **data);
int userInfo_initFromStorage(bool legacy, int *weekInPlan, UserInfo const **data);

unsigned char appUserData_updateUserSettings(unsigned char plan,
                                             unsigned char darkMode, short *weights);
bool appUserData_deleteSavedData(void);
unsigned char appUserData_addCompletedWorkout(unsigned char day);
bool appUserData_updateWeightMaxes(short *weights, short *output);

#endif /* AppUserData_h */
