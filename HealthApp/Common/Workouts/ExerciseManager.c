#include "ExerciseManager.h"
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFSet.h>
#include <dispatch/queue.h>
#include "AppDelegate.h"
#include "CocoaHelpers.h"

extern CFStringRef HKQuantityTypeIdentifierBodyMass;
extern CFStringRef HKSampleSortIdentifierStartDate;

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
    CFDataRef plistData = CFDataCreate(NULL, buf, CFReadStreamRead(stream, buf, 12000));
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
    return CFArrayGetValueAtIndex(CFDictionaryGetValue(plans, planKeys[plan]), weekInPlan);
}

static Workout *buildWorkout(CFDictionaryRef dict, WorkoutParams *params) {
    short weights[4];
    short exerciseSets = 1, customReps = 0, customCircuitReps = 0;
    bool testMax = false;
    if (params->type == WorkoutStrength) {
        const short *lifts = getUserInfo()->liftMaxes;
        float multiplier = params->weight / 100.f;

        weights[0] = (short)(multiplier * lifts[0]);
        if (params->index <= 1) {
            weights[1] = (short)(multiplier * lifts[LiftBench]);
            if (params->index == 0) {
                short weight = (short)((lifts[LiftPullup] + bodyweight) * multiplier) - bodyweight;
                if (weight < 0)
                    weight = 0;
                weights[2] = weight;
            } else {
                weights[2] = (short)(multiplier * lifts[LiftDeadlift]);
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
    int nActivities = (int)CFArrayGetCount(foundActivities);
    customAssert(nActivities > 0)

    Workout workout = {
        .activities = malloc((unsigned)nActivities * sizeof(Circuit)),
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

        CFNumberGetValue(CFDictionaryGetValue(act, Keys.type), kCFNumberCharType, &circuitType);
        if (!circuitReps) {
            CFNumberGetValue(CFDictionaryGetValue(act, Keys.reps), kCFNumberShortType, &circuitReps);
        }
#if TARGET_OS_SIMULATOR
        if (circuitType == CircuitAMRAP)
            circuitReps = nActivities > 1 ? 1 : 2;
#endif

        if (circuitType == CircuitAMRAP) {
            circuitHeader = formatStr(amrapFormat, circuitReps);
        } else if (circuitReps > 1) {
            circuitHeader = formatStr(roundsFormat, 1, circuitReps);
            circuitRange = CFStringFind(circuitHeader, CFSTR("1"), 0);
        }

        if (circuitHeader) {
            circuitHeaderMutable = CFStringCreateMutableCopy(NULL, 16, circuitHeader);
            CFRelease(circuitHeader);
        }

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("exercises"));
        int nExercises = (int)CFArrayGetCount(foundExercises);
        customAssert(nExercises > 0)
        Circuit circuit = {
            .exercises = malloc((unsigned)nExercises * sizeof(ExerciseEntry)),
            .headerStr = circuitHeaderMutable, .numberRange = circuitRange,
            .size = nExercises, .reps = circuitReps, .type = circuitType
        };

        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            CFMutableStringRef exerciseHeader = NULL;
            CFRange exerciseHeaderRange = {0, 0};
            short rest = 0, exerciseReps = customReps;
            unsigned char exerciseType;

            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.type), kCFNumberCharType, &exerciseType);
            if (!exerciseReps) {
                CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.reps), kCFNumberShortType, &exerciseReps);
            }
#if TARGET_OS_SIMULATOR
            if (workout.type == WorkoutHIC && exerciseType == ExerciseDuration && exerciseReps > 30)
                exerciseReps = 30;
#endif

            if (exerciseSets > 1) {
                CFStringRef numberStr = formatStr(setsFormat, 1, exerciseSets);
                exerciseHeaderRange = CFStringFind(numberStr, CFSTR("1"), 0);
                exerciseHeader = CFStringCreateMutableCopy(NULL, 16, numberStr);
                CFRelease(numberStr);
            }

            ExerciseEntry e = {
                .headerStr = exerciseHeader, .hRange = exerciseHeaderRange, .reps = exerciseReps,
                .sets = exerciseSets, .type = exerciseType
            };

            CFNumberGetValue(CFDictionaryGetValue(exDict, CFSTR("rest")), kCFNumberShortType, &rest);
            if (rest)
                e.restStr = formatStr(restFormat, rest);

            CFStringRef name = CFDictionaryGetValue(exDict, CFSTR("name"));
            CFStringRef titleStr;

            if (e.type == ExerciseReps) {
                if (!workout.type) {
                    titleStr = formatStr(weightFormat, name, e.reps, weights[j]);
                } else {
                    titleStr = formatStr(repsFormat, name, e.reps);
                }
            } else if (e.type == ExerciseDuration) {
                if (e.reps > 120) {
                    titleStr = formatStr(durationMinsFormat, name, e.reps / 60.f);
                } else {
                    titleStr = formatStr(durationSecsFormat, name, e.reps);
                }
            } else {
                titleStr = formatStr(distanceFormat, e.reps, (5 * e.reps) >> 2);
            }

            e.titleStr = CFStringCreateMutableCopy(NULL, 64, titleStr);
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

