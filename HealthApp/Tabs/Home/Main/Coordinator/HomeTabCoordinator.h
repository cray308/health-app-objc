//
//  HomeTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeTabCoordinator_h
#define HomeTabCoordinator_h

#import <UIKit/UIKit.h>
#include "HomeViewModel.h"

typedef struct {
    UINavigationController *navigationController;
    HomeViewModel viewModel;
    void *childCoordinator;
} HomeTabCoordinator;

void homeCoordinator_free(HomeTabCoordinator *this);
void homeCoordinator_start(HomeTabCoordinator *this);

void homeCoordinator_resetUI(HomeTabCoordinator *this);
void homeCoordinator_updateUI(HomeTabCoordinator *this);
void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index);
void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index);
void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, unsigned char type,
                                                    int index, int sets, int reps, int weight);
void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this, int totalCompletedWorkouts);
void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this);

#endif /* HomeTabCoordinator_h */
