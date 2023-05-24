#ifndef HomeVC_h
#define HomeVC_h

#include "ContainerView.h"
#include "UserData.h"

extern Class HomeVCClass;

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    CVPair planContainer;
    ContainerView *customContainer;
    int numWorkouts;
} HomeVC;

id homeVC_init(VCacheRef tbl, CCacheRef clr);
void homeVC_updateWorkoutsList(HomeVC *self, uint8_t completed);
void homeVC_createWorkoutsList(id self, UserData const *data);
void homeVC_updateColors(id self);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_workoutButtonTapped(id self, SEL _cmd, id btn);
void homeVC_customButtonTapped(id self, SEL _cmd, id btn);

void homeVC_handleFinishedWorkout(id self, uint8_t completed);

#endif /* HomeVC_h */
