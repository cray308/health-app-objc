#ifndef SetupWorkoutVC_h
#define SetupWorkoutVC_h

#include <objc/objc.h>
#include "ExerciseManager.h"

extern void homeVC_navigateToWorkout(id self, Workout *workout, CFMutableStringRef *headers);

extern Class SetupWorkoutVCClass;

typedef struct {
    id delegate;
    CFArrayRef names;
    id workoutField;
    int index;
    uint8_t type;
} SetupWorkoutVC;

id setupWorkoutVC_init(id delegate, uint8_t type);
void setupWorkoutVC_deinit(id self, SEL _cmd);
void setupWorkoutVC_viewDidLoad(id self, SEL _cmd);
void setupWorkoutVC_tappedButton(id self, SEL _cmd, id button);
long setupWorkoutVC_numberOfComponentsInPickerView(id self, SEL _cmd, id picker);
long setupWorkoutVC_numberOfRows(id self, SEL _cmd, id picker, long component);
CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd, id picker, long row, long component);
void setupWorkoutVC_didSelectRow(id self, SEL _cmd, id picker, long row, long component);

#endif /* SetupWorkoutVC_h */
