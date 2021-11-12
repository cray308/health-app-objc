//
//  ExerciseManager.c
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#include "ExerciseManager.h"
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include <objc/message.h>
#include "AppUserData.h"
#include "CocoaHelpers.h"
#include "Workout.h"

#define copyStringRef(x, y) x = CFStringCreateCopy(NULL, y)

gen_array_source(str, CFStringRef, copyStringRef, CFRelease)

struct DictWrapper {
    CFDictionaryRef root;
    CFDictionaryRef lib;
};

static struct __WorkoutKeys {
    CFStringRef const reps;
    CFStringRef const type;
    CFStringRef const index;
    CFStringRef const title;
} const Keys = {CFSTR("reps"), CFSTR("type"), CFSTR("index"), CFSTR("title")};

static CFStringRef ExerciseStates[4];

static void createRootAndLibDict(struct DictWrapper *data) {
#if DEBUG
    CFStringRef path = ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef))objc_msgSend)
    (getBundle(), sel_getUid("pathForResource:ofType:"), CFSTR("WorkoutData_d"), CFSTR("plist"));
#else
    CFStringRef path = ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef))objc_msgSend)
    (getBundle(), sel_getUid("pathForResource:ofType:"), CFSTR("WorkoutData"), CFSTR("plist"));
#endif
    data->root = getDict(allocClass("NSDictionary"), sel_getUid("initWithContentsOfFile:"), path);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("library"));
}

static CFArrayRef getLibraryArrayForType(struct DictWrapper *data, byte type) {
    static CFStringRef const keys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
    if (type > 3) return NULL;
    return CFDictionaryGetValue(data->lib, keys[type]);
}

static CFArrayRef getCurrentWeekForPlan(struct DictWrapper *data, byte plan, int week) {
    static CFStringRef const planKeys[] = {CFSTR("bb"), CFSTR("cc")};
    CFDictionaryRef plans = CFDictionaryGetValue(data->root, CFSTR("plans"));
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);
    if (week >= CFArrayGetCount(weeks)) return NULL;
    return CFArrayGetValueAtIndex(weeks, week);
}

static void buildWorkoutFromDict(CFDictionaryRef dict, WorkoutParams *params, Workout *w) {
    CFArrayRef foundActivities = CFDictionaryGetValue(dict, CFSTR("activities"));
    CFNumberRef number;
    CFStringRef str;
    unsigned int tempInt = 0;

    int nActivities = (int) CFArrayGetCount(foundActivities);

    w->type = params->type;
    w->activities = array_new(circuit);
    str = CFDictionaryGetValue(dict, Keys.title);
    w->title = CFStringCreateCopy(NULL, str);

    if (nActivities <= 0) return;

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        Circuit activities = {.exercises = array_new(exEntry), .timer = &w->timers[TimerGroup]};

        number = CFDictionaryGetValue(act, Keys.type);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        activities.type = (byte) tempInt;
        number = CFDictionaryGetValue(act, Keys.reps);
        CFNumberGetValue(number, kCFNumberIntType, &activities.reps);

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("exercises"));
        int nExercises = (int) CFArrayGetCount(foundExercises);
        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef ex = CFArrayGetValueAtIndex(foundExercises, j);
            ExerciseEntry exercise = {
                .sets = 1, .timer = &w->timers[TimerExercise], .stateNames = ExerciseStates
            };

            number = CFDictionaryGetValue(ex, Keys.type);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            exercise.type = (byte) tempInt;
            number = CFDictionaryGetValue(ex, Keys.reps);
            CFNumberGetValue(number, kCFNumberIntType, &exercise.reps);
            number = CFDictionaryGetValue(ex, CFSTR("rest"));
            CFNumberGetValue(number, kCFNumberIntType, &exercise.rest);
            str = CFDictionaryGetValue(ex, CFSTR("name"));
            exercise.name = CFStringCreateCopy(NULL, str);
            array_push_back(exEntry, activities.exercises, exercise);
        }

        if (activities.type == CircuitDecrement) {
            activities.completedReps = activities.exercises->arr[0].reps;
        }
        array_push_back(circuit, w->activities, activities);
    }

    Array_exEntry *exercises = w->activities->arr[0].exercises;
    ExerciseEntry *e;
    switch (params->type) {
        case WorkoutStrength: ;
            short *lifts = userData->liftMaxes;
            int nExercises = exercises->size;
            float multiplier = params->weight / 100.f;
            array_iter(exercises, e) {
                e->sets = params->sets;
                e->reps = params->reps;
            }
            ExerciseEntry *entries = exercises->arr;
            entries[0].weight = (int) (multiplier * lifts[0]);

            if (nExercises >= 3 && params->index <= 1) {
                entries[1].weight = (int) (multiplier * lifts[LiftBench]);
                if (params->index == 0) {
                    entries[2].weight = (int) (multiplier * lifts[LiftPullup]);
                } else {
                    entries[2].weight = (int) (multiplier * lifts[LiftDeadlift]);
                }
            } else if (nExercises >= 4 && params->index == 2) {
                for (int i = 1; i < 4; ++i) {
                    entries[i].weight = lifts[i];
                }
            }
            break;
        case WorkoutSE:
            w->activities->arr[0].reps = params->sets;
            array_iter(exercises, e) {
                e->reps = params->reps;
            }
            break;
        case WorkoutEndurance: ;
            int duration = params->reps * 60;
            array_iter(exercises, e) {
                e->reps = duration;
            }
        default:
            break;
    }
    w->group = &w->activities->arr[0];
    w->entry = &w->group->exercises->arr[0];
    for (int i = 0; i < 2; ++i) {
        memcpy(&w->timers[i], &(WorkoutTimer){.info = {.type = i}}, sizeof(WorkoutTimer));
        pthread_mutex_init(&w->timers[i].lock, NULL);
        pthread_cond_init(&w->timers[i].cond, NULL);
    }
    w->timers[TimerGroup].exercise = ExerciseTagNA;
}

