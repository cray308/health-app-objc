#ifndef UserData_h
#define UserData_h

#include <CoreFoundation/CoreFoundation.h>

#define PrefsKey CFSTR("userinfo")

enum {
    MaskWeekStart = 1,
    MaskPlanStart = 2,
    MaskTzOffset = 4,
    MaskDST = 8,
    MaskCurrentPlan = 16,
    MaskCompletedWorkouts = 32
};

typedef struct {
    time_t planStart;
    time_t weekStart;
    int lifts[4];
    uint8_t plan;
    uint8_t completedWorkouts;
} UserData;

void userData_create(UserData *res);
int userData_init(UserData *res, CFDictionaryRef prefs CF_CONSUMED, int *week);

bool userData_update(UserData *m, uint8_t plan, int const *weights);
bool userData_clear(UserData *m);
uint8_t userData_addWorkoutData(UserData *m, uint8_t day, int const *weights, bool *updatedWeights);

extern UserData const *getUserData(void);

#endif /* UserData_h */
