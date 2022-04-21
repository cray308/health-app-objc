#ifndef AppDelegate_h
#define AppDelegate_h

#include "ColorCache.h"
#include "ViewCache.h"

typedef struct {
    time_t planStart;
    const time_t weekStart;
    short liftMaxes[4];
    unsigned char darkMode;
    unsigned char currentPlan;
    unsigned char completedWorkouts;
} UserInfo;

typedef struct {
    short totalWorkouts;
    short durationByType[4];
    short cumulativeDuration[4];
    short weightArray[4];
} WeekDataModel;

typedef struct {
    Class isa;
    id window;
    id context;
    id children[3];
    ColorCache clr;
    VCache tbl;
    UserInfo userData;
} AppDelegate;

typedef void (^Callback)(void);
typedef void (*FetchHandler)(void*, CFArrayRef, WeekDataModel*, int, bool);

UserInfo const *getUserInfo(void);

void presentVC(id child);
void presentModalVC(id modal);
void dismissPresentedVC(Callback handler);

id createAlertController(CFStringRef title CF_CONSUMED, CFStringRef message CF_CONSUMED);
void addAlertAction(id ctrl, CFStringRef title CF_CONSUMED, int style, Callback handler);

#endif /* AppDelegate_h */
