#ifndef AppDelegate_h
#define AppDelegate_h

#include <objc/objc.h>
#include "CocoaHelpers.h"
#include "ExerciseManager.h"
#include "UserData.h"

typedef struct {
    Class isa;
    id window;
    id tabs[3];
    UserData userData;
} AppDelegate;

void presentVC(id child);
void presentModalVC(id modal);
void dismissPresentedVC(Callback handler);

id createAlertController(CFStringRef titleKey, CFStringRef msgKey);
void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler);

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id app, id opt);
int appDelegate_supportedOrientations(AppDelegate *self, SEL _cmd, id app, id win);

void updateUserInfo(uint8_t plan, uint8_t darkMode, int const *weights);
void deleteAppData(void);
void addWorkoutData(Workout const *workout, uint8_t day, int *weights, bool pop);

#endif /* AppDelegate_h */
