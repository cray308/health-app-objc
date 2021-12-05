#ifndef HomeVC_h
#define HomeVC_h

#include <CoreFoundation/CFBase.h>
#include "HomeTabCoordinator.h"

extern Class HomeVCClass;
extern Ivar HomeVCDataRef;

typedef struct __homeVCData {
    void *delegate;
    id greetingLabel;
    id planContainer;
    CFStringRef timeNames[3];
    CFStringRef stateNames[2];
    int numWorkouts;
} HomeVCData;

id homeVC_init(void *delegate);
void homeVC_updateWorkoutsList(id self);
void homeVC_createWorkoutsList(id self);
void homeVC_updateColors(id self);

void homeVC_viewDidLoad(id self, SEL _cmd);
void homeVC_viewWillAppear(id self, SEL _cmd, bool animated);
void homeVC_workoutButtonTapped(id self, SEL _cmd, id btn);
void homeVC_customButtonTapped(id self, SEL _cmd, id btn);

#endif /* HomeVC_h */
