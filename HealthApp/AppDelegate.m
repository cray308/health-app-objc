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
#import "ChartHelpers.h"
#import "TabCoordinator.h"


#import "Exercise.h"
#import "CalendarDateHelpers.h"
#import "WeeklyData+CoreDataClass.h"

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
    sharedHistoryXAxisFormatter_free();
    appCoordinator_free(_coordinator);
    [window setRootViewController:nil];
    [window release];
    [super dealloc];
}

- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];

    _coordinator = appCoordinator_init(window);
    if (!_coordinator) return false;

    sharedHistoryXAxisFormatter_setup();
    persistenceService_setup();
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
    if (_coordinator->currentlyActiveCoordinator != AppCoordinatorChild_Tabs) return;
    tabCoordinator_handleForegroundUpdate((TabCoordinator *) _coordinator->childCoordinator);
}

@end

void setupData(void) {
    int bench = 115, pullup = 20, squat = 300, i = 0;
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double start = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 126489600, calendar, DateSearchDirection_Previous, true);
    double end = date_lastMonth(calendar);
    CFRelease(calendar);

    while ( (int) start < (int) end) {
        WeeklyData *data = [[WeeklyData alloc] initWithContext:persistenceService_sharedContainer.viewContext];
        data.weekStart = start;
        data.weekEnd = start + (WeekSeconds - 1);
        data.bestBench = bench;
        data.bestPullup = pullup;
        data.bestSquat = squat;
        [data release];

        if (i % 10 == 0) {
            bench += 5; pullup += 5; squat += 5;
        }
        ++i;
        start += WeekSeconds;
    }

    //[NSUserDefaults.standardUserDefaults setBool:true forKey:@"hasLaunched"];

    /*

     signed char currentPlan;
     unsigned char workoutGoal;
     unsigned char completedWorkouts[6];
     unsigned short squatMax, pullUpMax, benchMax;

     UserInfo info = {.currentPlan = -1, .completedWorkouts = 0};
     userInfo_saveData(&info);
     */
}
