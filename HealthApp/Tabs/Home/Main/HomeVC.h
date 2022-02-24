#ifndef HomeVC_h
#define HomeVC_h

#include <CoreFoundation/CFBundle.h>
#include "ContainerView.h"

extern Class HomeVCClass;

typedef struct {
    ContainerView *planContainer;
    ContainerView *customContainer;
    int numWorkouts;
} HomeVC;

id homeVC_init(CFBundleRef bundle);
void homeVC_updateWorkoutsList(HomeVC *self, unsigned char completed);
void homeVC_createWorkoutsList(id self, unsigned char plan);
void homeVC_updateColors(id self);
void homeVC_navigateToAddWorkout(id self, void *workout);
void homeVC_handleFinishedWorkout(id self, unsigned char completed);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_workoutButtonTapped(id self, SEL _cmd, id btn);
void homeVC_customButtonTapped(id self, SEL _cmd, id btn);

#endif /* HomeVC_h */
