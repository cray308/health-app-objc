#ifndef AppUserData_h
#define AppUserData_h

#include <time.h>
#include <stdbool.h>

#define WeekSeconds 604800

typedef struct {
    time_t planStart;
    time_t weekStart;
    int tzOffset;
    int week;
    short liftMaxes[4];
    unsigned char darkMode;
    unsigned char currentPlan;
    unsigned char completedWorkouts;
} UserInfo;

extern UserInfo *userData;

void userInfo_create(bool darkMode, time_t *startOfWeek);
int userInfo_initFromStorage(time_t *startOfWeek);

bool appUserData_updateUserSettings(unsigned char plan, unsigned char darkMode, short *weights);
bool appUserData_deleteSavedData(void);
int appUserData_addCompletedWorkout(unsigned char day);
bool appUserData_updateWeightMaxes(short *weights);

#endif /* AppUserData_h */
