#ifndef SetupWorkoutVC_h
#define SetupWorkoutVC_h

#include <objc/objc.h>
#include "ExerciseManager.h"

#define SetupWorkoutVCEncoding "{?=@@@iC}"

extern Class SetupWorkoutVCClass;

typedef struct {
    id parent;
    CFArrayRef names;
    id workoutTextField;
    int index;
    unsigned char type;
} SetupWorkoutVC;

void homeVC_navigateToAddWorkout(id self, Workout *workout);

id setupWorkoutVC_init(id parent, unsigned char type);
void setupWorkoutVC_deinit(id self, SEL _cmd);
void setupWorkoutVC_viewDidLoad(id self, SEL _cmd);
void setupWorkoutVC_tappedButton(id self, SEL _cmd, id btn);
long setupWorkoutVC_numberOfComponents(id self, SEL _cmd, id picker);
long setupWorkoutVC_numberOfRows(id self, SEL _cmd, id picker, long section);
CFStringRef setupWorkoutVC_getTitle(id self, SEL _cmd, id picker, long row, long section);
void setupWorkoutVC_didSelectRow(id self, SEL _cmd, id picker, long row, long section);

#endif /* SetupWorkoutVC_h */
