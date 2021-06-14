//
//  WorkoutFinder.m
//  HealthApp
//
//  Created by Christopher Ray on 6/7/21.
//

#import "WorkoutFinder.h"
#import "AppUserData.h"
#import <Foundation/Foundation.h>

static CFStringRef libraryKeys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
static CFStringRef libraryKey = CFSTR("library");
static CFStringRef planKeys[] = {CFSTR("bb"), CFSTR("cc")};
static CFStringRef plansKey = CFSTR("plans");
static CFStringRef indexKey = CFSTR("index");
static CFStringRef setsKey = CFSTR("sets");
static CFStringRef repsKey = CFSTR("reps");
static CFStringRef weightKey = CFSTR("weight");
static CFStringRef typeKey = CFSTR("type");
static CFStringRef titleKey = CFSTR("title");
static CFStringRef activitiesKey = CFSTR("activities");
static CFStringRef exercisesKey = CFSTR("exercises");
static CFStringRef restKey = CFSTR("rest");
static CFStringRef nameKey = CFSTR("name");

CFArrayRef getLibraryArrayForType(CFDictionaryRef libDict, unsigned char type) {
    if (type > 3) return NULL;
    CFArrayRef arr = CFDictionaryGetValue(libDict, libraryKeys[type]);
    return arr;
}

void fillWorkoutForLibraryEntry(CFArrayRef libArr, unsigned int index, unsigned char type, unsigned int sets, unsigned int reps, unsigned int weight, Workout *w) {
    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
    CFArrayRef foundActivities = CFDictionaryGetValue(foundWorkout, activitiesKey);

    int nActivities = (int) CFArrayGetCount(foundActivities);
    CFNumberRef number;
    CFStringRef str;
    unsigned int tempInt = 0;

    w->type = type;
    workout_setup_activities(w);
    str = CFDictionaryGetValue(foundWorkout, titleKey);
    w->title = CFStringCreateCopy(NULL, str);

    for (int j = 0; j < nActivities; ++j) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, j);
        ExerciseGroup activities = {0};
        exerciseGroup_setup_exercises(&activities);

        number = CFDictionaryGetValue(act, typeKey);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        activities.type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(act, repsKey);
        CFNumberGetValue(number, kCFNumberIntType, &activities.reps);

        CFArrayRef foundExercises = CFDictionaryGetValue(act, exercisesKey);
        for (int k = 0; k < CFArrayGetCount(foundExercises); ++k) {
            CFDictionaryRef ex = CFArrayGetValueAtIndex(foundExercises, k);
            ExerciseEntry exercise = { .sets = 1 };

            number = CFDictionaryGetValue(ex, typeKey);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            exercise.type = (unsigned char) tempInt;
            number = CFDictionaryGetValue(ex, repsKey);
            CFNumberGetValue(number, kCFNumberIntType, &exercise.reps);
            number = CFDictionaryGetValue(ex, restKey);
            CFNumberGetValue(number, kCFNumberIntType, &exercise.rest);
            str = CFDictionaryGetValue(ex, nameKey);
            exercise.name = CFStringCreateCopy(NULL, str);
            exerciseGroup_add_exercise(&activities, &exercise);
        }
        workout_add_activity(w, &activities);
    }

    ExerciseGroup *g = workout_getExerciseGroup(w, 0);
    switch (type) {
        case WorkoutTypeStrength: ;
            int nExercises = exerciseGroup_getNumberOfExercises(g);
            double weightMultiplier = (double) weight / 100.0;
            unsigned int weights[4] = {(unsigned int) (weightMultiplier * (double) appUserDataShared->squatMax), 0, 0, 0};
            workout_setSetsAndRepsForExercises(w, sets, reps);

            if (nExercises >= 3 && index <= 1) {
                weights[1] = (unsigned int) (weightMultiplier * (double) appUserDataShared->benchMax);
                if (index == 0) {
                    weights[2] = (unsigned int) (weightMultiplier * (double) appUserDataShared->pullUpMax);
                } else {
                    weights[2] = (unsigned int) (weightMultiplier * (double) appUserDataShared->deadliftMax);
                }
                workout_setWeightsForExercises(w, weights, 3);
            } else if (nExercises >= 4 && index == 2) {
                weights[1] = (appUserDataShared->pullUpMax);
                weights[2] = (unsigned int) (appUserDataShared->benchMax);
                weights[3] = (unsigned int) (appUserDataShared->deadliftMax);
                workout_setWeightsForExercises(w, weights, 4);
            }
            break;
        case WorkoutTypeSE:
            g->reps = sets;
            workout_setSetsAndRepsForExercises(w, 1, reps);
            break;
        case WorkoutTypeEndurance:
            workout_setSetsAndRepsForExercises(w, 1, reps * 60);
            break;
        default:
            break;
    }
}

