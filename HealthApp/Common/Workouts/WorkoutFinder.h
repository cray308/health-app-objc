//
//  WorkoutFinder.h
//  HealthApp
//
//  Created by Christopher Ray on 6/7/21.
//

#ifndef WorkoutFinder_h
#define WorkoutFinder_h

#include "Exercise.h"

void workoutFinder_setWeeklyWorkoutNames(unsigned char plan, unsigned int week, CFStringRef *names);
Workout *workoutFinder_getWeeklyWorkoutAtIndex(unsigned char plan, unsigned int week, unsigned int index);
CFStringRef *workoutFinder_get_workout_names(unsigned char type, unsigned int *size);
Workout *workoutFinder_get_workout_from_library(unsigned char type, unsigned int index, unsigned int reps, unsigned int sets, unsigned int weight);

#endif /* WorkoutFinder_h */
