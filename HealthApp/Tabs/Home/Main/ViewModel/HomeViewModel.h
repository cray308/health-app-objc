//
//  HomeViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeViewModel_h
#define HomeViewModel_h

#include "array.h"
#import <Foundation/Foundation.h>
#import "Constants.h"

typedef struct HomeViewModel HomeViewModel;
typedef struct HomeTabCoordinator HomeTabCoordinator;
typedef struct HomeBarChartViewModel HomeBarChartViewModel;
typedef struct HomePieChartViewModel HomePieChartViewModel;
typedef struct AlertDetails AlertDetails;

gen_array(SingleActivityModel, SingleActivityModel, DSDefault_shallowCopy, DSDefault_shallowDelete)

struct HomeViewModel {
    HomeTabCoordinator *delegate;
    HomeBarChartViewModel *barChartViewModel;
    HomePieChartViewModel *pieChartViewModel;
    NSString *currentTokenTextFormat;
    enum { Morning, Afternoon, Evening } timeOfDay;
};

void homeViewModel_clear(HomeViewModel *model);
void homeViewModel_fetchData(HomeViewModel *model);
int homeViewModel_addNewTokens(HomeViewModel *model, int newTokens);
void homeViewModel_addIntensityDurations(HomeViewModel *model, int *durations);

bool homeViewModel_updateTimeOfDay(HomeViewModel *model);
void homeViewModel_updateForNewDay(HomeViewModel *model, int mostRecentDay, int currentDay);

NSString *homeViewModel_getGreeting(HomeViewModel *model);
NSString *homeViewModel_getCurrentTokensText(HomeViewModel *model);
AlertDetails *homeViewModel_getAlertDetailsForMeetingTokenGoal(void);

#endif /* HomeViewModel_h */
