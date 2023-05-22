#include "ExerciseManager.h"
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
static CFStringRef roundsLoc;
static CFStringRef amrapLoc;
static CFStringRef weightUnit;
static CFStringRef sets1;
static CFStringRef rounds1;

static void getHealthData(void);

void initExerciseData(int week) {
    restFormat = localize(CFSTR("exerciseTitleRest"));
    repsFormat = localize(CFSTR("exerciseTitleReps"));
    setsFormat = localize(CFSTR("exerciseHeader"));
    weightFormat = localize(CFSTR("exerciseTitleRepsWithWeight"));
    durationMinsFormat = localize(CFSTR("exerciseTitleDurationMinutes"));
    durationSecsFormat = localize(CFSTR("exerciseTitleDurationSeconds"));
    distanceFormat = localize(CFSTR("exerciseTitleDistance"));
    roundsFormat = localize(CFSTR("circuitHeaderRounds"));
    amrapFormat = localize(CFSTR("circuitHeaderAMRAP"));
    circuitProgressFmt = localize(CFSTR("circuitProgressHint"));
    roundsLoc = localize(CFSTR("circuitHeaderLocAndRounds"));
    amrapLoc = localize(CFSTR("circuitHeaderLocAndAMRAP"));
    weekInPlan = week;

    CFStringRef *refs[] = {&sets1, &rounds1};
    CFStringRef arr[] = {localize(CFSTR("sets1")), localize(CFSTR("rounds1"))};
    for (int i = 0; i < 2; ++i) {
        CFMutableStringRef m = CFStringCreateMutableCopy(NULL, CFStringGetLength(arr[i]) + 2, arr[i]);
        if (CFStringHasPrefix(m, CFSTR("\U0000202e")) || CFStringHasPrefix(m, CFSTR("\U0000202f")))
            CFStringDelete(m, (CFRange){0, 1});
        if (CFStringHasSuffix(m, CFSTR("\U0000202c")))
            CFStringDelete(m, (CFRange){CFStringGetLength(m) - 1, 1});
        *refs[i] = CFStringCreateCopy(NULL, m);
        CFRelease(arr[i]);
        CFRelease(m);
    }

    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef key;
    if (isMetric(l)) {
        key = CFSTR("kg");
    } else {
        key = CFSTR("lb");
    }
    CFRelease(l);
    id unit = msg1(id, CFStringRef, Sels.alloc(objc_getClass("NSUnit"), Sels.alo),
                   sel_getUid("initWithSymbol:"), key);
    id formatter = Sels.new(objc_getClass("NSMeasurementFormatter"), Sels.nw);
    CFStringRef _unitStr = msg1(CFStringRef, id, formatter, sel_getUid("stringFromUnit:"), unit);
    weightUnit = CFStringCreateCopy(NULL, _unitStr);
    Sels.objRel(unit, Sels.rel);
    Sels.objRel(formatter, Sels.rel);

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{ getHealthData(); });
}

void getHealthData(void) {
    Class storeClass = objc_getClass("HKHealthStore");
    if (!clsF0(bool, storeClass, sel_getUid("isHealthDataAvailable"))) return;

    id store = Sels.new(storeClass, Sels.nw);
    id weightType = clsF1(id, CFStringRef, objc_getClass("HKSampleType"),
                          sel_getUid("quantityTypeForIdentifier:"), HKQuantityTypeIdentifierBodyMass);
    CFSetRef set = CFSetCreate(NULL, (const void *[]){weightType}, 1, NULL);

    SEL auth = sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:");
    msg3(void, CFSetRef, CFSetRef, void(^)(bool,id), store, auth, NULL, set, ^(bool granted, id er _U_) {
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
                id qty = msg0(id, (id)CFArrayGetValueAtIndex(data, 0), sel_getUid("quantity"));
                bodyweight = (short)msg1(double, id, qty, sel_getUid("doubleValueForUnit:"), unit);
            }
            Sels.objRel(store, Sels.rel);
        }));
        msg1(void, id, store, sel_getUid("executeQuery:"), req);
        CFRelease(arr);
        Sels.objRel(req, Sels.rel);
    });
    CFRelease(set);
}

static void createRootAndLibDict(struct DictWrapper *data) {
    CFURLRef url = CFBundleCopyResourceURL(
      CFBundleGetMainBundle(), CFSTR("WorkoutData"), CFSTR("plist"), NULL);
    CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, url);
    CFRelease(url);
    CFReadStreamOpen(stream);
    unsigned char *buf = malloc(9000);
    CFDataRef plistData = CFDataCreate(NULL, buf, CFReadStreamRead(stream, buf, 9000));
    free(buf);
    CFReadStreamClose(stream);
    CFRelease(stream);
    data->root = CFPropertyListCreateWithData(NULL, plistData, 0, NULL, NULL);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("L"));
    CFRelease(plistData);
}