void initExerciseMgrStrings(void) {
    fillStringArray(ExerciseStates, CFSTR("exerciseState%d"), 4);
}

void workoutParams_init(WorkoutParams *this, signed char day) {
    memcpy(this, &(WorkoutParams){day, 0, 0, 1, 1, 1}, sizeof(WorkoutParams));
}

void exerciseManager_setWeeklyWorkoutNames(byte plan, int week, CFStringRef *names) {
    struct DictWrapper info;
    createRootAndLibDict(&info);

    CFArrayRef currWeek = getCurrentWeekForPlan(&info, plan, week);
    if (!currWeek) goto cleanup;
    int index = 0, tempInt = 0;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);

        CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        byte type = (byte) tempInt;
        number = CFDictionaryGetValue(day, Keys.index);
        CFNumberGetValue(number, kCFNumberIntType, &index);

        CFArrayRef libArr = getLibraryArrayForType(&info, type);
        if (!libArr) continue;

        CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
        CFStringRef str = CFDictionaryGetValue(foundWorkout, Keys.title);
        names[i] = CFStringCreateCopy(NULL, str);
    }
cleanup:
    CFRelease(info.root);
}

Workout *exerciseManager_getWeeklyWorkoutAtIndex(byte plan, int week, int index) {
    Workout *w = NULL;
    WorkoutParams params;
    workoutParams_init(&params, (signed char) index);
    struct DictWrapper info;
    createRootAndLibDict(&info);

    CFArrayRef currWeek = getCurrentWeekForPlan(&info, plan, week);
    if (!currWeek) goto cleanup;
    CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, index);

    CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
    CFNumberGetValue(number, kCFNumberIntType, &params.index);
    params.type = (byte) params.index;

    CFArrayRef libArr = getLibraryArrayForType(&info, params.type);
    if (!libArr) goto cleanup;

    number = CFDictionaryGetValue(day, Keys.index);
    CFNumberGetValue(number, kCFNumberIntType, &params.index);
    number = CFDictionaryGetValue(day, CFSTR("sets"));
    CFNumberGetValue(number, kCFNumberIntType, &params.sets);
    number = CFDictionaryGetValue(day, Keys.reps);
    CFNumberGetValue(number, kCFNumberIntType, &params.reps);
    number = CFDictionaryGetValue(day, CFSTR("weight"));
    CFNumberGetValue(number, kCFNumberIntType, &params.weight);

    w = calloc(1, sizeof(Workout));
    w->day = index;

    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, params.index);
    buildWorkoutFromDict(foundWorkout, &params, w);
cleanup:
    CFRelease(info.root);
    return w;
}

Array_str *exerciseManager_getWorkoutNamesForType(byte type) {
    Array_str *results = NULL;
    struct DictWrapper info;
    createRootAndLibDict(&info);

    int len = 0;
    CFArrayRef libArr = getLibraryArrayForType(&info, type);
    if (!(libArr && (len = (int) CFArrayGetCount(libArr)))) goto cleanup;

    if (type == WorkoutStrength) len = 2;

    results = array_new(str);
    array_reserve(str, results, len);
    for (int i = 0; i < len; ++i) {
        CFDictionaryRef week = CFArrayGetValueAtIndex(libArr, i);
        CFStringRef title = CFDictionaryGetValue(week, Keys.title);
        array_push_back(str, results, title);
    }
cleanup:
    CFRelease(info.root);
    return results;
}

Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params) {
    Workout *w = NULL;
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef libArr = getLibraryArrayForType(&info, params->type);
    if (!libArr) goto cleanup;
    w = calloc(1, sizeof(Workout));
    w->day = -1;
    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, params->index);
    buildWorkoutFromDict(foundWorkout, params, w);
cleanup:
    CFRelease(info.root);
    return w;
}
