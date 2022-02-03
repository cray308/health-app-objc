#include "ExerciseManager.h"
#include <CoreFoundation/CFNumber.h>
#include <stdlib.h>
#include <string.h>
#include "AppUserData.h"
#include "CocoaHelpers.h"

#if DEBUG
#define WK_DATA_PATH CFSTR("WorkoutData_d")
#else
#define WK_DATA_PATH CFSTR("WorkoutData")
#endif

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

static CFStringRef repsFormat;
static CFStringRef setsFormat;
static CFStringRef roundsFormat;
static CFStringRef restFormat;
static CFStringRef weightFormat;
static CFStringRef durationMinsFormat;
static CFStringRef durationSecsFormat;
static CFStringRef distanceFormat;
static CFStringRef amrapFormat;

static void createRootAndLibDict(struct DictWrapper *data) {
    id bundle = getBundle();
    id url = (((id(*)(id,SEL,CFStringRef,CFStringRef))objc_msgSend)
              (bundle, sel_getUid("URLForResource:withExtension:"), WK_DATA_PATH, CFSTR("plist")));
    id _dict = allocClass(objc_getClass("NSDictionary"));
    data->root = (((CFDictionaryRef(*)(id,SEL,id,id))objc_msgSend)
                  (_dict, sel_getUid("initWithContentsOfURL:error:"), url, nil));
    data->lib = CFDictionaryGetValue(data->root, CFSTR("library"));
}

static CFArrayRef getLibraryArrayForType(struct DictWrapper *data, unsigned char type) {
    static CFStringRef const keys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
    return CFDictionaryGetValue(data->lib, keys[type]);
}

static CFArrayRef getCurrentWeekForPlan(struct DictWrapper *data) {
    static CFStringRef const planKeys[] = {CFSTR("bb"), CFSTR("cc")};
    CFDictionaryRef plans = CFDictionaryGetValue(data->root, CFSTR("plans"));
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[userData->currentPlan]);
    return CFArrayGetValueAtIndex(weeks, userData->week);
}

static Workout *buildWorkoutFromDict(CFDictionaryRef dict, WorkoutParams *params) {
    CFArrayRef foundActivities = CFDictionaryGetValue(dict, CFSTR("activities"));
    CFNumberRef number;
    CFStringRef str;
    unsigned int tempInt = 0;

    unsigned nActivities = (unsigned) CFArrayGetCount(foundActivities);
    customAssert(nActivities > 0)

    Workout *w = calloc(1, sizeof(Workout));
    w->day = params->day;
    w->type = params->type;
    w->activities = calloc(nActivities, sizeof(Circuit));
    w->size = nActivities;
    str = CFDictionaryGetValue(dict, Keys.title);
    w->title = CFStringCreateCopy(NULL, str);

    bool addWeights = false;
    short weights[4];
    if (params->type == WorkoutStrength) {
        short *lifts = userData->liftMaxes;
        float multiplier = params->weight / 100.f;
        addWeights = true;

        weights[0] = (short) (multiplier * lifts[0]);
        if (params->index <= 1) {
            weights[1] = (short) (multiplier * lifts[LiftBench]);
            if (params->index == 0) {
                short bodyWeight = getBodyWeight();
                weights[2] = (short) ((lifts[LiftPullup] + bodyWeight) * multiplier) - bodyWeight;
                weights[2] = max(weights[2], 0);
            } else {
                weights[2] = (short) (multiplier * lifts[LiftDeadlift]);
            }
        } else {
            memcpy(&weights[1], &lifts[1], 3 * sizeof(short));
        }
    }

    for (unsigned i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        Circuit circuit = {0};
        unsigned customReps = 0, customSets = 0;

        number = CFDictionaryGetValue(act, Keys.type);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        circuit.type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(act, Keys.reps);
        CFNumberGetValue(number, kCFNumberIntType, &circuit.reps);

        if (params->type == WorkoutSE) {
            circuit.reps = params->sets;
            customReps = params->reps;
        } else if (params->type == WorkoutEndurance) {
            customReps = params->reps * 60;
        } else if (params->type == WorkoutStrength) {
            customReps = params->reps;
            customSets = params->sets;
        }

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("exercises"));
        unsigned nExercises = (unsigned) CFArrayGetCount(foundExercises);
        circuit.exercises = calloc(nExercises, sizeof(ExerciseEntry));
        circuit.size = nExercises;

        for (unsigned j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            ExerciseEntry e = {.sets = 1};

            number = CFDictionaryGetValue(exDict, Keys.type);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            e.type = (unsigned char) tempInt;
            number = CFDictionaryGetValue(exDict, Keys.reps);
            CFNumberGetValue(number, kCFNumberIntType, &e.reps);
            number = CFDictionaryGetValue(exDict, CFSTR("rest"));
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            str = CFDictionaryGetValue(exDict, CFSTR("name"));

            if (customReps)
                e.reps = customReps;
            if (customSets)
                e.sets = customSets;
            if (tempInt)
                e.restStr = CFStringCreateWithFormat(NULL, NULL, restFormat, tempInt);

            if (e.sets > 1) {
                e.headerStr = CFStringCreateMutable(NULL, 16);
                CFStringRef numberStr = CFStringCreateWithFormat(NULL, NULL, setsFormat, 1, e.sets);
                e.hRange = CFStringFind(numberStr, CFSTR("1"), 0);
                CFStringReplaceAll(e.headerStr, numberStr);
                CFRelease(numberStr);
            }

            CFStringRef titleStr;

            switch (e.type) {
                case ExerciseReps:
                    if (addWeights) {
                        titleStr = CFStringCreateWithFormat(NULL, NULL, weightFormat,
                                                            str, e.reps, weights[j]);
                    } else {
                        titleStr = CFStringCreateWithFormat(NULL, NULL, repsFormat,
                                                            str, e.reps);
                    }
                    break;

                case ExerciseDuration:
                    if (e.reps > 120) {
                        titleStr = CFStringCreateWithFormat(NULL, NULL, durationMinsFormat,
                                                            str, e.reps / 60.f);
                    } else {
                        titleStr = CFStringCreateWithFormat(NULL, NULL, durationSecsFormat,
                                                            str, e.reps);
                    }
                    break;

                default:
                    titleStr = CFStringCreateWithFormat(NULL, NULL, distanceFormat,
                                                        e.reps, (5 * e.reps) >> 2);
            }

            e.titleStr = CFStringCreateMutable(NULL, 64);
            CFStringReplaceAll(e.titleStr, titleStr);
            CFRelease(titleStr);
            if (circuit.type == CircuitDecrement && e.type == ExerciseReps)
                e.tRange = CFStringFind(e.titleStr, CFSTR("10"), 0);

            memcpy(&circuit.exercises[j], &e, sizeof(ExerciseEntry));
        }

        CFStringRef headerStr = NULL;
        if (circuit.type == CircuitDecrement) {
            circuit.completedReps = circuit.exercises[0].reps;
        } else if (circuit.type == CircuitAMRAP) {
            headerStr = CFStringCreateWithFormat(NULL, NULL, amrapFormat, circuit.reps);
        } else if (circuit.reps > 1) {
            headerStr = CFStringCreateWithFormat(NULL, NULL, roundsFormat, 1, circuit.reps);
            circuit.numberRange = CFStringFind(headerStr, CFSTR("1"), 0);
        }

        if (headerStr) {
            circuit.headerStr = CFStringCreateMutable(NULL, 16);
            CFStringReplaceAll(circuit.headerStr, headerStr);
            CFRelease(headerStr);
        }

        memcpy(&w->activities[i], &circuit, sizeof(Circuit));
    }

    if (CFStringCompareWithOptions(w->title, localize(CFSTR("workoutTitleTestDay")),
                                   (CFRange){0, CFStringGetLength(w->title)}, 0) == 0) {
        w->testMax = true;
    }
    w->group = &w->activities[0];
    return w;
}

