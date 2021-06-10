//
//  HomeViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeViewModel_h
#define HomeViewModel_h

#import <Foundation/Foundation.h>
#import "Constants.h"
#import "Exercise.h"

typedef struct HomeViewModel HomeViewModel;
typedef struct HomeTabCoordinator HomeTabCoordinator;

struct HomeViewModel {
    HomeTabCoordinator *delegate;
    Array_workout *workouts;
    enum { Morning, Afternoon, Evening } timeOfDay;
};

void homeViewModel_clear(HomeViewModel *model);
void homeViewModel_fetchData(HomeViewModel *model);

bool homeViewModel_updateTimeOfDay(HomeViewModel *model);

NSString *homeViewModel_getGreeting(HomeViewModel *model);

#endif /* HomeViewModel_h */
