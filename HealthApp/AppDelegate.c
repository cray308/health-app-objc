//
//  AppDelegate.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppDelegate.h"
#include "AppCoordinator.h"
#include "PersistenceService.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"
#include "SwiftBridging.h"

static CFStringRef const hasLaunchedKey = CFSTR("hasLaunched");

Class AppDelegateClass;

static inline time_t getStartOfDay(time_t date, struct tm *info) {
    const int seconds = (info->tm_hour * 3600) + (info->tm_min * 60) + info->tm_sec;
    return date - seconds;
}

static time_t calcStartOfWeek(time_t date) {
    struct tm localInfo;
    localtime_r(&date, &localInfo);
    int weekday = localInfo.tm_wday;

    if (weekday == 1) return getStartOfDay(date, &localInfo);

    date -= WeekSeconds;
    while (weekday != 1) {
        date += DaySeconds;
        weekday = weekday == 6 ? 0 : weekday + 1;
    }
    localtime_r(&date, &localInfo);
    return getStartOfDay(date, &localInfo);
}

static void setupData(time_t now, time_t weekStart);

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = createObjectWithFrame("UIWindow", bounds);

    bool hasLaunched = ((bool(*)(id,SEL,CFStringRef))objc_msgSend)
    (getUserDefaults(), sel_getUid("boolForKey:"), hasLaunchedKey);

    persistenceService_init();

    time_t now = time(NULL);
    time_t weekStart = calcStartOfWeek(now);

    if (!hasLaunched) {
        setupData(now, weekStart);
    } else {
        userInfo_initFromStorage();
    }

    int tzOffset = appUserData_checkTimezone(now);
    if (weekStart != appUserDataShared->weekStart)
        appUserData_handleNewWeek(weekStart);

    id appearance = ((id(*)(id,SEL))objc_msgSend)(allocClass("UITabBarAppearance"),
                                                  sel_getUid("init"));
    setBackground(appearance, createColor("systemBackgroundColor"));
    setTabBarItemColors(((id(*)(id,SEL))objc_msgSend)(appearance,
                                                      sel_getUid("stackedLayoutAppearance")));
    setTabBarItemColors(((id(*)(id,SEL))objc_msgSend)(appearance,
                                                      sel_getUid("inlineLayoutAppearance")));
    setTabBarItemColors(((id(*)(id,SEL))objc_msgSend)(appearance,
                                                      sel_getUid("compactInlineLayoutAppearance")));

    id tabVC = ((id(*)(id,SEL))objc_msgSend)(allocClass("UITabBarController"), sel_getUid("init"));
    id bar = ((id(*)(id,SEL))objc_msgSend)(tabVC, sel_getUid("tabBar"));
    ((void(*)(id,SEL,id))objc_msgSend)(bar, sel_getUid("setStandardAppearance:"), appearance);
    appCoordinatorShared = calloc(1, sizeof(AppCoordinator));
    appCoordinator_start(appCoordinatorShared, tabVC);
    ((void(*)(id,SEL,id))objc_msgSend)(self->window, sel_getUid("setRootViewController:"), tabVC);
    objc_singleArg(self->window, sel_getUid("makeKeyAndVisible"));

    if (!hasLaunched) {
        ((void(*)(id,SEL,int,void(^)(BOOL,id)))objc_msgSend)
        (getNotificationCenter(), sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
         6, ^(BOOL granted _U_, id error _U_) {});
    }
    releaseObj(appearance);
    releaseObj(tabVC);

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        persistenceService_start(tzOffset);
        appCoordinator_fetchHistory(appCoordinatorShared);
    });
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_,
                                      id application _U_, id window _U_) {
    id device = objc_staticMethod(objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    int idiom = ((int(*)(id,SEL))objc_msgSend)(device, sel_getUid("userInterfaceIdiom"));
    return idiom == 1 ? 26 : 2;
}

void setupData(time_t now, time_t weekStart) {
#if DEBUG
    int16_t lifts[] = {300, 20, 185, 235};
    int i = 0;
    unsigned char plan = 0;
    time_t start = calcStartOfWeek(time(NULL) - 126489600);
    time_t end = calcStartOfWeek(time(NULL) - 2678400);
    id context = ((id(*)(id,SEL))objc_msgSend)(persistenceServiceShared, sel_getUid("viewContext"));

    while (start < end) {
        int16_t totalWorkouts = 0;
        int16_t times[4] = {0};
        id data = ((id(*)(id,SEL,id))objc_msgSend)(allocClass("WeeklyData"),
                                                   sel_getUid("initWithContext:"), context);
        ((void(*)(id,SEL,int64_t))objc_msgSend)(data, sel_getUid("setWeekStart:"), start);

        if (plan == 0) {
            for (int j = 0; j < 6; ++j) {
                int extra = 10;
                bool didSE = true;
                switch (j) {
                    case 1:
                    case 2:
                    case 5:
                        times[2] += ((rand() % 30) + 30);
                        totalWorkouts += 1;
                        break;
                    case 4:
                        if ((didSE = (rand() % 10 >= 5))) extra = 0;
                    case 0:
                    case 3:
                        if (didSE) {
                            times[1] += ((rand() % 20) + extra);
                            totalWorkouts += 1;
                        }
                    default:
                        break;
                }
            }
        } else {
            for (int j = 0; j < 6; ++j) {
                switch (j) {
                    case 0:
                    case 2:
                    case 4:
                        times[0] += ((rand() % 20) + 20);
                        totalWorkouts += 1;
                        break;
                    case 1:
                    case 3:
                        times[3] += ((rand() % 20) + 15);
                        totalWorkouts += 1;
                        break;
                    case 5:
                        times[2] += ((rand() % 30) + 60);
                        totalWorkouts += 1;
                    default:
                        break;
                }
            }
        }

        if (i == 7) {
            plan = 1;
        } else if (i == 20 || i == 32 || i == 44) {
            lifts[2] = 185 + (rand() % 50);
            lifts[1] = 20 + (rand() % 20);
            lifts[0] = 300 + (rand() % 80);
            lifts[3] = 235 + (rand() % 50);
        }
        weekData_setTotalWorkouts(data, totalWorkouts);
        for (int l = 0; l < 4; ++l) {
            weekData_setWorkoutTimeForType(data, l, times[l]);
            weekData_setLiftingMaxForType(data, l, lifts[l]);
        }
        releaseObj(data);

        if (++i == 52) {
            i = 0;
            plan = 0;
        }
        start += WeekSeconds;
    }
    ((bool(*)(id,SEL,id))objc_msgSend)(context, sel_getUid("save:"), nil);
#endif

    ((void(*)(id,SEL,bool,CFStringRef))objc_msgSend)
    (getUserDefaults(),sel_getUid("setBool:forKey:"), true, hasLaunchedKey);
    userInfo_create(now, weekStart);
}
