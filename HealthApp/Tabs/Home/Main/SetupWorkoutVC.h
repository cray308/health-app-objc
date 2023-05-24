#ifndef SetupWorkoutVC_h
#define SetupWorkoutVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include "ColorCache.h"
#include "ExerciseManager.h"
#include "ViewCache.h"

extern void homeVC_navigateToWorkout(id self, Workout *workout);

extern Class SetupWorkoutVCClass;

typedef struct {
    id parent;
    CFArrayRef names;
    CFDictionaryRef normalDict;
    CFDictionaryRef selectedDict;
    id workoutTextField;
    int index;
    uint8_t type;
} SetupWorkoutVC;

id setupWorkoutVC_init(id parent, uint8_t type, VCacheRef tbl, CCacheRef clr);
void setupWorkoutVC_deinit(id self, SEL _cmd);
void setupWorkoutVC_viewDidLoad(id self, SEL _cmd);
void setupWorkoutVC_tappedButton(id self, SEL _cmd, id btn);
long setupWorkoutVC_numberOfComponents(id self, SEL _cmd, id p);
long setupWorkoutVC_numberOfComponentsLegacy(id self, SEL _cmd, id picker);
long setupWorkoutVC_numberOfRows(id self, SEL _cmd, id p, long s);
CFStringRef setupWorkoutVC_getTitle(id self, SEL _cmd, id p, long row, long s);
CFAttributedStringRef setupWorkoutVC_getAttrTitle(id self, SEL _cmd, id p, long row, long s);
void setupWorkoutVC_didSelectRow(id self, SEL _cmd, id p, long row, long s);
void setupWorkoutVC_didSelectRowLegacy(id self, SEL _cmd, id picker, long row, long s);

#endif /* SetupWorkoutVC_h */
