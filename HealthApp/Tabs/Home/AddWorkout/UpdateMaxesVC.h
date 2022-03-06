#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <objc/objc.h>
#include <CoreFoundation/CFBase.h>

typedef struct {
    id parent;
    id stepper;
    id stepperLabel;
    CFStringRef stepperFormat;
    int index;
    short bodyweight;
} UpdateMaxesVC;

#endif /* UpdateMaxesVC_h */
