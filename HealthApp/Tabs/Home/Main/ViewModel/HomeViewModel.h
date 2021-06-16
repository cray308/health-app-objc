//
//  HomeViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeViewModel_h
#define HomeViewModel_h

#import <UIKit/UIKit.h>
#include "Exercise.h"

typedef struct HomeViewModel HomeViewModel;
typedef struct HomeTabCoordinator HomeTabCoordinator;

struct HomeViewModel {
    HomeTabCoordinator *delegate;
    CFStringRef workoutNames[7];
    enum { Morning, Afternoon, Evening } timeOfDay;
};

HomeViewModel *homeViewModel_init(void);
void homeViewModel_free(HomeViewModel *model);

void homeViewModel_fetchData(HomeViewModel *model);

bool homeViewModel_updateTimeOfDay(HomeViewModel *model);

void homeViewModel_handleDayWorkoutButtonTap(HomeViewModel *model, int index);

unsigned char homeViewModel_hasWorkoutsForThisWeek(HomeViewModel *model);
unsigned char homeViewModel_shouldShowConfetti(HomeViewModel *model, int totalCompletedWorkouts);

void homeViewModel_handleCustomWorkoutButtonTap(HomeViewModel *model, int index);
void homeViewModel_finishedSettingUpCustomWorkout(HomeViewModel *model, UIViewController *presenter, unsigned char type, unsigned int index, unsigned int sets, unsigned int reps, unsigned int weight);
void homeViewModel_cancelCustomWorkout(UIViewController *presenter);

#endif /* HomeViewModel_h */
