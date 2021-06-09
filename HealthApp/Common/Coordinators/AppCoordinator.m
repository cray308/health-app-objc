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
    coordinator->weekStart = date_calcStartOfWeek(date, calendar, DateSearchDirection_Previous, true);
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

    UserInfo *info = userInfo_initFromStorage();
    if (!info) return;
    appUserDataShared = info;

    persistenceService_downsample();
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


    Array_workout *dummyWorkouts = workoutFinder_get_weekly_workouts(FitnessPlanContinuation, 0); // remove
    if (!dummyWorkouts) { NSLog(@"workouts returned nil"); return; }
    NSLog(@"size is %d", array_size(dummyWorkouts));
    for (int i = 0; i < array_size(dummyWorkouts); ++i) {
        NSLog(@"\n\n\n Workout for day %u", dummyWorkouts->arr[i].day);
        NSLog(@"Workout type: %u", dummyWorkouts->arr[i].type);
        NSLog(@"Reps %u", dummyWorkouts->arr[i].reps);
        NSLog(@"Sets: %u", dummyWorkouts->arr[i].sets);
        NSLog(@"Title: %@", dummyWorkouts->arr[i].title);

        Array_exGroup *activities = dummyWorkouts->arr[i].activities;
        for (int j = 0; j < array_size(activities); ++j) {
            NSLog(@"\n\n Type for activity %d: %u", j, activities->arr[j].type);
            NSLog(@"Reps for activity: %u", activities->arr[j].reps);

            Array_exEntry *exercises = activities->arr[j].exercises;
            for (int k = 0; k < array_size(exercises); ++k) {
                 NSLog(@"\n Type for exercise %d: %u", k, exercises->arr[k].type);
                 NSLog(@"Reps %u", exercises->arr[k].reps);
                 NSLog(@"Sets: %u", exercises->arr[k].sets);
                 NSLog(@"Rest: %u", exercises->arr[k].rest);
                 NSLog(@"Name: %@", exercises->arr[k].name);
            }
        }
    }
    array_free(workout, dummyWorkouts);
}

void appCoordinator_setTabToLoaded(AppCoordinator *coordinator, LoadedViewController controller) {
    coordinator->loadedViewControllers |= controller;
    appCoordinator_updateNavBarTokens(coordinator, coordinator->totalTokens);
}

void appCoordinator_updateNavBarTokens(AppCoordinator *coordinator, int value) {
    coordinator->totalTokens = value;
    unsigned char loadedVCs = coordinator->loadedViewControllers;
    NSString *label = [[NSString alloc] initWithFormat:@"%d", value];
    if (loadedVCs & LoadedViewController_Home) {
        homeCoordinator_updateNavBarTokens(coordinator->homeCoordinator, label);
    }
    if (loadedVCs & LoadedViewController_History) {
        historyCoordinator_updateNavBarTokens(coordinator->historyCoordinator, label);
    }
    if (loadedVCs & LoadedViewController_Settings) {
        settingsCoordinator_updateNavBarTokens(coordinator->settingsCoordinator, label);
    }
    [label release];
}

void appCoordinator_handleForegroundUpdate(AppCoordinator *coordinator) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double now = CFAbsoluteTimeGetCurrent();
    int currentDay;
    double weekStart = date_calcStartOfWeek(now, calendar, DateSearchDirection_Previous, true);
    if ((int) weekStart != (int) coordinator->weekStart) { // new week
        coordinator->weekStart = weekStart;
        coordinator->currentDay = date_getDayOfWeek(now, calendar);
        //persistenceService_performForegroundUpdate();
        persistenceService_downsample();

        if (coordinator->loadedViewControllers & LoadedViewController_Home) {
            homeCoordinator_updateForNewWeek(coordinator->homeCoordinator);
        }

        if (coordinator->tabVC.selectedIndex == TabHistory) {
            historyCoordinator_performForegroundUpdate(coordinator->historyCoordinator);
        }

    } else if ((currentDay = (date_getDayOfWeek(now, calendar))) != coordinator->currentDay) { // new day
        int mostRecentDay = coordinator->currentDay;
        coordinator->currentDay = currentDay;

        if (coordinator->loadedViewControllers & LoadedViewController_Home) {
            homeCoordinator_updateForNewDay(coordinator->homeCoordinator, mostRecentDay, currentDay);
        }

    } else if (coordinator->loadedViewControllers & LoadedViewController_Home) {
        homeCoordinator_performForegroundUpdate(coordinator->homeCoordinator);
    }
    CFRelease(calendar);
}

void appCoordinator_updatedUserInfo(AppCoordinator *coordinator) {
    NSLog(@"%u", coordinator->loadedViewControllers);
    //appUserData_setUserInfo(name, goal);
//    homeCoordinator_handleUserInfoChange(coordinator->homeCoordinator);
//    if (coordinator->loadedViewControllers & LoadedViewController_History) {
//        historyCoordinator_handleUserInfoChange(coordinator->historyCoordinator);
//    }
}

void appCoordinator_deletedAppData(AppCoordinator *coordinator) {
    appCoordinator_updateNavBarTokens(coordinator, 0);
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

