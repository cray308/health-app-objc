#include "ExerciseManager.h"
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFSet.h>
#include <stdlib.h>
#include <string.h>
#include "AppUserData.h"
#include "CocoaHelpers.h"

extern id HKQuantityTypeIdentifierBodyMass;
extern id HKSampleSortIdentifierStartDate;

struct DictWrapper {
    CFDictionaryRef root;
    CFDictionaryRef lib;
};

static int weekInPlan;
static short bodyweight = 145;

static struct {
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
    CFBundleRef bundle = CFBundleGetMainBundle();
    CFURLRef url = CFBundleCopyResourceURL(bundle, CFSTR("WorkoutData"), CFSTR("plist"), NULL);
    CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, url);
    CFRelease(url);
    CFReadStreamOpen(stream);
    unsigned char *buf = malloc(12000);
    long nBytes = CFReadStreamRead(stream, buf, 12000);
    CFDataRef plistData = CFDataCreate(NULL, buf, nBytes);
    free(buf);
    CFReadStreamClose(stream);
    CFRelease(stream);
    CFPropertyListFormat format = kCFPropertyListXMLFormat_v1_0;
    data->root = CFPropertyListCreateWithData(NULL, plistData, 0, &format, NULL);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("library"));
    CFRelease(plistData);
}

static CFArrayRef getLibraryArrayForType(struct DictWrapper *data, unsigned char type) {
    static CFStringRef const keys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
    return CFDictionaryGetValue(data->lib, keys[type]);
}

static CFArrayRef getCurrentWeekForPlan(struct DictWrapper *data, unsigned char plan) {
    static CFStringRef const planKeys[] = {CFSTR("bb"), CFSTR("cc")};
    CFDictionaryRef plans = CFDictionaryGetValue(data->root, CFSTR("plans"));
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);
    return CFArrayGetValueAtIndex(weeks, weekInPlan);
}

static Workout *buildWorkout(CFDictionaryRef dict, WorkoutParams *params) {
    CFNumberRef number;
    short weights[4];
    short exerciseSets = 1, customReps = 0, customCircuitReps = 0;
    bool testMax = false;
    if (params->type == WorkoutStrength) {
        short *lifts = userData->liftMaxes;
        float multiplier = params->weight / 100.f;

        weights[0] = (short) (multiplier * lifts[0]);
        if (params->index <= 1) {
            weights[1] = (short) (multiplier * lifts[LiftBench]);
            if (params->index == 0) {
                short weight = (short) ((lifts[LiftPullup] + bodyweight) * multiplier) - bodyweight;
                if (weight < 0)
                    weight = 0;
                weights[2] = weight;
            } else {
                weights[2] = (short) (multiplier * lifts[LiftDeadlift]);
            }
        } else {
            memcpy(&weights[1], &lifts[1], 3 * sizeof(short));
            testMax = true;
        }
        customReps = params->reps;
        exerciseSets = params->sets;
    } else if (params->type == WorkoutSE) {
        customCircuitReps = params->sets;
        customReps = params->reps;
    } else if (params->type == WorkoutEndurance) {
        customReps = params->reps * 60;
    }

    CFArrayRef foundActivities = CFDictionaryGetValue(dict, CFSTR("activities"));
    int nActivities = (int) CFArrayGetCount(foundActivities);
    customAssert(nActivities > 0)

    Workout workout = {
        .activities = malloc((unsigned) nActivities * sizeof(Circuit)),
        .title = CFRetain(CFDictionaryGetValue(dict, Keys.title)), .size = nActivities,
        .bodyweight = bodyweight, .type = params->type, .day = params->day, .testMax = testMax
    };
    workout.group = &workout.activities[0];

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        CFMutableStringRef circuitHeaderMutable = NULL;
        CFStringRef circuitHeader = NULL;
        CFRange circuitRange = {0, 0};
        short circuitReps = customCircuitReps;
        unsigned char circuitType;

        number = CFDictionaryGetValue(act, Keys.type);
        CFNumberGetValue(number, kCFNumberCharType, &circuitType);
        if (!circuitReps) {
            number = CFDictionaryGetValue(act, Keys.reps);
            CFNumberGetValue(number, kCFNumberShortType, &circuitReps);
        }
#if TARGET_OS_SIMULATOR
        if (circuitType == CircuitAMRAP)
            circuitReps = nActivities > 1 ? 1 : 2;
#endif

        if (circuitType == CircuitAMRAP) {
            circuitHeader = CFStringCreateWithFormat(NULL, NULL, amrapFormat, circuitReps);
        } else if (circuitReps > 1) {
            circuitHeader = CFStringCreateWithFormat(NULL, NULL, roundsFormat, 1, circuitReps);
            circuitRange = CFStringFind(circuitHeader, CFSTR("1"), 0);
        }

        if (circuitHeader) {
            circuitHeaderMutable = CFStringCreateMutable(NULL, 16);
            CFStringReplaceAll(circuitHeaderMutable, circuitHeader);
            CFRelease(circuitHeader);
        }

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("exercises"));
        int nExercises = (int) CFArrayGetCount(foundExercises);
        customAssert(nExercises > 0)
        Circuit circuit = {
            .exercises = malloc((unsigned) nExercises * sizeof(ExerciseEntry)),
            .headerStr = circuitHeaderMutable, .numberRange = circuitRange,
            .size = nExercises, .reps = circuitReps, .type = circuitType
        };

        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            CFMutableStringRef exerciseHeader = NULL;
            CFRange exerciseHeaderRange = {0, 0};
            short rest = 0, exerciseReps = customReps;
            unsigned char exerciseType;

            number = CFDictionaryGetValue(exDict, Keys.type);
            CFNumberGetValue(number, kCFNumberCharType, &exerciseType);
            if (!exerciseReps) {
                number = CFDictionaryGetValue(exDict, Keys.reps);
                CFNumberGetValue(number, kCFNumberShortType, &exerciseReps);
            }
