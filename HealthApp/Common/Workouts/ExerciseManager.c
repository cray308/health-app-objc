#include "ExerciseManager.h"
#include <CoreFoundation/CFNumber.h>
#include "AppUserData.h"
#include "CocoaHelpers.h"

#if DEBUG
#define WK_DATA_PATH CFSTR("WorkoutData_d")
#else
#define WK_DATA_PATH CFSTR("WorkoutData")
#endif

#define freeExerciseEntry(x) do { \
    CFRelease((x).titleStr); \
    if ((x).restStr) CFRelease((x).restStr); \
    if ((x).headerStr) CFRelease((x).headerStr); \
} while (0)

#define freeCircuit(x) do { \
    if ((x).headerStr) CFRelease((x).headerStr); \
    array_free(exEntry, (x).exercises); \
} while (0)

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

    bool addWeights = false;
    int weights[4];
    if (params->type == WorkoutStrength) {
        short *lifts = userData->liftMaxes;
        float multiplier = params->weight / 100.f;
        addWeights = true;

        weights[0] = (int) (multiplier * lifts[0]);
        if (params->index <= 1) {
            weights[1] = (int) (multiplier * lifts[LiftBench]);
            if (params->index == 0) {
                weights[2] = (int) ((lifts[LiftPullup] + 145) * multiplier) - 145;
                weights[2] = max(weights[2], 0);
            } else {
                weights[2] = (int) (multiplier * lifts[LiftDeadlift]);
            }
        } else if (params->index == 2) {
            for (int i = 1; i < 4; ++i) {
                weights[i] = lifts[i];
            }
        }
    }

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        Circuit circuit = {.exercises = array_new(exEntry)};
        int customReps = 0, customSets = 0;

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
        int nExercises = (int) CFArrayGetCount(foundExercises);
        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef dict = CFArrayGetValueAtIndex(foundExercises, j);
            ExerciseEntry e = {.sets = 1};
            int rest = 0;

            number = CFDictionaryGetValue(dict, Keys.type);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            e.type = (unsigned char) tempInt;
            number = CFDictionaryGetValue(dict, Keys.reps);
            CFNumberGetValue(number, kCFNumberIntType, &e.reps);
            number = CFDictionaryGetValue(dict, CFSTR("rest"));
            CFNumberGetValue(number, kCFNumberIntType, &rest);
            str = CFDictionaryGetValue(dict, CFSTR("name"));

            if (customReps)
                e.reps = customReps;
            if (customSets)
                e.sets = customSets;
            if (rest)
                e.restStr = CFStringCreateWithFormat(NULL, NULL, restFormat, rest);

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

            array_push_back(exEntry, circuit.exercises, e);
        }

        CFStringRef headerStr = NULL;
        if (circuit.type == CircuitDecrement) {
            circuit.completedReps = circuit.exercises->arr[0].reps;
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

        array_push_back(circuit, w->activities, circuit);
    }

    if (CFStringCompareWithOptions(w->title, localize(CFSTR("workoutTitleTestDay")),
                                   (CFRange){0, CFStringGetLength(w->title)}, 0) == 0) {
        w->testMax = true;
    }
    w->group = &w->activities->arr[0];
    w->entry = &w->group->exercises->arr[0];
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

    if (type == WorkoutStrength)
        len = 2;

    results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFDictionaryRef week = CFArrayGetValueAtIndex(libArr, i);
        CFStringRef title = CFDictionaryGetValue(week, Keys.title);
        CFArrayAppendValue(results, title);
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
