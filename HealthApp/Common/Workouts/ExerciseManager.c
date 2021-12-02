#include "ExerciseManager.h"
#include <CoreFoundation/CFNumber.h>
#include "AppUserData.h"
#include "CocoaHelpers.h"

#if DEBUG
#define WK_DATA_PATH CFSTR("WorkoutData_d")
#else
#define WK_DATA_PATH CFSTR("WorkoutData")
#endif

#define freeExerciseEntry(x) CFRelease((x).name)
#define freeCircuit(x) array_free(exEntry, (x).exercises)

gen_array_source(exEntry, ExerciseEntry, DSDefault_shallowCopy, freeExerciseEntry)
gen_array_source(circuit, Circuit, DSDefault_shallowCopy, freeCircuit)

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

static void createRootAndLibDict(struct DictWrapper *data) {
    id bundle = getBundle();
    CFStringRef path = ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef))objc_msgSend)
    (bundle, sel_getUid("pathForResource:ofType:"), WK_DATA_PATH, CFSTR("plist"));
    id _dict = allocClass(objc_getClass("NSDictionary"));
    data->root = getDict(_dict, sel_getUid("initWithContentsOfFile:"), path);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("library"));
}

static CFArrayRef getLibraryArrayForType(struct DictWrapper *data, unsigned char type) {
    static CFStringRef const keys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
    if (type > 3) return NULL;
    return CFDictionaryGetValue(data->lib, keys[type]);
}

static CFArrayRef getCurrentWeekForPlan(struct DictWrapper *data, unsigned char plan, int week) {
    static CFStringRef const planKeys[] = {CFSTR("bb"), CFSTR("cc")};
    CFDictionaryRef plans = CFDictionaryGetValue(data->root, CFSTR("plans"));
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);
    if (week >= CFArrayGetCount(weeks)) return NULL;
    return CFArrayGetValueAtIndex(weeks, week);
}

static Workout *buildWorkoutFromDict(CFDictionaryRef dict, WorkoutParams *params) {
    CFArrayRef foundActivities = CFDictionaryGetValue(dict, CFSTR("activities"));
    CFNumberRef number;
    CFStringRef str;
    unsigned int tempInt = 0;

    int nActivities = (int) CFArrayGetCount(foundActivities);
    if (nActivities <= 0) return NULL;

    Workout *w = calloc(1, sizeof(Workout));
    w->day = params->day;
    w->type = params->type;
    w->activities = array_new(circuit);
    str = CFDictionaryGetValue(dict, Keys.title);
    w->title = CFStringCreateCopy(NULL, str);

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        Circuit activities = {.exercises = array_new(exEntry)};

        number = CFDictionaryGetValue(act, Keys.type);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        activities.type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(act, Keys.reps);
        CFNumberGetValue(number, kCFNumberIntType, &activities.reps);

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("exercises"));
        int nExercises = (int) CFArrayGetCount(foundExercises);
        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef ex = CFArrayGetValueAtIndex(foundExercises, j);
            ExerciseEntry exercise = {.sets = 1};

            number = CFDictionaryGetValue(ex, Keys.type);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            exercise.type = (unsigned char) tempInt;
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
        case WorkoutEndurance:
            exercises->arr[0].reps = params->reps * 60;
        default:
            break;
    }
    w->group = &w->activities->arr[0];
    w->entry = &w->group->exercises->arr[0];
    return w;
}

void workoutParams_init(WorkoutParams *this, signed char day) {
    memcpy(this, &(WorkoutParams){day, 0, 0, 1, 1, 1}, sizeof(WorkoutParams));
}

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names) {
    struct DictWrapper info;
    createRootAndLibDict(&info);

    CFArrayRef currWeek = getCurrentWeekForPlan(&info, plan, week);
    if (!currWeek) goto cleanup;
    int index = 0, tempInt = 0;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);

        CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        unsigned char type = (unsigned char) tempInt;
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

Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index) {
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
    params.type = (unsigned char) params.index;

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

    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, params.index);
    w = buildWorkoutFromDict(foundWorkout, &params);
cleanup:
    CFRelease(info.root);
    return w;
}

CFArrayRef exerciseManager_createWorkoutNames(unsigned char type) {
    CFMutableArrayRef results = NULL;
    struct DictWrapper info;
    createRootAndLibDict(&info);

    int len = 0;
    CFArrayRef libArr = getLibraryArrayForType(&info, type);
    if (!(libArr && (len = (int) CFArrayGetCount(libArr)))) goto cleanup;

    if (type == WorkoutStrength) len = 2;

    CFArrayCallBacks callbacks = kCFTypeArrayCallBacks;
    callbacks.retain = NULL;
    results = CFArrayCreateMutable(NULL, len, &callbacks);
    for (int i = 0; i < len; ++i) {
        CFDictionaryRef week = CFArrayGetValueAtIndex(libArr, i);
        CFStringRef title = CFDictionaryGetValue(week, Keys.title);
        CFStringRef titleCopy = CFStringCreateCopy(NULL, title);
        CFArrayAppendValue(results, titleCopy);
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
    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, params->index);
    w = buildWorkoutFromDict(foundWorkout, params);
cleanup:
    CFRelease(info.root);
    return w;
}
