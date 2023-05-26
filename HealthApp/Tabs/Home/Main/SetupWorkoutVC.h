#ifndef SetupWorkoutVC_h
#define SetupWorkoutVC_h

#include <objc/objc.h>
#include "ExerciseManager.h"

extern void homeVC_navigateToWorkout(id self, Workout *workout);

extern Class SetupWorkoutVCClass;

typedef struct {
    id parent;
    CFArrayRef names;
    CFDictionaryRef normalDict;
    CFDictionaryRef selectedDict;
    id workoutField;
    int index;
    uint8_t type;
} SetupWorkoutVC;

void initSetupWorkoutData(void);

id setupWorkoutVC_init(id parent, uint8_t type);
void setupWorkoutVC_deinit(id self, SEL _cmd);
void setupWorkoutVC_viewDidLoad(id self, SEL _cmd);
void setupWorkoutVC_tappedButton(id self, SEL _cmd, id button);
long setupWorkoutVC_numberOfComponents(id self, SEL _cmd, id p);
long setupWorkoutVC_numberOfRows(id self, SEL _cmd, id p, long s);
CFStringRef setupWorkoutVC_getTitle(id self, SEL _cmd, id p, long row, long s);
CFAttributedStringRef setupWorkoutVC_getAttrTitle(id self, SEL _cmd, id p, long row, long s);
void setupWorkoutVC_didSelectRow(id self, SEL _cmd, id p, long row, long s);

#endif /* SetupWorkoutVC_h */
