//
//  AddWorkoutCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutCoordinator_h
#define AddWorkoutCoordinator_h

#include "CocoaHelpers.h"
#include "Exercise.h"

typedef struct {
    id navVC;
    Workout *workout;
    void *parent;
} AddWorkoutCoordinator;

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this);
void addWorkoutCoordinator_free(AddWorkoutCoordinator *this);

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this);
void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this,
                                            bool dismissVC, bool showModalIfRequired);
void addWorkoutCoordinator_finishedAddingNewWeights(AddWorkoutCoordinator *this, short *weights);
void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this);

#endif /* AddWorkoutCoordinator_h */
