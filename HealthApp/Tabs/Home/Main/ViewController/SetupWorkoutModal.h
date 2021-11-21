#ifndef SetupWorkoutModal_h
#define SetupWorkoutModal_h

#include <objc/runtime.h>
#include "ExerciseManager.h"

extern Class SetupWorkoutModalClass;
extern Ivar SetupWorkoutModalDataRef;

typedef struct __setupWorkoutModalData {
    void *delegate;
    CFArrayRef names;
    id workoutTextField;
    WorkoutParams output;
} SetupWorkoutModalData;

id setupWorkoutVC_init(void *delegate, unsigned char type, CFArrayRef names);
void setupWorkoutVC_deinit(id self, SEL _cmd);
void setupWorkoutVC_viewDidLoad(id self, SEL _cmd);
void setupWorkoutVC_tappedButton(id self, SEL _cmd, id btn);

long setupWorkoutVC_numberOfComponents(id self, SEL _cmd, id picker);
long setupWorkoutVC_numberOfRows(id self, SEL _cmd, id picker, long section);
CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd, id picker, long row, long section);
void setupWorkoutVC_didSelectRow(id self, SEL _cmd, id picker, long row, long section);

#endif /* SetupWorkoutModal_h */
