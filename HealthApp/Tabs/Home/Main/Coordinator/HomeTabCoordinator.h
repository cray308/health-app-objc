//
//  HomeTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeTabCoordinator_h
#define HomeTabCoordinator_h

#include "CocoaBridging.h"
#include "HomeViewModel.h"

typedef struct {
    id navVC;
    HomeViewModel viewModel;
    void *childCoordinator;
} HomeTabCoordinator;

void homeCoordinator_start(HomeTabCoordinator *this);

void homeCoordinator_resetUI(HomeTabCoordinator *this);
void homeCoordinator_updateUI(HomeTabCoordinator *this);
void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index);
void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index);
void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, unsigned char type,
                                                    int index, short *params);
void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this, int totalCompletedWorkouts);
void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this);

#endif /* HomeTabCoordinator_h */
