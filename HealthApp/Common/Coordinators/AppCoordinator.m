//
//  AppCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppCoordinator.h"
#import "AppUserData.h"
#import "HomeTabCoordinator.h"
#import "HistoryTabCoordinator.h"
#import "SettingsTabCoordinator.h"
#include "CalendarDateHelpers.h"
#import "PersistenceService.h"

typedef enum {
    TabHome, TabHistory, TabSettings
} AppTab;

void appCoordinator_free(AppCoordinator *coordinator) {
    homeCoordinator_free(coordinator->children[0]);
    historyCoordinator_free(coordinator->children[1]);
    settingsCoordinator_free(coordinator->children[2]);
    [coordinator->tabVC release];
}

void appCoordinator_start(AppCoordinator *coordinator, time_t now, time_t weekStart) {
    coordinator->tabVC = [[UITabBarController alloc] init];

    appUserDataShared = userInfo_initFromStorage();
    if (!appUserDataShared) return;

    int tzOffset = appUserData_checkTimezone(now);
    if (tzOffset) {
        persistenceService_changeTimestamps(tzOffset);
    }

    if (weekStart != appUserDataShared->weekStart) {
        appUserData_handleNewWeek(weekStart);
    }

    persistenceService_performForegroundUpdate();
    UINavigationController *controllers[3];
    UITabBarItem *items[3] = {
        [[UITabBarItem alloc] initWithTitle:@"Home" image:[UIImage systemImageNamed:@"house"] tag:0],
        [[UITabBarItem alloc] initWithTitle:@"History" image:[UIImage systemImageNamed:@"chart.bar"] tag:1],
        [[UITabBarItem alloc] initWithTitle:@"Settings" image:[UIImage systemImageNamed:@"gear"] tag:2]};
    for (int i = 0; i < 3; ++i) {
        controllers[i] = [[UINavigationController alloc] initWithNibName:nil bundle:nil];
        [controllers[i].navigationBar setBarTintColor:UIColor.tertiarySystemGroupedBackgroundColor];
        controllers[i].tabBarItem = items[i];
    }

    HomeTabCoordinator *homeCoord = calloc(1, sizeof(HomeTabCoordinator));
    homeCoord->navigationController = controllers[0];
    homeCoordinator_start(homeCoord);

    HistoryTabCoordinator *histCoord = calloc(1, sizeof(HistoryTabCoordinator));
    histCoord->navigationController = controllers[1];
    historyCoordinator_start(histCoord);

    SettingsTabCoordinator *settingsCoord = malloc(sizeof(SettingsTabCoordinator));
    settingsCoord->navigationController = controllers[2];
    settingsCoordinator_start(settingsCoord);

    coordinator->children[0] = homeCoord;
    coordinator->children[1] = histCoord;
    coordinator->children[2] = settingsCoord;

    [coordinator->tabVC setViewControllers:@[controllers[0], controllers[1], controllers[2]] animated:false];

    for (int i = 0; i < 3; ++i) {
        [controllers[i] release];
        [items[i] release];
    }
}

void appCoordinator_updatedUserInfo(AppCoordinator *coordinator) {
    homeCoordinator_resetUI(coordinator->children[TabHome]);
}

void appCoordinator_deletedAppData(AppCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator->children[TabHome]);
    if (coordinator->loadedViewControllers & LoadedViewController_History) {
        historyCoordinator_updateUI(coordinator->children[TabHistory]);
    }
}

void appCoordinator_updateMaxWeights(AppCoordinator *coordinator) {
    if (coordinator->loadedViewControllers & LoadedViewController_Settings) {
        settingsCoordinator_updateWeightText(coordinator->children[TabSettings]);
    }
}
