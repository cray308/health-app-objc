#ifndef HomeVC_h
#define HomeVC_h

#include "ContainerView.h"
#include "SetupWorkoutVC.h"
#include "UserData.h"

#define HomeVCEncoding "{?={?=@@}@}"

extern Class HomeVCClass;

typedef struct {
    CVPair planContainer;
    id firstDiv;
} HomeVC;

id homeVC_init(time_t startDate);
void homeVC_updateWorkoutsList(HomeVC *d, unsigned char completed);
void homeVC_createWorkoutsList(id self, const UserInfo *info);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_workoutButtonTapped(id self, SEL _cmd, id btn);
void homeVC_customButtonTapped(id self, SEL _cmd, id btn);

void homeVC_handleFinishedWorkout(id self, unsigned char completed);

#endif /* HomeVC_h */
