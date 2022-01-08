#ifndef SetupWorkoutVC_h
#define SetupWorkoutVC_h

#include <objc/runtime.h>
#include "ExerciseManager.h"

extern Class SetupWorkoutVCClass;
extern Ivar SetupWorkoutVCDataRef;

typedef struct {
    id parent;
    CFArrayRef names;
    id workoutTextField;
    unsigned char type;
    int index;
} SetupWorkoutVCData;

id setupWorkoutVC_init(id parent, unsigned char type);
void setupWorkoutVC_deinit(id self, SEL _cmd);
void setupWorkoutVC_viewDidLoad(id self, SEL _cmd);
void setupWorkoutVC_tappedButton(id self, SEL _cmd, id btn);

long setupWorkoutVC_numberOfComponents(id self, SEL _cmd, id picker);
long setupWorkoutVC_numberOfRows(id self, SEL _cmd, id picker, long section);
id setupWorkoutVC_attrTitleForRow(id self, SEL _cmd, id picker, long row, long section);
CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd, id picker, long row, long section);
void setupWorkoutVC_didSelectRow(id self, SEL _cmd, id picker, long row, long section);

#endif /* SetupWorkoutVC_h */
