#ifndef UserData_h
#define UserData_h

#include <stdbool.h>
#include <time.h>

#define HasLaunchedKey CFSTR("hasLaunched")
#define WeekSeconds 604800
#define DaySeconds 86400
#define MaxValidCharVal 1
#define isCharValueValid(v) (v <= MaxValidCharVal)

enum {
    MaskWeekStart = 1,
    MaskPlanStart = 2,
    MaskTzOffset = 4,
    MaskCurrentPlan = 8,
    MaskCompletedWorkouts = 16
};

typedef struct {
    time_t planStart;
    const time_t weekStart;
    short liftMaxes[4];
    unsigned char currentPlan;
    unsigned char completedWorkouts;
} UserInfo;

extern float toSavedMass;

void userInfo_create(UserInfo *out, time_t *weekStart);
int userInfo_init(UserInfo *out, time_t *weekStart, int *week);

bool userInfo_update(UserInfo *m, unsigned char plan, short *weights);
bool userInfo_deleteData(UserInfo *m);
unsigned char userInfo_addWorkoutData(UserInfo *m,
                                      unsigned char day, short *weights, bool *updatedWeights);

UserInfo const *getUserInfo(void);

#endif /* UserData_h */