void workoutFinder_setWeeklyWorkoutNames(unsigned char plan, unsigned int week, CFStringRef *names) {
    if (plan > 1) return;

    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);
    CFDictionaryRef plans = CFDictionaryGetValue(root, plansKey);
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);

    if (week >= CFArrayGetCount(weeks)) {
        CFRelease(root);
        return;
    }

    CFArrayRef currWeek = CFArrayGetValueAtIndex(weeks, week);
    unsigned int index = 0, tempInt = 0;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);

        CFNumberRef number = CFDictionaryGetValue(day, typeKey);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        unsigned char type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(day, indexKey);
        CFNumberGetValue(number, kCFNumberIntType, &index);

        CFArrayRef libArr = getLibraryArrayForType(lib, type);
        if (!libArr) continue;

        CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
        CFStringRef str = CFDictionaryGetValue(foundWorkout, titleKey);
        names[i] = CFStringCreateCopy(NULL, str);
    }
    CFRelease(root);
}

Workout *workoutFinder_getWeeklyWorkoutAtIndex(unsigned char plan, unsigned int week, unsigned int index) {
    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);
    CFDictionaryRef plans = CFDictionaryGetValue(root, plansKey);
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);

    if (week >= CFArrayGetCount(weeks)) {
        CFRelease(root);
        return NULL;
    }

    CFArrayRef currWeek = CFArrayGetValueAtIndex(weeks, week);
    CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, index);
    unsigned int idx = 0, sets = 0, reps = 0, weight = 0;
    unsigned char type = 0;

    CFNumberRef number = CFDictionaryGetValue(day, typeKey);
    CFNumberGetValue(number, kCFNumberIntType, &idx);
    type = (unsigned char) idx;

    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!libArr) {
        CFRelease(root);
        return NULL;
    }

    number = CFDictionaryGetValue(day, indexKey);
    CFNumberGetValue(number, kCFNumberIntType, &idx);
    number = CFDictionaryGetValue(day, setsKey);
    CFNumberGetValue(number, kCFNumberIntType, &sets);
    number = CFDictionaryGetValue(day, repsKey);
    CFNumberGetValue(number, kCFNumberIntType, &reps);
    number = CFDictionaryGetValue(day, weightKey);
    CFNumberGetValue(number, kCFNumberIntType, &weight);

    Workout *w = calloc(1, sizeof(Workout));
    w->day = index;
    fillWorkoutForLibraryEntry(libArr, idx, type, sets, reps, weight, w);
    CFRelease(root);
    return w;
}

CFStringRef *workoutFinder_get_workout_names(unsigned char type, unsigned int *size) {
    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);

    long len = 0;
    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!(libArr && (len = CFArrayGetCount(libArr)))) {
        CFRelease(root);
        return NULL;
    }

    if (type == WorkoutTypeStrength) len = 2;

    CFStringRef *results = calloc(len, sizeof(CFStringRef));
    *size = (unsigned int) len;
    for (long i = 0; i < len; ++i) {
        CFDictionaryRef week = CFArrayGetValueAtIndex(libArr, i);
        CFStringRef title = CFDictionaryGetValue(week, titleKey);
        results[i] = CFStringCreateCopy(NULL, title);
    }
    CFRelease(root);
    return results;
}

Workout *workoutFinder_get_workout_from_library(unsigned char type, unsigned int index, unsigned int reps, unsigned int sets, unsigned int weight) {
    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);
    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!libArr) {
        CFRelease(root);
        return NULL;
    }
    Workout *w = calloc(1, sizeof(Workout));
    w->day = -1;
    fillWorkoutForLibraryEntry(libArr, index, type, sets, reps, weight, w);
    if (!workout_getNumberOfActivities(w)) {
        free(w);
        CFRelease(root);
        return NULL;
    }
    CFRelease(root);
    return w;
}
