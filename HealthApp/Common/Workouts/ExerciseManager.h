//
//  ExerciseManager.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef ExerciseManager_h
#define ExerciseManager_h

#include "Workout.h"

typedef struct {
    const signed char day;
    unsigned char type;
    int index;
    int sets;
    int reps;
    int weight;
} WorkoutParams;

gen_array_headers(str, CFStringRef)

void workoutParams_init(WorkoutParams *this, signed char day);

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names);
Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index);
Array_str *exerciseManager_getWorkoutNamesForType(unsigned char type);
Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params);

#endif /* ExerciseManager_h */
