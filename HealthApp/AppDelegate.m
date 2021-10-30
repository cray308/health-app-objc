#import <UIKit/UIKit.h>
#import <UserNotifications/UNUserNotificationCenter.h>
#include "CocoaHelpers.h"

extern void initExerciseStrings(void);
extern void initCircuitStrings(void);
extern void initTimerStrings(void);
extern void initExerciseMgrStrings(void);
extern void initValidatorStrings(void);
extern void userInfo_create(void);
extern int userInfo_initFromStorage(void);
extern void persistenceService_init(void);
extern void persistenceService_start(int tzOffset);
extern void appCoordinator_start(id tabVC);
extern void appCoordinator_fetchHistory(void);

#if DEBUG
extern void persistenceService_create(void);
#endif

@interface AppDelegate: UIResponder<UIApplicationDelegate> @end
@interface AppDelegate() {
    UIWindow *window;
}
@end
@implementation AppDelegate
- (BOOL) application: (UIApplication *)application didFinishLaunchingWithOptions: (NSDictionary<UIApplicationLaunchOptionsKey,id> *)launchOptions {
    initValidatorStrings();
    initExerciseStrings();
    initCircuitStrings();
    initTimerStrings();
    initExerciseMgrStrings();
    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    NSString *hasLaunchedKey = @"hasLaunched";
    persistenceService_init();
    int tzOffset = 0;

    if (![NSUserDefaults.standardUserDefaults boolForKey:hasLaunchedKey]) {
        [NSUserDefaults.standardUserDefaults setBool:true forKey:hasLaunchedKey];
        userInfo_create();
#if DEBUG
        persistenceService_create();
#endif
        [UNUserNotificationCenter.currentNotificationCenter requestAuthorizationWithOptions:6 completionHandler:^(BOOL granted _U_, id error _U_) {}];
    } else {
        tzOffset = userInfo_initFromStorage();
    }

    UITabBarController *tabVC = [[UITabBarController alloc] init];
    appCoordinator_start(tabVC);
    window.rootViewController = tabVC;
    [window makeKeyAndVisible];
    [tabVC release];

    persistenceService_start(tzOffset);
    appCoordinator_fetchHistory();
    return true;
}

- (UIInterfaceOrientationMask) application: (UIApplication *)application supportedInterfaceOrientationsForWindow: (UIWindow *)window {
    return UIDevice.currentDevice.userInterfaceIdiom == 1 ? 26 : 2;
}
@end