static void getHealthData(void) {
    Class storeClass = objc_getClass("HKHealthStore");
    if (!clsF0(bool, storeClass, sel_getUid("isHealthDataAvailable"))) return;

    id store = Sels.new(storeClass, Sels.nw);
    id weightType = clsF1(id, CFStringRef, objc_getClass("HKSampleType"),
                          sel_getUid("quantityTypeForIdentifier:"), HKQuantityTypeIdentifierBodyMass);
    CFSetRef set = CFSetCreate(NULL, (const void *[]){weightType}, 1, NULL);

    msg3(void, CFSetRef, CFSetRef, void(^)(bool,id), store,
         sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:"),
         NULL, set, ^(bool granted, id er _U_) {
        if (!granted) {
            Sels.objRel(store, Sels.rel);
            return;
        }

        CFArrayRef arr = createSortDescriptors(HKSampleSortIdentifierStartDate, false);
        id _req = Sels.alloc(objc_getClass("HKSampleQuery"), Sels.alo);
        SEL qInit = sel_getUid("initWithSampleType:predicate:limit:sortDescriptors:resultsHandler:");
        id req = (((id(*)(id,SEL,id,id,unsigned long,CFArrayRef,void(^)(id,CFArrayRef,id)))objc_msgSend)
                  (_req, qInit, weightType, nil, 1, arr, ^(id q _U_, CFArrayRef data, id err2 _U_) {
            short weight = 145;
            if (data && CFArrayGetCount(data)) {
                id unit = clsF0(id, objc_getClass("HKUnit"), sel_getUid("poundUnit"));
                id quantity = msg0(id, (id)CFArrayGetValueAtIndex(data, 0), sel_getUid("quantity"));
                weight = (short)msg1(double, id, quantity, sel_getUid("doubleValueForUnit:"), unit);
            }
            bodyweight = weight;
            Sels.objRel(store, Sels.rel);
        }));
        msg1(void, id, store, sel_getUid("executeQuery:"), req);
        CFRelease(arr);
        Sels.objRel(req, Sels.rel);
    });
    CFRelease(set);
}

void initExerciseData(int week, CFBundleRef bundle) {
    restFormat = localize(bundle, CFSTR("exerciseTitleRest"));
    repsFormat = localize(bundle, CFSTR("exerciseTitleReps"));
    setsFormat = localize(bundle, CFSTR("exerciseHeader"));
    weightFormat = localize(bundle, CFSTR("exerciseTitleRepsWithWeight"));
    durationMinsFormat = localize(bundle, CFSTR("exerciseTitleDurationMinutes"));
    durationSecsFormat = localize(bundle, CFSTR("exerciseTitleDurationSeconds"));
    distanceFormat = localize(bundle, CFSTR("exerciseTitleDistance"));
    roundsFormat = localize(bundle, CFSTR("circuitHeaderRounds"));
    amrapFormat = localize(bundle, CFSTR("circuitHeaderAMRAP"));
    weekInPlan = week;

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{ getHealthData(); });
}

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int newValue) { weekInPlan = newValue; }
#endif

void setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names) {
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef currWeek = getCurrentWeekForPlan(&info, plan);
    int index = 0;
    unsigned char type;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), kCFNumberCharType, &type);
        if (type > 3) continue;

        CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), kCFNumberIntType, &index);
        CFArrayRef libArr = getLibraryArrayForType(&info, type);
        names[i] = CFRetain(CFDictionaryGetValue(CFArrayGetValueAtIndex(libArr, index), Keys.title));
    }
    CFRelease(info.root);
}

Workout *getWeeklyWorkout(unsigned char plan, int index) {
    struct DictWrapper info;
    int arrayIdx;
    short sets, reps, weight;
    unsigned char type;
    createRootAndLibDict(&info);
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeekForPlan(&info, plan), index);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), kCFNumberCharType, &type);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), kCFNumberIntType, &arrayIdx);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("sets")), kCFNumberShortType, &sets);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.reps), kCFNumberShortType, &reps);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("weight")), kCFNumberShortType, &weight);

    Workout *w = buildWorkout(CFArrayGetValueAtIndex(getLibraryArrayForType(&info, type), arrayIdx),
                              &(WorkoutParams){arrayIdx, sets, reps, weight, type, (unsigned char)index});
    CFRelease(info.root);
    return w;
}

CFArrayRef createWorkoutNames(unsigned char type) {
    struct DictWrapper info;
    createRootAndLibDict(&info);

    CFArrayRef libArr = getLibraryArrayForType(&info, type);
    int len = (int)CFArrayGetCount(libArr);
    if (type == WorkoutStrength)
        len = 2;

    CFMutableArrayRef results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFArrayAppendValue(results, CFDictionaryGetValue(CFArrayGetValueAtIndex(libArr, i), Keys.title));
    }
    CFRelease(info.root);
    return results;
}

Workout *getWorkoutFromLibrary(WorkoutParams *params) {
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef libArr = getLibraryArrayForType(&info, params->type);
    Workout *w = buildWorkout(CFArrayGetValueAtIndex(libArr, params->index), params);
    CFRelease(info.root);
    return w;
}
