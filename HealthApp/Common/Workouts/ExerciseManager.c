#include "ExerciseManager.h"
#include "CocoaHelpers.h"

extern CFStringRef HKQuantityTypeIdentifierBodyMass;
extern CFStringRef HKSampleSortIdentifierStartDate;

typedef struct {
    CFDictionaryRef root;
    CFArrayRef lib;
} WorkoutPlist;

static int weekInPlan;
static int bodyweight = 165;

static struct {
    CFStringRef reps;
    CFStringRef type;
    CFStringRef index;
} const EMKeys = {CFSTR("R"), CFSTR("T"), CFSTR("I")};

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
    id unit = msgV(objSig(id, CFStringRef), alloc(objc_getClass("NSUnit")),
                   sel_getUid("initWithSymbol:"), key);
    id formatter = new(objc_getClass("NSMeasurementFormatter"));
    CFStringRef _unitStr = msgV(objSig(CFStringRef, id), formatter, sel_getUid("stringFromUnit:"), unit);
    weightUnit = CFStringCreateCopy(NULL, _unitStr);
    releaseObject(unit);
    releaseObject(formatter);

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{ getHealthData(); });
}

void getHealthData(void) {
    Class storeClass = objc_getClass("HKHealthStore");
    if (!msgV(clsSig(bool), storeClass, sel_getUid("isHealthDataAvailable"))) return;

    id store = new(storeClass);
    id weightType = msgV(clsSig(id, CFStringRef), objc_getClass("HKSampleType"),
                          sel_getUid("quantityTypeForIdentifier:"), HKQuantityTypeIdentifierBodyMass);
    CFSetRef set = CFSetCreate(NULL, (const void *[]){weightType}, 1, NULL);

    SEL auth = sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:");
    ((objSig(void, CFSetRef, CFSetRef, void(^)(bool, id))objc_msgSend)
     (store, auth, NULL, set, ^(bool granted, id er _U_) {
        if (!granted) {
            releaseObject(store);
            return;
        }

        CFArrayRef arr = createSortDescriptors(HKSampleSortIdentifierStartDate, false);
        id _req = alloc(objc_getClass("HKSampleQuery"));
        SEL qInit = sel_getUid("initWithSampleType:predicate:limit:sortDescriptors:resultsHandler:");
        id req = ((objSig(id, id, id, u_long, CFArrayRef, void(^)(id, CFArrayRef, id))objc_msgSend)
                  (_req, qInit, weightType, nil, 1, arr, ^(id q _U_, CFArrayRef data, id err2 _U_) {
            if (data && CFArrayGetCount(data)) {
                id unit = msgV(clsSig(id), objc_getClass("HKUnit"), sel_getUid("poundUnit"));
                id qty = msgV(objSig(id), (id)CFArrayGetValueAtIndex(data, 0), sel_getUid("quantity"));
                bodyweight = (int)msgV(objSig(double, id), qty, sel_getUid("doubleValueForUnit:"), unit);
            }
            releaseObject(store);
        }));
        msgV(objSig(void, id), store, sel_getUid("executeQuery:"), req);
        CFRelease(arr);
        releaseObject(req);
    }));
    CFRelease(set);
}

static void createPlist(WorkoutPlist *data) {
    CFURLRef url = CFBundleCopyResourceURL(
      CFBundleGetMainBundle(), CFSTR("WorkoutData"), CFSTR("plist"), NULL);
    CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, url);
    CFRelease(url);
    CFReadStreamOpen(stream);
    uint8_t *buf = malloc(9000);
    CFDataRef plistData = CFDataCreate(NULL, buf, CFReadStreamRead(stream, buf, 9000));
    free(buf);
    CFReadStreamClose(stream);
    CFRelease(stream);
    data->root = CFPropertyListCreateWithData(NULL, plistData, 0, NULL, NULL);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("L"));
    CFRelease(plistData);
}

static CFArrayRef getCurrentWeek(WorkoutPlist const *data, uint8_t plan) {
    CFArrayRef plans = CFDictionaryGetValue(data->root, CFSTR("P"));
    return CFArrayGetValueAtIndex(CFArrayGetValueAtIndex(plans, plan), weekInPlan);
}

static CFStringRef createTitle(int type, int index) {
    CFStringRef key = createWorkoutTitleKey(type, index);
    CFStringRef title = localize(key);
    CFRelease(key);
    return title;
}

