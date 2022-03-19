#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <objc/objc.h>
#include <CoreFoundation/CFBase.h>

typedef struct {
    void *parent;
    id stepper;
    id stepperLabel;
    CFMutableStringRef repsStr;
    CFRange range;
    int index;
    short bodyweight;
} UpdateMaxesVC;

#endif /* UpdateMaxesVC_h */
