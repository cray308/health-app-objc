#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include "ColorCache.h"
#include "ViewCache.h"

extern void workoutVC_finishedBottomSheet(void *self, int index, int weight);

extern Class StepperViewClass;
extern Class UpdateMaxesVCClass;

typedef struct {
    SEL stxt;
    void (*setText)(id,SEL,CFStringRef);
    id label;
    id stepper;
    CFMutableStringRef repsStr;
    CFRange range;
} StepperView;

typedef struct {
    void *parent;
    StepperView *stack;
    int index;
    int bodyweight;
} UpdateMaxesVC;

void stepperView_deinit(id self, SEL _cmd);
void stepperView_updatedStepper(id self, SEL _cmd);
void stepperView_increment(id self, SEL _cmd);
void stepperView_decrement(id self, SEL _cmd);

id updateMaxesVC_init(void *parent, int index, int bodyweight, VCacheRef tbl, CCacheRef clr);
void updateMaxesVC_deinit(id self, SEL _cmd);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd);

#endif /* UpdateMaxesVC_h */
