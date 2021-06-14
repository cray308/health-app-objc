//
//  AppDelegate.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppDelegate.h"
#import "AppCoordinator.h"
#import "AppUserData.h"
#import "PersistenceService.h"
#import "ViewControllerHelpers.h"
#include "CalendarDateHelpers.h"

#if DEBUG
#import "WeeklyData+CoreDataClass.h"
#endif

void setupData(void);

@interface AppDelegate () {
    AppCoordinator *_coordinator;
    UIWindow *window;
}

@end

@implementation AppDelegate

-(void) dealloc {
    appUserData_free();
    persistenceService_free();
    viewControllerHelper_cleanupValidNumericChars();
    appCoordinator_free(_coordinator);
    [window setRootViewController:nil];
    [window release];
    [super dealloc];
}

- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];

    _coordinator = appCoordinator_init(window);
    if (!_coordinator) return false;

    persistenceService_setup();
    viewControllerHelper_setupValidNumericChars();
    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"hasLaunched"]) {
        setupData();
    }
    appCoordinator_start(_coordinator);
    return true;
}

- (UIInterfaceOrientationMask) application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window {
    return ([[UIDevice currentDevice] userInterfaceIdiom] != UIUserInterfaceIdiomPad) ?
    UIInterfaceOrientationMaskAllButUpsideDown : UIInterfaceOrientationMaskPortrait;
}

- (void) applicationWillEnterForeground: (UIApplication *)application {
    appCoordinator_handleForegroundUpdate(_coordinator);
}

- (AppCoordinator *) getAppCoordinator {
    return _coordinator;
}

@end

void setupData(void) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    #if DEBUG
    int bench = 185, pullup = 20, squat = 300, deadlift = 235, i = 0;
    unsigned char plan = 0;
    double start = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 126489600, calendar, DateSearchDirection_Previous, true);
    double end = date_lastMonth(calendar);

    while ( (int) start < (int) end) {
        WeeklyData *data = [[WeeklyData alloc] initWithContext:persistenceService_sharedContainer.viewContext];
        data.weekStart = start;
        data.weekEnd = start + (WeekSeconds - 1);

        for (int j = 0; j < 6; ++j) {
            switch (j) {
                case 0:
                    if (plan == 0) { data.timeSE += ((rand() % 20) + 10); } else { data.timeStrength += ((rand() % 20) + 20); }
                    data.totalWorkouts += 1;
                    break;
                case 1:
                    if (plan == 0) { data.timeEndurance += ((rand() % 30) + 30); } else { data.timeHIC += ((rand() % 20) + 15); }
                    data.totalWorkouts += 1;
                    break;
                case 2:
                    if (plan == 0) { data.timeEndurance += ((rand() % 30) + 30); } else { data.timeStrength += ((rand() % 20) + 20); }
                    data.totalWorkouts += 1;
                    break;
                case 3:
                    if (plan == 0) { data.timeSE += ((rand() % 20) + 10); } else { data.timeHIC += ((rand() % 20) + 15); }
                    data.totalWorkouts += 1;
                    break;
                case 4:
                    if (plan == 0) { if (rand() % 10 >= 5) { data.timeSE += (rand() % 20); data.totalWorkouts += 1; } } else { data.timeStrength += ((rand() % 20) + 20); data.totalWorkouts += 1; }
                    break;
                case 5:
                    if (plan == 0) { data.timeEndurance += ((rand() % 30) + 30); } else { data.timeEndurance += ((rand() % 30) + 60); }
                    data.totalWorkouts += 1;
                    break;
                default:
                    break;
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
            i = 0; plan = 0;
        }
        start += WeekSeconds;
    }
    #endif

    [NSUserDefaults.standardUserDefaults setBool:true forKey:@"hasLaunched"];
    UserInfo info = {
        .currentPlan = -1,
        .weekStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, 1),
    };
    userInfo_saveData(&info);
    CFRelease(calendar);
}
