#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <objc/runtime.h>
#include <CoreFoundation/CFBase.h>

extern Class UpdateMaxesVCClass;

typedef struct {
    id parent;
    id stepper;
    id stepperLabel;
    CFStringRef stepperFormat;
    int index;
    short bodyweight;
} UpdateMaxesVC;

id updateMaxesVC_init(id parent, int index, short bodyweight);
void updateMaxesVC_deinit(id self, SEL _cmd);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_updatedStepper(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd);

#endif /* UpdateMaxesVC_h */
