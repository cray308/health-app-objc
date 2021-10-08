//
//  HomeViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeViewModel_h
#define HomeViewModel_h

#include <CoreFoundation/CFBase.h>

typedef struct {
    CFStringRef workoutNames[7];
    CFStringRef timeNames[3];
    enum { Morning, Afternoon, Evening } timeOfDay;
} HomeViewModel;

void homeViewModel_init(HomeViewModel *model);

void homeViewModel_fetchData(HomeViewModel *model);
bool homeViewModel_updateTimeOfDay(HomeViewModel *model);
bool homeViewModel_hasWorkoutsForThisWeek(HomeViewModel *model);
bool homeViewModel_shouldShowConfetti(HomeViewModel *model, int totalCompletedWorkouts);

#endif /* HomeViewModel_h */
