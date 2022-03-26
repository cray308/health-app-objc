#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <objc/objc.h>
#include <CoreFoundation/CFBase.h>

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
    short bodyweight;
} UpdateMaxesVC;

#endif /* UpdateMaxesVC_h */
