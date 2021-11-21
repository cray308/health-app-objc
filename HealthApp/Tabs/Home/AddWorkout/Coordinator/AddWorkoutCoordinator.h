#ifndef AddWorkoutCoordinator_h
#define AddWorkoutCoordinator_h

#include <objc/runtime.h>
#include "ExerciseManager.h"

typedef struct {
    id navVC;
    Workout *workout;
    void *parent;
} AddWorkoutCoordinator;

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this);

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this);
void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this,
                                            bool dismissVC, bool showModalIfRequired, short *lifts);
void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this);

#endif /* AddWorkoutCoordinator_h */