#if TARGET_OS_SIMULATOR
            if (workout.type == WorkoutHIC && exerciseType == ExerciseDuration && exerciseReps > 30)
                exerciseReps = 30;
#endif

            if (exerciseSets > 1) {
                exerciseHeader = CFStringCreateMutable(NULL, 16);
                CFStringRef numberStr = CFStringCreateWithFormat(NULL, NULL,
                                                                 setsFormat, 1, exerciseSets);
                exerciseHeaderRange = CFStringFind(numberStr, CFSTR("1"), 0);
                CFStringReplaceAll(exerciseHeader, numberStr);
                CFRelease(numberStr);
            }

            ExerciseEntry e = {
                .headerStr = exerciseHeader, .hRange = exerciseHeaderRange, .reps = exerciseReps,
                .sets = exerciseSets, .type = exerciseType
            };

            number = CFDictionaryGetValue(exDict, CFSTR("rest"));
            CFNumberGetValue(number, kCFNumberShortType, &rest);
            if (rest)
                e.restStr = CFStringCreateWithFormat(NULL, NULL, restFormat, rest);

            CFStringRef name = CFDictionaryGetValue(exDict, CFSTR("name"));
            CFStringRef titleStr;

            if (e.type == ExerciseReps) {
                if (!workout.type) {
                    titleStr = CFStringCreateWithFormat(NULL, NULL, weightFormat,
                                                        name, e.reps, weights[j]);
                } else {
                    titleStr = CFStringCreateWithFormat(NULL, NULL, repsFormat, name, e.reps);
                }
            } else if (e.type == ExerciseDuration) {
                if (e.reps > 120) {
                    titleStr = CFStringCreateWithFormat(NULL, NULL, durationMinsFormat,
                                                        name, e.reps / 60.f);
                } else {
                    titleStr = CFStringCreateWithFormat(NULL, NULL, durationSecsFormat,
                                                        name, e.reps);
                }
            } else {
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

        if (circuit.type == CircuitDecrement)
            circuit.completedReps = circuit.exercises[0].reps;
        memcpy(&workout.activities[i], &circuit, sizeof(Circuit));
    }

    Workout *w = malloc(sizeof(Workout));
    memcpy(w, &workout, sizeof(Workout));
    return w;
}

