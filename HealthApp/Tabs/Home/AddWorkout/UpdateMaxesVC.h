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
    id parent;
    id repsStepper;
    int index;
    int bodyweight;
} UpdateMaxesVC;

void initUpdateMaxesData(void);

void stepperView_deinit(id self, SEL _cmd);
void stepperView_updatedStepper(id self, SEL _cmd);
void stepperView_increment(id self, SEL _cmd);
void stepperView_decrement(id self, SEL _cmd);

id updateMaxesVC_init(id parent, int index, int bodyweight);
void updateMaxesVC_deinit(id self, SEL _cmd);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd, id button);

#endif /* UpdateMaxesVC_h */
