#ifndef AppDelegate_h
#define AppDelegate_h

#include <objc/objc.h>
#include "ExerciseManager.h"
#include "UserData.h"

#define AppDelEncoding "@@[3@]{?=qq[4s]CCC}"

typedef struct {
    Class isa;
    id window;
    id context;
    id children[3];
    UserInfo userData;
} AppDelegate;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id app, id opt);
u_long appDelegate_supportedOrientations(id self, SEL _cmd, id app, id win);
void appDelegate_receivedNotif(AppDelegate *self, SEL _cmd,
                               id unc, id notif, void (^callback)(u_long));

void updateUserInfo(unsigned char plan, unsigned char darkMode, short *weights);
void deleteAppData(void);
void addWorkoutData(id caller, Workout *workout, unsigned char day, short *weights, bool pop);

#endif /* AppDelegate_h */
