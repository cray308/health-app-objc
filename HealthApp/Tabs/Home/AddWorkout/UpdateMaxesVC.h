#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

extern Class StepperViewClass;
extern Class UpdateMaxesVCClass;

typedef struct {
    id label;
    id stepper;
    CFMutableStringRef repsStr;
    CFRange range;
} StepperView;

typedef struct {
    id parent;
    id stepper;
    int index;
} UpdateMaxesVC;

void workoutVC_finishedBottomSheet(id self, int index, short weight);

void stepperView_deinit(id self, SEL _cmd);
void stepperView_updatedStepper(id self, SEL _cmd);
void stepperView_increment(id self, SEL _cmd);
void stepperView_decrement(id self, SEL _cmd);

id updateMaxesVC_init(id parent, int index);
void updateMaxesVC_deinit(id self, SEL _cmd);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd, id btn);

#endif /* UpdateMaxesVC_h */
