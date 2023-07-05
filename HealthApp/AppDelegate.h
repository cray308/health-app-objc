#ifndef AppDelegate_h
#define AppDelegate_h

#include <objc/objc.h>
#include "ExerciseManager.h"
#include "UserData.h"

typedef struct {
    Class isa;
    id window;
    id tabs[3];
    UserData userData;
} AppDelegate;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id app, CFDictionaryRef options);
u_long appDelegate_supportedInterfaceOrientations(id self, SEL _cmd, id app, id window);
void appDelegate_willPresentNotification(AppDelegate *self, SEL _cmd,
                                         id center, id notification, void (^callback)(u_long));

void updateUserInfo(uint8_t plan, int const *weights);
void deleteAppData(void);
void addWorkout(Workout const *workout, uint8_t day, int *weights, bool pop);

#endif /* AppDelegate_h */
