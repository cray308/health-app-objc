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
    float weightArray[4];
    short totalWorkouts;
    short durationByType[4];
    short cumulativeDuration[4];
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

extern int massType;
extern float fromSavedMass;
extern float toSavedMass;

UserInfo const *getUserInfo(void);

void presentVC(id child);
void presentModalVC(id modal);
void dismissPresentedVC(Callback handler);

id createAlertController(CFStringRef titleKey, CFStringRef msgKey);
void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler);

#endif /* AppDelegate_h */
