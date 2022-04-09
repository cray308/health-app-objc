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
    CFArrayRef lib;
};

static int weekInPlan;
static short bodyweight = 165;

static struct {
    CFStringRef const reps;
    CFStringRef const type;
    CFStringRef const index;
} const Keys = {CFSTR("R"), CFSTR("T"), CFSTR("I")};

static CFStringRef repsFormat;
static CFStringRef setsFormat;
static CFStringRef roundsFormat;
static CFStringRef restFormat;
static CFStringRef weightFormat;
static CFStringRef durationMinsFormat;
static CFStringRef durationSecsFormat;
static CFStringRef distanceFormat;
static CFStringRef amrapFormat;
static CFStringRef circuitProgressFmt;
static long setLoc;
static long roundLoc;

static void createRootAndLibDict(CFBundleRef bundle, struct DictWrapper *data) {
    CFURLRef url = CFBundleCopyResourceURL(bundle, CFSTR("WorkoutData"), CFSTR("plist"), NULL);
    CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, url);
    CFRelease(url);
    CFReadStreamOpen(stream);
    unsigned char *buf = malloc(9000);
    CFDataRef plistData = CFDataCreate(NULL, buf, CFReadStreamRead(stream, buf, 9000));
    free(buf);
    CFReadStreamClose(stream);
    CFRelease(stream);
    CFPropertyListFormat format = kCFPropertyListXMLFormat_v1_0;
    data->root = CFPropertyListCreateWithData(NULL, plistData, 0, &format, NULL);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("L"));
    CFRelease(plistData);
}

static CFArrayRef getCurrentWeekForPlan(struct DictWrapper *data, unsigned char plan) {
    CFArrayRef plans = CFDictionaryGetValue(data->root, CFSTR("P"));
    return CFArrayGetValueAtIndex(CFArrayGetValueAtIndex(plans, plan), weekInPlan);
}

static CFStringRef createTitle(CFBundleRef bundle, int type, int index) {
    CFStringRef key = formatStr(CFSTR("wkNames%d%02d"), type, index);
    CFStringRef title = localize(bundle, key);
    CFRelease(key);
    return title;
}

static Workout *buildWorkout(CFBundleRef bundle, CFArrayRef acts, WorkoutParams *params) {
    short weights[4];
    short customSets = 1, customReps = 0, customCircuitReps = 0;
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
        customSets = params->sets;
    } else if (params->type == WorkoutSE) {
        customCircuitReps = params->sets;
        customReps = params->reps;
    } else if (params->type == WorkoutEndurance) {
        customReps = params->reps * 60;
    }

    int nActivities = (int)CFArrayGetCount(acts);
    customAssert(nActivities > 0)

    Workout workout = {
        .activities = malloc((unsigned)nActivities * sizeof(Circuit)),
        .title = createTitle(bundle, params->type, params->index), .size = nActivities,
        .bodyweight = bodyweight, .type = params->type, .day = params->day, .testMax = testMax
    };
    workout.group = &workout.activities[0];

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(acts, i);
        CFMutableStringRef circuitHeader = CFStringCreateMutable(NULL, 40);
        CFStringRef separator = CFSTR("");
        CFRange circuitRange = {roundLoc, 1};
        short circuitReps = customCircuitReps, exerciseSets = customSets;
        unsigned char circuitType;

        CFNumberGetValue(CFDictionaryGetValue(act, Keys.type), kCFNumberCharType, &circuitType);
        if (!circuitReps)
            CFNumberGetValue(CFDictionaryGetValue(act, Keys.reps), kCFNumberShortType, &circuitReps);
#if TARGET_OS_SIMULATOR
        if (circuitType == CircuitAMRAP) circuitReps = nActivities > 1 ? 1 : 2;
#endif

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("E"));
        int nExercises = (int)CFArrayGetCount(foundExercises);

        if (params->type == WorkoutHIC && !circuitType && nExercises == 1) {
            exerciseSets = circuitReps;
            circuitReps = 1;
        }
        if (nActivities > 1) {
            CFStringRef circuitLoc = formatStr(circuitProgressFmt, i + 1, nActivities);
            CFStringAppend(circuitHeader, circuitLoc);
            CFRelease(circuitLoc);
            separator = CFSTR(" - ");
        }
        if (circuitType == CircuitAMRAP) {
            CFStringRef header = formatStr(amrapFormat, circuitReps);
            CFStringAppend(circuitHeader, separator);
            CFStringAppend(circuitHeader, header);
            CFRelease(header);
        } else if (circuitReps > 1) {
            CFStringRef header = formatStr(roundsFormat, 1, circuitReps);
            CFStringAppend(circuitHeader, separator);
            circuitRange.location += CFStringGetLength(circuitHeader);
            CFStringAppend(circuitHeader, header);
            CFRelease(header);
        }

        if (CFStringGetLength(circuitHeader)) {
            CFRetain(circuitHeader);
        } else {
            CFRelease(circuitHeader);
            circuitHeader = NULL;
        }

        Circuit circuit = {
            .exercises = malloc((unsigned)nExercises * sizeof(ExerciseEntry)),
            .headerStr = circuitHeader, .numberRange = circuitRange,
            .size = nExercises, .reps = circuitReps, .type = circuitType
        };
        customAssert(nExercises > 0)
        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            CFMutableStringRef exerciseHeader = NULL;
            short rest = 0, exerciseReps = customReps;
            unsigned char exerciseType;

            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.type), kCFNumberCharType, &exerciseType);
            if (!exerciseReps)
                CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.reps), kCFNumberShortType, &exerciseReps);