static CFArrayRef getCurrentWeekForPlan(struct DictWrapper *data, unsigned char plan) {
    CFArrayRef plans = CFDictionaryGetValue(data->root, CFSTR("P"));
    return CFArrayGetValueAtIndex(CFArrayGetValueAtIndex(plans, plan), weekInPlan);
}

static CFStringRef createTitle(int type, int index) {
    CFStringRef key = formatStr(NULL, CFSTR("wkNames%d%02d"), type, index);
    CFStringRef title = localize(key);
    CFRelease(key);
    return title;
}

static Workout *buildWorkout(CFArrayRef acts, WorkoutParams *params) {
    CFLocaleRef l = CFLocaleCopyCurrent();
    float weights[4] = {[3] = 0};
    short customSets = 1, customReps = 0, customCircuitReps = 0;
    bool testMax = false;
    if (params->type == WorkoutStrength) {
        const short *lifts = getUserInfo()->liftMaxes;
        float multiplier = params->weight / 100.f;

        weights[0] = lifts[0] * multiplier;
        if (params->index <= 1) {
            weights[1] = lifts[LiftBench] * multiplier;
            if (params->index == 0) {
                int weight = (int)((lifts[LiftPullup] + bodyweight) * multiplier) - bodyweight;
                weights[2] = max(weight, 0);
            } else {
                weights[2] = lifts[LiftDeadlift] * multiplier;
            }
        } else {
            for (int i = 1; i < 4; ++i) weights[i] = lifts[i];
            testMax = true;
        }
        if (isMetric(l)) {
            for (int i = 0; i < 4; ++i) weights[i] *= ToKg;
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
        .title = createTitle(params->type, params->index), .size = nActivities,
        .bodyweight = bodyweight, .type = params->type, .day = params->day, .testMax = testMax
    };
    workout.group = &workout.activities[0];
    bool multiple = nActivities > 1;
    CFStringRef one = formatStr(l, CFSTR("%d"), 1);

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(acts, i);
        CFStringRef circuitHeader = NULL;
        short circuitReps = customCircuitReps, exerciseSets = customSets;
        unsigned char circuitType;

        CFNumberGetValue(CFDictionaryGetValue(act, Keys.type), 7, &circuitType);
        if (!circuitReps) CFNumberGetValue(CFDictionaryGetValue(act, Keys.reps), 8, &circuitReps);
#if TARGET_OS_SIMULATOR
        if (circuitType == CircuitAMRAP) circuitReps = nActivities > 1 ? 1 : 2;
#endif

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("E"));
        int nExercises = (int)CFArrayGetCount(foundExercises);
        customAssert(nExercises > 0)

        if (params->type == WorkoutHIC && !circuitType && nExercises == 1) {
            exerciseSets = circuitReps;
            circuitReps = 1;
        }

        Circuit circuit = {
            .exercises = malloc((unsigned)nExercises * sizeof(ExerciseEntry)),
            .size = nExercises, .reps = circuitReps, .type = circuitType
        };

        if (circuitType == CircuitAMRAP) {
            if (multiple) {
                circuitHeader = formatStr(l, amrapLoc, i + 1, nActivities, circuitReps);
            } else {
                circuitHeader = formatStr(l, amrapFormat, circuitReps);
            }
        } else if (circuitReps > 1) {
            CFRange subrange, numrange;
            if (multiple) {
                circuitHeader = formatStr(l, roundsLoc, i + 1, nActivities, 1, circuitReps);
            } else {
                circuitHeader = formatStr(l, roundsFormat, 1, circuitReps);
            }
            subrange = CFStringFind(circuitHeader, rounds1, 0);
            CFStringRef subhead = CFStringCreateWithSubstring(NULL, circuitHeader, subrange);
            CFStringFindWithOptionsAndLocale(
              subhead, one, (CFRange){0, CFStringGetLength(subhead)}, 0, l, &numrange);
            CFRelease(subhead);
            circuit.range.location = subrange.location + numrange.location;
            circuit.range.length = numrange.length;
        } else if (multiple) {
            circuitHeader = formatStr(l, circuitProgressFmt, i + 1, nActivities);
        }

        if (circuitHeader) {
            circuit.header = CFStringCreateMutableCopy(NULL, 80, circuitHeader);
            CFRetain(circuit.header);
            CFRelease(circuitHeader);
        }

        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            short rest = 0, eReps = customReps;
            unsigned char eType;

            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.type), 7, &eType);
            if (!eReps) CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.reps), 8, &eReps);
