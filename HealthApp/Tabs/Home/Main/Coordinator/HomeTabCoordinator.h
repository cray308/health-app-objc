#ifndef HomeTabCoordinator_h
#define HomeTabCoordinator_h

#include <objc/runtime.h>

typedef struct {
    id navVC;
    void *childCoordinator;
} HomeTabCoordinator;

void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index);
void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index);
void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, void *params);
void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this);

#endif /* HomeTabCoordinator_h */
