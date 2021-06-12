//
//  AppCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppCoordinator.h"
#import "AppUserData.h"
#import "HomeTabCoordinator.h"
#import "HomeViewController.h"
#import "HistoryTabCoordinator.h"
#import "HistoryViewController.h"
#import "SettingsTabCoordinator.h"
#import "SettingsViewController.h"
#import "CalendarDateHelpers.h"
#import "PersistenceService.h"

#import "WorkoutFinder.h" // remove

typedef enum {
    TabHome, TabHistory, TabSettings
} AppTab;

HomeTabCoordinator *startHome(AppCoordinator *coordinator, UINavigationController *navVC);
HistoryTabCoordinator *startHistory(AppCoordinator *coordinator, UINavigationController *navVC);
SettingsTabCoordinator *startSettings(AppCoordinator *coordinator, UINavigationController *navVC);

AppCoordinator *appCoordinator_init(UIWindow *window) {
    AppCoordinator *coordinator = calloc(1, sizeof(AppCoordinator));
    if (!coordinator) return NULL;
    coordinator->window = window;
    coordinator->tabVC = [[UITabBarController alloc] init];
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double date = CFAbsoluteTimeGetCurrent();
    coordinator->currentDay = date_getDayOfWeek(date, calendar);
    CFRelease(calendar);
    return coordinator;
}

void appCoordinator_free(AppCoordinator *coordinator) {
    for (int i = 0; i < 3; ++i) {
        [coordinator->controllers[i] release];
        coordinator->controllers[i] = nil;
    }
    homeCoordinator_free(coordinator->homeCoordinator);
    historyCoordinator_free(coordinator->historyCoordinator);
    settingsCoordinator_free(coordinator->settingsCoordinator);
    [coordinator->tabVC setViewControllers:nil];
    [coordinator->tabVC release];
    free(coordinator);
}

void appCoordinator_start(AppCoordinator *coordinator) {
    [coordinator->window setRootViewController:coordinator->tabVC];
    [coordinator->window makeKeyAndVisible];

    appUserDataShared = userInfo_initFromStorage();
    if (!appUserDataShared) return;

    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double weekStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, true);
    if ((int) weekStart != (int) appUserDataShared->weekStart) {
        appUserData_handleNewWeek(weekStart);
    }
    CFRelease(calendar);

    NSLog(@"Remove this line in appCoordinator_start\n\n");
    //appUserData_setWorkoutPlan(0); //this line

    NSLog(@"Maxes: \n bench: %u \n deadlift: %u \n squat: %u \n pullup: %u \n", appUserDataShared->benchMax, appUserDataShared->deadliftMax, appUserDataShared->squatMax, appUserDataShared->pullUpMax);

    persistenceService_performForegroundUpdate();
    coordinator->controllers[0] = [[UINavigationController alloc] initWithNibName:nil bundle:nil];
    if (!(coordinator->homeCoordinator = (startHome(coordinator, coordinator->controllers[0])))) return;

    coordinator->controllers[1] = [[UINavigationController alloc] initWithNibName:nil bundle:nil];
    if (!(coordinator->historyCoordinator = (startHistory(coordinator, coordinator->controllers[1])))) return;

    coordinator->controllers[2] = [[UINavigationController alloc] initWithNibName:nil bundle:nil];
    if (!(coordinator->settingsCoordinator = (startSettings(coordinator, coordinator->controllers[2])))) return;

    [coordinator->tabVC setViewControllers:@[
        coordinator->controllers[0], coordinator->controllers[1], coordinator->controllers[2]
    ] animated:false];
}

void appCoordinator_setTabToLoaded(AppCoordinator *coordinator, LoadedViewController controller) {
    coordinator->loadedViewControllers |= controller;
}

void appCoordinator_handleForegroundUpdate(AppCoordinator *coordinator) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double now = CFAbsoluteTimeGetCurrent();
    int currentDay;
    double weekStart = date_calcStartOfWeek(now, calendar, DateSearchDirection_Previous, true);
    if ((int) weekStart != (int) appUserDataShared->weekStart) { // new week
        coordinator->currentDay = date_getDayOfWeek(now, calendar);
        persistenceService_performForegroundUpdate();
        appUserData_handleNewWeek(weekStart);

        if (coordinator->loadedViewControllers & LoadedViewController_Home) {
            homeCoordinator_updateForNewWeek(coordinator->homeCoordinator);
        }

        if (coordinator->tabVC.selectedIndex == TabHistory) {
            historyCoordinator_performForegroundUpdate(coordinator->historyCoordinator);
        }

    } else if ((currentDay = (date_getDayOfWeek(now, calendar))) != coordinator->currentDay) { // new day
        coordinator->currentDay = currentDay;

        if (coordinator->loadedViewControllers & LoadedViewController_Home) {
            homeCoordinator_updateForNewDay(coordinator->homeCoordinator);
        }

    } else if (coordinator->loadedViewControllers & LoadedViewController_Home) {
        homeCoordinator_performForegroundUpdate(coordinator->homeCoordinator);
    }
    CFRelease(calendar);
}

void appCoordinator_updatedUserInfo(AppCoordinator *coordinator) {
    homeCoordinator_handleUserInfoChange(coordinator->homeCoordinator);
}

void appCoordinator_deletedAppData(AppCoordinator *coordinator) {
    homeCoordinator_handleDataDeletion(coordinator->homeCoordinator);
    if (coordinator->loadedViewControllers & LoadedViewController_History) {
        historyCoordinator_handleDataDeletion(coordinator->historyCoordinator);
    }
}

#pragma mark - Helpers

HomeTabCoordinator *startHome(AppCoordinator *coordinator, UINavigationController *navVC) {
    HomeTabCoordinator *child = homeCoordinator_init(navVC, coordinator);
    if (!child) return NULL;

    UITabBarItem *item = [[UITabBarItem alloc] initWithTitle:@"Home" image:[UIImage systemImageNamed:@"house"] tag:0];
    [navVC.navigationBar setBarTintColor:UIColor.tertiarySystemGroupedBackgroundColor];
    navVC.tabBarItem = item;
    [item release];
    homeCoordinator_start(child);
    return child;
}

HistoryTabCoordinator *startHistory(AppCoordinator *coordinator, UINavigationController *navVC) {
    HistoryTabCoordinator *child = historyCoordinator_init(navVC, coordinator);
    if (!child) return NULL;

    UITabBarItem *item = [[UITabBarItem alloc] initWithTitle:@"History" image:[UIImage systemImageNamed:@"chart.bar"] tag:1];
    [navVC.navigationBar setBarTintColor:UIColor.tertiarySystemGroupedBackgroundColor];
    navVC.tabBarItem = item;
    [item release];
    historyCoordinator_start(child);
    return child;
}

SettingsTabCoordinator *startSettings(AppCoordinator *coordinator, UINavigationController *navVC) {
    SettingsTabCoordinator *child = settingsCoordinator_init(navVC, coordinator);
    if (!child) return NULL;

    UITabBarItem *item = [[UITabBarItem alloc] initWithTitle:@"Settings" image:[UIImage systemImageNamed:@"gear"] tag:2];
    [navVC.navigationBar setBarTintColor:UIColor.tertiarySystemGroupedBackgroundColor];
    navVC.tabBarItem = item;
    [item release];
    settingsCoordinator_start(child);
    return child;
}

