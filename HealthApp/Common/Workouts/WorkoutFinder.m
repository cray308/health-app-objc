//
//  WorkoutFinder.m
//  HealthApp
//
//  Created by Christopher Ray on 6/7/21.
//

#import "WorkoutFinder.h"

Array_workout *workoutFinder_get_weekly_workouts(signed char plan, size_t week) {
    NSString *planStr;
    switch (plan) {
        case FitnessPlanBaseBuilding:
            planStr = @"bb";
            break;
        case FitnessPlanContinuation:
            planStr = @"cc";
            break;
        default:
            return NULL;
    }

    NSDictionary *dict = [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    NSDictionary *lib = [dict objectForKey:@"library"];
    NSDictionary *plans = [dict objectForKey:@"plans"];
    NSArray *weeks = [plans objectForKey:planStr];
    if (week >= weeks.count) {
        [dict release];
        return NULL;
    }

    NSArray *currWeek = weeks[week];
    Array_workout *resultArr = array_new(workout);


    for (size_t i = 0; i < currWeek.count; ++i) {
        NSDictionary *day = currWeek[i];
        unsigned idx = [day[@"index"] unsignedIntValue];
        unsigned sets = [day[@"sets"] unsignedIntValue];
        unsigned reps = [day[@"reps"] unsignedIntValue];
        Workout wk = { .type = (unsigned char) [day[@"type"] unsignedIntValue], .weight = [day[@"weight"] unsignedIntValue] };

        NSString *libraryKey;
        switch (wk.type) {
            case WorkoutTypeStrength:
                libraryKey = @"st";
                break;
            case WorkoutTypeSE:
                libraryKey = @"se";
                break;
            case WorkoutTypeEndurance:
                libraryKey = @"en";
                break;
            case WorkoutTypeHIC:
                libraryKey = @"hi";
                break;
            default:
                continue;
        }
        wk.activities = array_new(exGroup);
        wk.day = (unsigned int) i;

        NSArray *libArr = [lib objectForKey:libraryKey];
        NSDictionary *foundWorkout = libArr[idx];
        wk.title = [[NSString alloc] initWithString:foundWorkout[@"title"]];
        NSArray *foundActivities = foundWorkout[@"activities"];
        for (size_t j = 0; j < foundActivities.count; ++j) {
            NSDictionary *act = foundActivities[j];
            ExerciseGroup activities = {
                .type = (unsigned char) [act[@"type"] unsignedIntValue],
                .reps = [act[@"reps"] unsignedIntValue]
            };
            activities.exercises = array_new(exEntry);

            NSArray *foundExercises = act[@"exercises"];
            for (size_t k = 0; k < foundExercises.count; ++k) {
                NSDictionary *ex = foundExercises[k];
                ExerciseEntry exercise = {
                    .type = (unsigned char) [ex[@"type"] unsignedIntValue],
                    //.reps = [ex[@"reps"] unsignedIntValue],
                    .sets = 1,
                    .rest = [ex[@"rest"] unsignedIntValue],
                    .name = [[NSString alloc] initWithString:ex[@"name"]]
                };

                array_push_back(exEntry, activities.exercises, exercise);
            }
            array_push_back(exGroup, wk.activities, activities);
        }

        ExerciseEntry *e;
        Array_exEntry *exercises = wk.activities->arr[0].exercises;
        switch (wk.type) {
            case WorkoutTypeStrength:
                exercises = wk.activities->arr[0].exercises;
                array_iter(exercises, e) {
                    e->sets = sets;
                    e->reps = reps;
                }
                break;
            case WorkoutTypeSE:
                wk.activities->arr[0].reps = sets;
                exercises = wk.activities->arr[0].exercises;
                array_iter(exercises, e) {
                    e->reps = reps;
                }
                break;
            default:
                break;
        }

        array_push_back(workout, resultArr, wk);
    }
    [dict release];
    return resultArr;
}
