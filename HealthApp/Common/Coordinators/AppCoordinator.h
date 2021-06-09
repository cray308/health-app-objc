//
//  AppCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppCoordinator_h
#define AppCoordinator_h

#import <UIKit/UIKit.h>

typedef struct UserInfo UserInfo;
typedef struct AppCoordinator AppCoordinator;
typedef struct HomeTabCoordinator HomeTabCoordinator;
typedef struct HistoryTabCoordinator HistoryTabCoordinator;
typedef struct SettingsTabCoordinator SettingsTabCoordinator;

typedef enum {
    LoadedViewController_Home = 0x1,
    LoadedViewController_History = 0x2,
    LoadedViewController_Settings = 0x4
} LoadedViewController;

struct AppCoordinator {
    unsigned char loadedViewControllers;
    int totalTokens;
    int currentDay;
    double weekStart;
    UIWindow *window;
    UITabBarController *tabVC;
    UINavigationController *controllers[3];
    HomeTabCoordinator *homeCoordinator;
    HistoryTabCoordinator *historyCoordinator;
    SettingsTabCoordinator *settingsCoordinator;
};

AppCoordinator *appCoordinator_init(UIWindow *window);
void appCoordinator_free(AppCoordinator *coordinator);
void appCoordinator_start(AppCoordinator *coordinator);

void appCoordinator_setTabToLoaded(AppCoordinator *coordinator, LoadedViewController controller);
void appCoordinator_handleForegroundUpdate(AppCoordinator *coordinator);
void appCoordinator_updateNavBarTokens(AppCoordinator *coordinator, int value);

void appCoordinator_updatedUserInfo(AppCoordinator *coordinator);
void appCoordinator_deletedAppData(AppCoordinator *coordinator);

#endif /* AppCoordinator_h */
