#ifndef SetupWorkoutVC_h
#define SetupWorkoutVC_h

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <objc/objc.h>

extern Class SetupWorkoutVCClass;

typedef struct {
    id parent;
    CFArrayRef names;
    CFDictionaryRef normalDict;
    CFDictionaryRef selectedDict;
    id workoutTextField;
    int index;
    unsigned char type;
} SetupWorkoutVC;

#endif /* SetupWorkoutVC_h */