static Workout *buildWorkout(WorkoutPlist *data, WorkoutParams const *params, int const *lifts) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    float weights[4] = {[3] = 0};
    short customSets = 1, customReps = 0, customCircuitReps = 0;
    bool testMax = false;
    if (params->type == WorkoutStrength) {
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
        if (isMetric(locale)) {
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

    CFArrayRef lib = CFArrayGetValueAtIndex(data->lib, params->type);
    CFArrayRef activities = CFArrayGetValueAtIndex(lib, params->index);
    int nActivities = (int)CFArrayGetCount(activities);
    customAssert(nActivities > 0)

    Workout *workout = calloc(1, sizeof(Workout));
    workout->size = nActivities;
    workout->circuits = calloc((unsigned)nActivities, sizeof(Circuit));
    workout->nameIdx = params->index;
    workout->bodyweight = bodyweight;
    workout->type = params->type;
    workout->day = params->day;
    workout->testMax = testMax;
    workout->group = &workout->circuits[0];
    bool multiple = nActivities > 1;
    CFStringRef one = getOneStr(locale);

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(activities, i);
        Circuit *c = &workout->circuits[i];
        CFStringRef circuitHeader = NULL;
        short exerciseSets = customSets;

        getDictValue(act, EMKeys.type, kCFNumberCharType, &c->type);
        c->reps = customCircuitReps;
        if (!c->reps) getDictValue(act, EMKeys.reps, kCFNumberShortType, &c->reps);
#if TARGET_OS_SIMULATOR
        if (c->type == CircuitAMRAP) c->reps = nActivities > 1 ? 1 : 2;
#endif

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("E"));
        int nExercises = (int)CFArrayGetCount(foundExercises);
        customAssert(nExercises > 0)

        if (params->type == WorkoutHIC && !c->type && nExercises == 1) {
            exerciseSets = c->reps;
            c->reps = 1;
        }

        c->exercises = calloc((unsigned)nExercises, sizeof(Exercise));
        c->size = nExercises;

        if (c->type == CircuitAMRAP) {
            if (multiple) {
                circuitHeader = formatStr(locale, amrapLoc, i + 1, nActivities, c->reps);
            } else {
                circuitHeader = formatStr(locale, amrapFormat, c->reps);
            }
        } else if (c->reps > 1) {
            if (multiple) {
                circuitHeader = formatStr(locale, roundsLoc, i + 1, nActivities, 1, c->reps);
            } else {
                circuitHeader = formatStr(locale, roundsFormat, 1, c->reps);
            }
            c->range = findNumber(circuitHeader, locale, one, rounds1);
        } else if (multiple) {
            circuitHeader = formatStr(locale, circuitProgressFmt, i + 1, nActivities);
        }

        if (circuitHeader) {
            c->header = CFStringCreateMutableCopy(NULL, 80, circuitHeader);
            CFRetain(c->header);
            CFRelease(circuitHeader);
        }

        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            Exercise *e = &c->exercises[j];

            getDictValue(exDict, EMKeys.type, kCFNumberCharType, &e->type);
            e->sets = exerciseSets;
            e->reps = customReps;
            if (!e->reps) getDictValue(exDict, EMKeys.reps, kCFNumberShortType, &e->reps);
#if TARGET_OS_SIMULATOR
            if (e->type == ExerciseDuration) e->reps = workout->type == WorkoutHIC ? 15 : 120;
#endif

            if (exerciseSets > 1) {
                CFStringRef header = formatStr(locale, setsFormat, 1, exerciseSets);
                e->headerRange = findNumber(header, locale, one, sets1);
                e->header = CFStringCreateMutableCopy(NULL, 32, header);
                CFRetain(e->header);
                CFRelease(header);
            }

            int rest = 0, n;
            getDictValue(exDict, CFSTR("B"), kCFNumberIntType, &rest);
            if (rest) e->rest = formatStr(locale, restFormat, rest);

            getDictValue(exDict, EMKeys.index, kCFNumberIntType, &n);
            CFStringRef nameKey = formatStr(NULL, CFSTR("exNames%02d"), n);
            CFStringRef name = localize(nameKey);
            CFStringRef titleStr;

            if (e->type == ExerciseReps) {
                if (!workout->type) {
                    titleStr = formatStr(locale, weightFormat, name, e->reps, weights[j], weightUnit);
                } else {
                    titleStr = formatStr(locale, repsFormat, name, e->reps);
                }
            } else if (e->type == ExerciseDuration) {
                if (e->reps > 120) {
                    titleStr = formatStr(locale, durationMinsFormat, name, e->reps / 60.f);
                } else {
                    titleStr = formatStr(locale, durationSecsFormat, name, e->reps);
                }
            } else {
                titleStr = formatStr(locale, distanceFormat, name, e->reps, (5 * e->reps) >> 2);
            }
            e->title = CFStringCreateMutableCopy(NULL, 64, titleStr);
            CFRetain(e->title);
            CFRelease(nameKey);
            CFRelease(name);
            CFRelease(titleStr);
        }

        if (c->type == CircuitDecrement) {
            c->completedReps = c->exercises[0].reps;
            CFStringRef ten = formatStr(locale, CFSTR("%d"), 10);
            Exercise *end = &c->exercises[c->size];
            for (Exercise *e = c->exercises; e < end; ++e) {
                if (e->type == ExerciseReps) e->titleRange = findNumber(e->title, locale, ten, NULL);
            }
            CFRelease(ten);
        }
    }
    CFRelease(one);
    CFRelease(locale);
    CFRelease(data->root);
    return workout;
}

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int newValue) { weekInPlan = newValue; }
#endif

void getWeeklyWorkoutNames(CFStringRef *names, uint8_t plan) {
    WorkoutPlist plist;
    createPlist(&plist);
    CFArrayRef currWeek = getCurrentWeek(&plist, plan);
    int index = 0;
    uint8_t type;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        getDictValue(day, EMKeys.type, kCFNumberCharType, &type);
        if (type > 3) continue;

        getDictValue(day, EMKeys.index, kCFNumberIntType, &index);
        names[i] = createTitle(type, index);
    }
    CFRelease(plist.root);
}

Workout *getWeeklyWorkout(int index, uint8_t plan, int const *lifts) {
    WorkoutPlist plist;
    createPlist(&plist);
    WorkoutParams params = {.day = (uint8_t)index};
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeek(&plist, plan), index);
    getDictValue(day, EMKeys.type, kCFNumberCharType, &params.type);
    getDictValue(day, EMKeys.index, kCFNumberIntType, &params.index);
    getDictValue(day, CFSTR("S"), kCFNumberShortType, &params.sets);
    getDictValue(day, EMKeys.reps, kCFNumberShortType, &params.reps);
    getDictValue(day, CFSTR("W"), kCFNumberShortType, &params.weight);
    return buildWorkout(&plist, &params, lifts);
}

CFArrayRef createWorkoutNames(uint8_t type) {
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

Workout *getWorkoutFromLibrary(WorkoutParams const *params, int const *lifts) {
    WorkoutPlist plist;
    createPlist(&plist);
    return buildWorkout(&plist, params, lifts);
}