#if TARGET_OS_SIMULATOR
            if (eType == ExerciseDuration) eReps = workout.type == WorkoutHIC ? 15 : 120;
#endif
            ExerciseEntry e = {.reps = eReps, .sets = exerciseSets, .type = eType};

            if (exerciseSets > 1) {
                CFRange subrange, numrange;
                CFStringRef header = formatStr(l, setsFormat, 1, exerciseSets);
                subrange = CFStringFind(header, sets1, 0);
                CFStringRef subhead = CFStringCreateWithSubstring(NULL, header, subrange);
                CFStringFindWithOptionsAndLocale(
                  subhead, one, (CFRange){0, CFStringGetLength(subhead)}, 0, l, &numrange);
                e.header = CFStringCreateMutableCopy(NULL, 32, header);
                CFRelease(subhead);
                CFRelease(header);
                e.hRange.location = subrange.location + numrange.location;
                e.hRange.length = numrange.length;
            }

            CFNumberGetValue(CFDictionaryGetValue(exDict, CFSTR("B")), 8, &rest);
            if (rest) e.rest = formatStr(l, restFormat, rest);

            int n;
            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.index), 9, &n);
            CFStringRef nameKey = formatStr(NULL, CFSTR("exNames%02d"), n);
            CFStringRef name = localize(nameKey);
            CFStringRef titleStr;

            if (e.type == ExerciseReps) {
                if (!workout.type) {
                    titleStr = formatStr(l, weightFormat, name, e.reps, weights[j], weightUnit);
                } else {
                    titleStr = formatStr(l, repsFormat, name, e.reps);
                }
            } else if (e.type == ExerciseDuration) {
                if (e.reps > 120) {
                    titleStr = formatStr(l, durationMinsFormat, name, e.reps / 60.f);
                } else {
                    titleStr = formatStr(l, durationSecsFormat, name, e.reps);
                }
            } else {
                titleStr = formatStr(l, distanceFormat, name, e.reps, (5 * e.reps) >> 2);
            }
            CFRelease(nameKey);
            CFRelease(name);

            e.title = CFStringCreateMutableCopy(NULL, 64, titleStr);
            CFRetain(e.title);
            CFRelease(titleStr);
            if (circuit.type == CircuitDecrement && e.type == ExerciseReps) {
                CFStringRef ten = formatStr(l, CFSTR("%d"), 10);
                CFStringFindWithOptionsAndLocale(
                  e.title, ten, (CFRange){0, CFStringGetLength(e.title)}, 0, l, &e.tRange);
                CFRelease(ten);
            }

            memcpy(&circuit.exercises[j], &e, sizeof(ExerciseEntry));
        }

        if (circuit.type == CircuitDecrement) circuit.completedReps = circuit.exercises[0].reps;
        memcpy(&workout.activities[i], &circuit, sizeof(Circuit));
    }
    CFRelease(one);
    CFRelease(l);

    Workout *w = malloc(sizeof(Workout));
    memcpy(w, &workout, sizeof(Workout));
    return w;
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
        CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), 7, &type);
        if (type > 3) continue;

        CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), 9, &index);
        names[i] = createTitle(type, index);
    }
    CFRelease(info.root);
}

Workout *getWeeklyWorkout(unsigned char plan, int i) {
    struct DictWrapper info;
    int arrayIdx;
    short sets, reps, weight;
    unsigned char type;
    createRootAndLibDict(&info);
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeekForPlan(&info, plan), i);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), 7, &type);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), 9, &arrayIdx);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("S")), 8, &sets);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.reps), 8, &reps);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("W")), 8, &weight);

    CFArrayRef libArr = CFArrayGetValueAtIndex(info.lib, type);
    Workout *w = buildWorkout(CFArrayGetValueAtIndex(libArr, arrayIdx),
                              &(WorkoutParams){arrayIdx, sets, reps, weight, type, (unsigned char)i});
    CFRelease(info.root);
    return w;
}

CFArrayRef createWorkoutNames(unsigned char type) {
    static const int counts[] = {2, 8, 1, 27};
    int len = counts[type];
    CFMutableArrayRef results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFStringRef t = createTitle(type, i);
        CFArrayAppendValue(results, t);
        CFRelease(t);
    }
    return results;
}

Workout *getWorkoutFromLibrary(WorkoutParams *params) {
    struct DictWrapper info;
    createRootAndLibDict(&info);
    CFArrayRef libArr = CFArrayGetValueAtIndex(info.lib, params->type);
    Workout *w = buildWorkout(CFArrayGetValueAtIndex(libArr, params->index), params);
    CFRelease(info.root);
    return w;
}
