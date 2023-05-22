#ifndef HomeVC_h
#define HomeVC_h

#include "AppDelegate.h"
#include "ContainerView.h"

extern Class HomeVCClass;

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    CVPair planContainer;
    ContainerView *customContainer;
    int numWorkouts;
} HomeVC;

id homeVC_init(VCacheRef tbl, CCacheRef clr, time_t startDate);
void homeVC_updateWorkoutsList(HomeVC *self, unsigned char completed);
void homeVC_createWorkoutsList(id self, const UserInfo *info);
void homeVC_updateColors(id self);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_workoutButtonTapped(id self, SEL _cmd, id btn);
void homeVC_customButtonTapped(id self, SEL _cmd, id btn);

void homeVC_handleFinishedWorkout(id self, unsigned char completed);

#endif /* HomeVC_h */