void initExerciseStrings(void) {
    restFormat = localize(CFSTR("exerciseTitleRest"));
    repsFormat = localize(CFSTR("exerciseTitleReps"));
    setsFormat = localize(CFSTR("exerciseHeader"));
    weightFormat = localize(CFSTR("exerciseTitleRepsWithWeight"));
    durationMinsFormat = localize(CFSTR("exerciseTitleDurationMinutes"));
    durationSecsFormat = localize(CFSTR("exerciseTitleDurationSeconds"));
    distanceFormat = localize(CFSTR("exerciseTitleDistance"));
    roundsFormat = localize(CFSTR("circuitHeaderRounds"));
    amrapFormat = localize(CFSTR("circuitHeaderAMRAP"));
}

void exerciseManager_setWeeklyWorkoutNames(CFStringRef *names) {
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef currWeek = getCurrentWeekForPlan(&info);
    int tempInt = 0;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        if (tempInt > 3) continue;

        CFArrayRef libArr = getLibraryArrayForType(&info, (unsigned char) tempInt);
        number = CFDictionaryGetValue(day, Keys.index);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        CFStringRef str = CFDictionaryGetValue(CFArrayGetValueAtIndex(libArr, tempInt), Keys.title);
        names[i] = CFStringCreateCopy(NULL, str);
    }
    CFRelease(info.root);
}

Workout *exerciseManager_getWeeklyWorkout(int index) {
    WorkoutParams params = {(signed char) index, 0, 0, 1, 1, 1};
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeekForPlan(&info), index);

    CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
    CFNumberGetValue(number, kCFNumberIntType, &params.index);
    params.type = (unsigned char) params.index;

    CFArrayRef libArr = getLibraryArrayForType(&info, params.type);

    number = CFDictionaryGetValue(day, Keys.index);
    CFNumberGetValue(number, kCFNumberIntType, &params.index);
    number = CFDictionaryGetValue(day, CFSTR("sets"));
    CFNumberGetValue(number, kCFNumberIntType, &params.sets);
    number = CFDictionaryGetValue(day, Keys.reps);
    CFNumberGetValue(number, kCFNumberIntType, &params.reps);
    number = CFDictionaryGetValue(day, CFSTR("weight"));
    CFNumberGetValue(number, kCFNumberIntType, &params.weight);

    Workout *w = buildWorkoutFromDict(CFArrayGetValueAtIndex(libArr, params.index), &params);
    CFRelease(info.root);
    return w;
}

CFArrayRef exerciseManager_createWorkoutNames(unsigned char type) {
    struct DictWrapper info;
    createRootAndLibDict(&info);

    CFArrayRef libArr = getLibraryArrayForType(&info, type);
    int len = (int) CFArrayGetCount(libArr);
    if (type == WorkoutStrength)
        len = 2;

    CFMutableArrayRef results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFStringRef title = CFDictionaryGetValue(CFArrayGetValueAtIndex(libArr, i), Keys.title);
        CFArrayAppendValue(results, title);
    }
    CFRelease(info.root);
    return results;
}

Workout *exerciseManager_getWorkoutFromLibrary(WorkoutParams *params) {
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef libArr = getLibraryArrayForType(&info, params->type);
    Workout *w = buildWorkoutFromDict(CFArrayGetValueAtIndex(libArr, params->index), params);
    CFRelease(info.root);
    return w;
}