#if TARGET_OS_SIMULATOR
            if (exerciseType == ExerciseDuration) exerciseReps = workout.type == WorkoutHIC ? 15 : 120;
#endif

            if (exerciseSets > 1) {
                CFStringRef numberStr = formatStr(setsFormat, 1, exerciseSets);
                exerciseHeader = CFStringCreateMutableCopy(NULL, 16, numberStr);
                CFRelease(numberStr);
            }

            ExerciseEntry e = {
                .headerStr = exerciseHeader, .hRange = (CFRange){setLoc, 1}, .reps = exerciseReps,
                .sets = exerciseSets, .type = exerciseType
            };

            CFNumberGetValue(CFDictionaryGetValue(exDict, CFSTR("B")), kCFNumberShortType, &rest);
            if (rest)
                e.restStr = formatStr(restFormat, rest);

            int n;
            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.index), kCFNumberIntType, &n);
            CFStringRef nameKey = formatStr(CFSTR("exNames%02d"), n);
            CFStringRef name = localize(bundle, nameKey);
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
                titleStr = formatStr(distanceFormat, name, e.reps, (5 * e.reps) >> 2);
            }
            CFRelease(nameKey);
            CFRelease(name);

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
            if (data && CFArrayGetCount(data)) {
                id unit = clsF0(id, objc_getClass("HKUnit"), sel_getUid("poundUnit"));
                id quantity = msg0(id, (id)CFArrayGetValueAtIndex(data, 0), sel_getUid("quantity"));
                bodyweight = (short)msg1(double, id, quantity, sel_getUid("doubleValueForUnit:"), unit);
            }
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
    circuitProgressFmt = localize(bundle, CFSTR("circuitProgressHint"));
    weekInPlan = week;

    long *locs[] = {&setLoc, &roundLoc};
    CFStringRef strs[] = {setsFormat, roundsFormat};
    for (int i = 0; i < 2; ++i) {
        CFStringRef str = formatStr(strs[i], 1, 5);
        *(locs[i]) = CFStringFind(str, CFSTR("1"), 0).location;
        CFRelease(str);
    }

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{ getHealthData(); });
}

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int newValue) { weekInPlan = newValue; }
#endif

void setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names) {
    CFBundleRef bundle = CFBundleGetMainBundle();
    struct DictWrapper info;
    createRootAndLibDict(bundle, &info);
    CFArrayRef currWeek = getCurrentWeekForPlan(&info, plan);
    int index = 0;
    unsigned char type;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), kCFNumberCharType, &type);
        if (type > 3) continue;

        CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), kCFNumberIntType, &index);
        names[i] = createTitle(bundle, type, index);
    }
    CFRelease(info.root);
}

Workout *getWeeklyWorkout(unsigned char plan, int index) {
    CFBundleRef bundle = CFBundleGetMainBundle();
    struct DictWrapper info;
    int arrayIdx;
    short sets, reps, weight;
    unsigned char type;
    createRootAndLibDict(bundle, &info);
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeekForPlan(&info, plan), index);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), kCFNumberCharType, &type);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), kCFNumberIntType, &arrayIdx);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("S")), kCFNumberShortType, &sets);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.reps), kCFNumberShortType, &reps);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("W")), kCFNumberShortType, &weight);

    CFArrayRef libArr = CFArrayGetValueAtIndex(info.lib, type);
    Workout *w = buildWorkout(bundle, CFArrayGetValueAtIndex(libArr, arrayIdx),
                              &(WorkoutParams){arrayIdx, sets, reps, weight, type, (unsigned char)index});
    CFRelease(info.root);
    return w;
}

CFArrayRef createWorkoutNames(unsigned char type) {
    static const int counts[] = {2, 8, 1, 27};
    CFBundleRef bundle = CFBundleGetMainBundle();
    int len = counts[type];
    CFMutableArrayRef results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFStringRef t = createTitle(bundle, type, i);
        CFArrayAppendValue(results, t);
        CFRelease(t);
    }
    return results;
}

Workout *getWorkoutFromLibrary(WorkoutParams *params) {
    CFBundleRef bundle = CFBundleGetMainBundle();
    struct DictWrapper info;
    createRootAndLibDict(bundle, &info);
    CFArrayRef libArr = CFArrayGetValueAtIndex(info.lib, params->type);
    Workout *w = buildWorkout(bundle, CFArrayGetValueAtIndex(libArr, params->index), params);
    CFRelease(info.root);
    return w;
}
