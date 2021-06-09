//
//  HomeTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeTabCoordinator_h
#define HomeTabCoordinator_h

#import <UIKit/UIKit.h>

@class HomeViewController;
typedef struct AppCoordinator AppCoordinator;
typedef struct HomeTabCoordinator HomeTabCoordinator;
typedef struct HomeViewModel HomeViewModel;
typedef struct AddWorkoutCoordinator AddWorkoutCoordinator;

struct HomeTabCoordinator {
    UINavigationController *navigationController;
    AppCoordinator *delegate;
    HomeViewModel *viewModel;
    AddWorkoutCoordinator *childCoordinator;
};

HomeTabCoordinator *homeCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate);
void homeCoordinator_free(HomeTabCoordinator *coordinator);
void homeCoordinator_start(HomeTabCoordinator *coordinator);

void homeCoordinator_updateNavBarTokens(HomeTabCoordinator *coordinator, NSString *label);
void homeCoordinator_performForegroundUpdate(HomeTabCoordinator *coordinator);
void homeCoordinator_updateForNewWeek(HomeTabCoordinator *coordinator);
void homeCoordinator_updateForNewDay(HomeTabCoordinator *coordinator, int mostRecentDay, int currentDay);

void homeCoordinator_handleUserInfoChange(HomeTabCoordinator *coordinator);
void homeCoordinator_handleDataDeletion(HomeTabCoordinator *coordinator);

void homeCoordinator_navigateToAddWorkouts(HomeTabCoordinator *coordinator);
void homeCoordinator_didFinishAddingWorkouts(HomeTabCoordinator *coordinator, int newTokens, int *durations);

#endif /* HomeTabCoordinator_h */
