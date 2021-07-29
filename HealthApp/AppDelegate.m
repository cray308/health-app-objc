//
//  AppDelegate.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppDelegate.h"
#import "PersistenceService.h"
#import <UserNotifications/UserNotifications.h>
#include "AppUserData.h"
#include "CalendarDateHelpers.h"

#if DEBUG
#import "WeeklyData+CoreDataClass.h"
#endif

#define _U_ __attribute__((__unused__))

void setupData(CFTimeZoneRef tz, long now, long weekStart);

@implementation AppDelegate

- (void) dealloc {
    if (appUserDataShared) free(appUserDataShared);
    appUserDataShared = NULL;
    if (persistenceServiceShared) [persistenceServiceShared release];
    persistenceServiceShared = NULL;
    appCoordinator_free(&coordinator);
    [window release];
    [super dealloc];
}

- (BOOL) application: (UIApplication *)application didFinishLaunchingWithOptions: (NSDictionary *)launchOptions {
    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];

    bool hasLaunched = [NSUserDefaults.standardUserDefaults boolForKey:@"hasLaunched"];

    persistenceServiceShared = [[NSPersistentContainer alloc] initWithName:@"HealthApp"];
    [persistenceServiceShared
     loadPersistentStoresWithCompletionHandler: ^(NSPersistentStoreDescription *description _U_, NSError *error _U_) {}];

    CFCalendarRef calendar = CFCalendarCopyCurrent();
    CFTimeZoneRef tz = CFCalendarCopyTimeZone(calendar);
    long now = CFAbsoluteTimeGetCurrent();
    long weekStart = date_calcStartOfWeek(now, calendar, DateSearchDirectionPrev, true);

    if (!hasLaunched) setupData(tz, now, weekStart);
    appCoordinator_start(&coordinator, tz, now, weekStart);
    [window setRootViewController:coordinator.tabVC];
    [window makeKeyAndVisible];

    if (!hasLaunched) {
        [UNUserNotificationCenter.currentNotificationCenter
         requestAuthorizationWithOptions:UNAuthorizationOptionAlert | UNAuthorizationOptionSound
         completionHandler:^(BOOL granted _U_, NSError *_Nullable error _U_) {}];
    }
    CFRelease(tz);
    CFRelease(calendar);
    return true;
}

- (UIInterfaceOrientationMask) application: (UIApplication *)application
   supportedInterfaceOrientationsForWindow: (UIWindow *)window {
    return (UIDevice.currentDevice.userInterfaceIdiom != UIUserInterfaceIdiomPad) ?
    UIInterfaceOrientationMaskAllButUpsideDown : UIInterfaceOrientationMaskPortrait;
}

@end

void setupData(CFTimeZoneRef tz, long now, long weekStart) {
#if DEBUG
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    int bench = 185, pullup = 20, squat = 300, deadlift = 235, i = 0;
    unsigned char plan = 0;
    long start = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 126489600, calendar, DateSearchDirectionPrev, true);
    long end = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 2678400, calendar, DateSearchDirectionPrev, true);

    while (start < end) {
        WeeklyData *data = [[WeeklyData alloc] initWithContext:persistenceServiceShared.viewContext];
        data.weekStart = start;

        if (plan == 0) {
            for (int j = 0; j < 6; ++j) {
                int extra = 10;
                bool didSE = true;
                switch (j) {
                    case 1:
                    case 2:
                    case 5:
                        data.timeEndurance += ((rand() % 30) + 30);
                        data.totalWorkouts += 1;
                        break;
                    case 4:
                        if ((didSE = (rand() % 10 >= 5))) extra = 0;
                    case 0:
                    case 3:
                        if (didSE) {
                            data.timeSE += ((rand() % 20) + extra);
                            data.totalWorkouts += 1;
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
                        data.timeStrength += ((rand() % 20) + 20);
                        data.totalWorkouts += 1;
                        break;
                    case 1:
                    case 3:
                        data.timeHIC += ((rand() % 20) + 15);
                        data.totalWorkouts += 1;
                        break;
                    case 5:
                        data.timeEndurance += ((rand() % 30) + 60);
                        data.totalWorkouts += 1;
                    default:
                        break;
                }
            }
        }

        if (i == 7) {
            plan = 1;
        } else if (i == 20 || i == 32 || i == 44) {
            bench = 185 + (rand() % 50);
            pullup = 20 + (rand() % 20);
            squat = 300 + (rand() % 80);
            deadlift = 235 + (rand() % 50);
        }
        data.bestBench = bench;
        data.bestPullup = pullup;
        data.bestSquat = squat;
        data.bestDeadlift = deadlift;
        [data release];

        if (++i == 52) {
            i = 0;
            plan = 0;
        }
        start += WeekSeconds;
    }
    CFRelease(calendar);
#endif

    [NSUserDefaults.standardUserDefaults setBool:true forKey:@"hasLaunched"];
    UserInfo info = {.currentPlan = -1, .weekStart = weekStart, .tzOffset = CFTimeZoneGetSecondsFromGMT(tz, now)};
    userInfo_saveData(&info);
}