void initExerciseData(int week, CFBundleRef bundle) {
    restFormat = CFBundleCopyLocalizedString(bundle, CFSTR("exerciseTitleRest"), NULL, NULL);
    repsFormat = CFBundleCopyLocalizedString(bundle, CFSTR("exerciseTitleReps"), NULL, NULL);
    setsFormat = CFBundleCopyLocalizedString(bundle, CFSTR("exerciseHeader"), NULL, NULL);
    weightFormat = CFBundleCopyLocalizedString(bundle,
                                               CFSTR("exerciseTitleRepsWithWeight"), NULL, NULL);
    durationMinsFormat = CFBundleCopyLocalizedString(bundle, CFSTR("exerciseTitleDurationMinutes"),
                                                     NULL, NULL);
    durationSecsFormat = CFBundleCopyLocalizedString(bundle, CFSTR("exerciseTitleDurationSeconds"),
                                                     NULL, NULL);
    distanceFormat = CFBundleCopyLocalizedString(bundle,
                                                 CFSTR("exerciseTitleDistance"), NULL, NULL);
    roundsFormat = CFBundleCopyLocalizedString(bundle, CFSTR("circuitHeaderRounds"), NULL, NULL);
    amrapFormat = CFBundleCopyLocalizedString(bundle, CFSTR("circuitHeaderAMRAP"), NULL, NULL);
    weekInPlan = week;

    Class storeClass = objc_getClass("HKHealthStore");
    if (!((bool(*)(Class,SEL))objc_msgSend)(storeClass, sel_getUid("isHealthDataAvailable"))) {
        return;
    }

    id store = createNew(storeClass);
    id weightType = staticMethodWithString(objc_getClass("HKSampleType"),
                                           sel_getUid("quantityTypeForIdentifier:"),
                                           (CFStringRef) HKQuantityTypeIdentifierBodyMass);
    CFSetRef set = CFSetCreate(NULL, (const void *[]){weightType}, 1, NULL);

    (((void(*)(id,SEL,CFSetRef,CFSetRef,void(^)(bool,id)))objc_msgSend)
     (store, sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:"),
      NULL, set, ^(bool granted, id error1 _U_) {
        if (!granted) {
            releaseObj(store);
            return;
        }

        CFArrayRef arr = createSortDescriptors((CFStringRef)HKSampleSortIdentifierStartDate, false);
        id _req = allocClass(objc_getClass("HKSampleQuery"));
        id req = (((id(*)(id,SEL,id,id,unsigned long,CFArrayRef,void(^)(id,CFArrayRef,id)))objc_msgSend)
                  (_req, sel_getUid("initWithSampleType:predicate:limit:sortDescriptors:resultsHandler:"),
                   weightType, nil, 1, arr, ^(id query _U_, CFArrayRef data, id error2 _U_) {
            short weight = 145;
            if (data && CFArrayGetCount(data)) {
                id unit = staticMethod(objc_getClass("HKUnit"), sel_getUid("poundUnit"));
                id sample = (id) CFArrayGetValueAtIndex(data, 0);
                id quantity = getObject(sample, sel_getUid("quantity"));
                weight = (short) (((double(*)(id,SEL,id))objc_msgSend)
                                  (quantity, sel_getUid("doubleValueForUnit:"), unit));
            }
            bodyweight = weight;
            releaseObj(store);
        }));
        setObject(store, sel_getUid("executeQuery:"), req);
        CFRelease(arr);
        releaseObj(req);
    }));
    CFRelease(set);
}

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int newValue) { weekInPlan = newValue; }
#endif

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names) {
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef currWeek = getCurrentWeekForPlan(&info, plan);
    int index = 0;
    unsigned char type;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
        CFNumberGetValue(number, kCFNumberCharType, &type);
        if (type > 3) continue;

        CFArrayRef libArr = getLibraryArrayForType(&info, type);
        number = CFDictionaryGetValue(day, Keys.index);
        CFNumberGetValue(number, kCFNumberIntType, &index);
        CFStringRef str = CFDictionaryGetValue(CFArrayGetValueAtIndex(libArr, index), Keys.title);
        names[i] = CFStringCreateCopy(NULL, str);
    }
    CFRelease(info.root);
}

Workout *exerciseManager_getWeeklyWorkout(unsigned char plan, int index) {
    struct DictWrapper info;
    int arrayIdx;
    short sets, reps, weight;
    unsigned char type;
    createRootAndLibDict(&info);
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeekForPlan(&info, plan), index);

    CFNumberRef number = CFDictionaryGetValue(day, Keys.type);
    CFNumberGetValue(number, kCFNumberCharType, &type);

    CFArrayRef libArr = getLibraryArrayForType(&info, type);

    number = CFDictionaryGetValue(day, Keys.index);
    CFNumberGetValue(number, kCFNumberIntType, &arrayIdx);
    number = CFDictionaryGetValue(day, CFSTR("sets"));
    CFNumberGetValue(number, kCFNumberShortType, &sets);
    number = CFDictionaryGetValue(day, Keys.reps);
    CFNumberGetValue(number, kCFNumberShortType, &reps);
    number = CFDictionaryGetValue(day, CFSTR("weight"));
    CFNumberGetValue(number, kCFNumberShortType, &weight);

    Workout *w = buildWorkout(CFArrayGetValueAtIndex(libArr, arrayIdx), &(WorkoutParams){
        arrayIdx, sets, reps, weight, type, (unsigned char) index
    });
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
    Workout *w = buildWorkout(CFArrayGetValueAtIndex(libArr, params->index), params);
    CFRelease(info.root);
    return w;
}
