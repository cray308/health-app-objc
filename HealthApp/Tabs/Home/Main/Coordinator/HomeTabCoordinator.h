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
typedef struct Workout Workout;

struct HomeTabCoordinator {
    UINavigationController *navigationController;
    AppCoordinator *delegate;
    HomeViewModel *viewModel;
    AddWorkoutCoordinator *childCoordinator;
};

HomeTabCoordinator *homeCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate);
void homeCoordinator_free(HomeTabCoordinator *coordinator);
void homeCoordinator_start(HomeTabCoordinator *coordinator);

void homeCoordinator_performForegroundUpdate(HomeTabCoordinator *coordinator);
void homeCoordinator_updateForNewWeek(HomeTabCoordinator *coordinator);
void homeCoordinator_updateForNewDay(HomeTabCoordinator *coordinator);

void homeCoordinator_handleUserInfoChange(HomeTabCoordinator *coordinator);
void homeCoordinator_handleDataDeletion(HomeTabCoordinator *coordinator);

void homeCoordinator_showWorkoutPickerVC(HomeTabCoordinator *coordinator, unsigned char type, CFStringRef *names, unsigned int count);
void homeCoordinator_navigateToAddWorkout(HomeTabCoordinator *coordinator, UIViewController *presenter, Workout *workout);
void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *coordinator, int totalCompletedWorkouts);

#endif /* HomeTabCoordinator_h */
