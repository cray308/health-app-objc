#ifndef AppDelegate_h
#define AppDelegate_h

#include <objc/objc.h>
#include "ColorCache.h"
#include "ViewCache.h"

typedef struct {
    time_t planStart;
    const time_t weekStart;
    int liftMaxes[4];
    unsigned char darkMode;
    unsigned char currentPlan;
    unsigned char completedWorkouts;
} UserInfo;

typedef struct {
    Class isa;
    id window;
    id children[3];
    ColorCache clr;
    VCache tbl;
    UserInfo userData;
} AppDelegate;

typedef void (^Callback)(void);

UserInfo const *getUserInfo(void);

void presentVC(id child);
void presentModalVC(id modal);
void dismissPresentedVC(Callback handler);

id createAlertController(CFStringRef titleKey, CFStringRef msgKey);
void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler);

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id app, id opt);
int appDelegate_supportedOrientations(AppDelegate *self, SEL _cmd, id app, id win);

void updateUserInfo(unsigned char plan, unsigned char darkMode, int *weights);
void deleteAppData(void);
unsigned char addWorkoutData(unsigned char day, unsigned char type, int duration, int *weights);

#endif /* AppDelegate_h */
