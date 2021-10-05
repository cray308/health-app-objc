//
//  HomeTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeTabCoordinator_h
#define HomeTabCoordinator_h

#include "CocoaHelpers.h"
#include "HomeViewModel.h"

typedef struct {
    id navVC;
    HomeViewModel model;
    void *childCoordinator;
} HomeTabCoordinator;

void homeCoordinator_start(HomeTabCoordinator *this);

void homeCoordinator_resetUI(HomeTabCoordinator *this);
void homeCoordinator_updateUI(HomeTabCoordinator *this);
void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index);
void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index);
void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, void *params);
void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this);

#endif /* HomeTabCoordinator_h */
