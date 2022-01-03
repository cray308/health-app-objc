#ifndef HomeVC_h
#define HomeVC_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>

extern Class HomeVCClass;
extern Ivar HomeVCDataRef;

typedef struct {
    id planContainer;
    CFStringRef stateNames[2];
    int numWorkouts;
} HomeVCData;

id homeVC_init(void);
void homeVC_updateWorkoutsList(id self);
void homeVC_createWorkoutsList(id self);
void homeVC_updateColors(id self);
void homeVC_navigateToAddWorkout(id self, void *workout);
void homeVC_handleFinishedWorkout(id self, int totalCompleted);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_workoutButtonTapped(id self, SEL _cmd, id btn);
void homeVC_customButtonTapped(id self, SEL _cmd, id btn);

#endif /* HomeVC_h */
