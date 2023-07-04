#ifndef HomeVC_h
#define HomeVC_h

#include "ContainerView.h"
#include "UserData.h"

extern Class HomeVCClass;

typedef struct {
    CVPair planContainer;
    id divider;
} HomeVC;

void homeVC_updateWorkoutsList(HomeVC *d, uint8_t completedWorkouts);
void homeVC_createWorkoutsList(id self, UserData const *data);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_planButtonTapped(id self, SEL _cmd, id button);
void homeVC_customButtonTapped(id self, SEL _cmd, id button);

void homeVC_handleFinishedWorkout(id self, uint8_t completedWorkouts);

#endif /* HomeVC_h */
