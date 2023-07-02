#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

extern void workoutVC_finishedBottomSheet(id self, int index, int weight);

extern Class StepperViewClass;
extern Class UpdateMaxesVCClass;

typedef struct {
    id label;
    id stepper;
    CFMutableStringRef reps;
    CFRange range;
} StepperView;

typedef struct {
    id delegate;
    id repsStepper;
    int index;
} UpdateMaxesVC;

void initUpdateMaxesData(void);

void stepperView_deinit(id self, SEL _cmd);
void stepperView_changedValue(id self, SEL _cmd);
void stepperView_accessibilityIncrement(id self, SEL _cmd);
void stepperView_accessibilityDecrement(id self, SEL _cmd);

id updateMaxesVC_init(id delegate, int index);
void updateMaxesVC_deinit(id self, SEL _cmd);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd, id button);

#endif /* UpdateMaxesVC_h */
