#include "ExerciseManager.h"
#include "HealthDataManager.h"
#include "StringUtils.h"

short bodyweight = 165;

typedef struct {
    CFDictionaryRef root;
    CFArrayRef lib;
} WorkoutPlist;

static int weekInPlan;
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
static struct {
    CFStringRef const reps;
    CFStringRef const type;
    CFStringRef const index;
} const Keys = {CFSTR("R"), CFSTR("T"), CFSTR("I")};

void initExerciseData(int week, float *multiplier) {
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
    CFLocaleRef locale = CFLocaleCopyCurrent();
    oneStr = formatStr(locale, CFSTR("%d"), 1);
    CFRelease(locale);
    weekInPlan = week;

    CFStringRef *refs[] = {&sets1, &rounds1};
    CFStringRef arr[] = {localize(CFSTR("sets1")), localize(CFSTR("rounds1"))};
    for (int i = 0; i < 2; ++i) {
        CFMutableStringRef s = CFStringCreateMutableCopy(NULL, CFStringGetLength(arr[i]) + 2, arr[i]);
        if (CFStringHasPrefix(s, CFSTR("\U0000202e")) || CFStringHasPrefix(s, CFSTR("\U0000202f")))
            CFStringDelete(s, (CFRange){0, 1});
        if (CFStringHasSuffix(s, CFSTR("\U0000202c")))
            CFStringDelete(s, (CFRange){CFStringGetLength(s) - 1, 1});
        *refs[i] = CFStringCreateCopy(NULL, s);
        CFRelease(arr[i]);
        CFRelease(s);
    }

    getHealthData(&bodyweight, multiplier, &weightUnit);
}

static void createRootAndLibDict(WorkoutPlist *data) {
    CFBundleRef bundle = CFBundleGetMainBundle();
    CFURLRef url = CFBundleCopyResourceURL(bundle, CFSTR("WorkoutData"), CFSTR("plist"), NULL);
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

static CFArrayRef getCurrentWeekForPlan(WorkoutPlist *data, unsigned char plan) {
    CFArrayRef plans = CFDictionaryGetValue(data->root, CFSTR("P"));
    return CFArrayGetValueAtIndex(CFArrayGetValueAtIndex(plans, plan), weekInPlan);
}

static CFStringRef createTitle(int type, int index) {
    CFStringRef key = formatStr(NULL, CFSTR("wkNames%d%02d"), type, index);
    CFStringRef title = localize(key);
    CFRelease(key);
    return title;
}

static Workout *buildWorkout(WorkoutPlist *data, WorkoutParams *params, const short *lifts) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    float weights[4] = {[3] = 0};
    short customSets = 1, customReps = 0, customCircuitReps = 0;
    if (params->type == WorkoutStrength) {
        float multiplier = params->weight / 100.f;

        weights[0] = lifts[0] * multiplier;
        if (params->index < StrengthIndexTestMax) {
            weights[1] = lifts[LiftBench] * multiplier;
            if (params->index == StrengthIndexMain) {
                int weight = (int)((lifts[LiftPullup] + bodyweight) * multiplier) - bodyweight;
                weights[2] = max(weight, 0);
            } else {
                weights[2] = lifts[LiftDeadlift] * multiplier;
            }
        } else {
            for (int i = 1; i < 4; ++i) weights[i] = lifts[i];
        }
        if (CFBooleanGetValue(CFLocaleGetValue(locale, kCFLocaleUsesMetricSystem))) {
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

    CFArrayRef libArr = CFArrayGetValueAtIndex(data->lib, params->type);
    CFArrayRef acts = CFArrayGetValueAtIndex(libArr, params->index);
    int wSize = (int)CFArrayGetCount(acts);
    customAssert(wSize > 0)

    Workout workout = {
        .jIdx = params->index,
        .size = wSize,
        .type = params->type,
        .day = params->day,
        .testMax = params->type == WorkoutStrength && params->index == StrengthIndexTestMax
    };
    workout.activities = malloc((unsigned)wSize * sizeof(Circuit));
    workout.group = workout.activities;
    bool multiple = wSize > 1;

    for (int i = 0; i < wSize; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(acts, i);
        CFStringRef circuitHeader = NULL;
        short circuitReps = customCircuitReps, exerciseSets = customSets;
        unsigned char circuitType;

        CFNumberGetValue(CFDictionaryGetValue(act, Keys.type), kCFNumberCharType, &circuitType);
        if (!circuitReps)
            CFNumberGetValue(CFDictionaryGetValue(act, Keys.reps), kCFNumberShortType, &circuitReps);
#if TARGET_OS_SIMULATOR
        if (circuitType == CircuitAMRAP) circuitReps = wSize > 1 ? 1 : 2;
#endif

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("E"));
        int cSize = (int)CFArrayGetCount(foundExercises);
        customAssert(cSize > 0)

        if (params->type == WorkoutHIC && circuitType == CircuitRounds && cSize == 1) {
            exerciseSets = circuitReps;
            circuitReps = 1;
        }

        Circuit circuit = {.size = cSize, .reps = circuitReps, .type = circuitType};
        circuit.exercises = malloc((unsigned)cSize * sizeof(ExerciseEntry));

        if (circuitType == CircuitAMRAP) {
            if (multiple) {
                circuitHeader = formatStr(locale, amrapLoc, i + 1, wSize, circuitReps);
            } else {
                circuitHeader = formatStr(locale, amrapFormat, circuitReps);
            }
        } else if (circuitReps > 1) {
            if (multiple) {
                circuitHeader = formatStr(locale, roundsLoc, i + 1, wSize, 1, circuitReps);
            } else {
                circuitHeader = formatStr(locale, roundsFormat, 1, circuitReps);
            }
            circuit.range = findNumber(circuitHeader, locale, oneStr, rounds1);
        } else if (multiple) {
            circuitHeader = formatStr(locale, circuitProgressFmt, i + 1, wSize);
        }

        if (circuitHeader) {
            circuit.header = CFStringCreateMutableCopy(NULL, 80, circuitHeader);
            CFRetain(circuit.header);
            CFRelease(circuitHeader);
        }

        for (int j = 0; j < cSize; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            short rest = 0, eReps = customReps;
            unsigned char eType;

            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.type), kCFNumberCharType, &eType);
            if (!eReps)
                CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.reps), kCFNumberShortType, &eReps);
#if TARGET_OS_SIMULATOR
            if (eType == ExerciseDuration) eReps = workout.type == WorkoutHIC ? 15 : 60;
#endif
            ExerciseEntry e = {.reps = eReps, .sets = exerciseSets, .type = eType};

            if (exerciseSets > 1) {
                CFStringRef header = formatStr(locale, setsFormat, 1, exerciseSets);
                e.hRange = findNumber(header, locale, oneStr, sets1);
                e.header = CFStringCreateMutableCopy(NULL, 32, header);
                CFRelease(header);
            }

            CFNumberGetValue(CFDictionaryGetValue(exDict, CFSTR("B")), kCFNumberShortType, &rest);
            if (rest) e.rest = formatStr(locale, restFormat, rest);

            int n;
            CFNumberGetValue(CFDictionaryGetValue(exDict, Keys.index), kCFNumberIntType, &n);
            CFStringRef nameKey = formatStr(NULL, CFSTR("exNames%02d"), n);
            CFStringRef name = localize(nameKey);
            CFStringRef titleStr;

            if (e.type == ExerciseReps) {
                if (workout.type == WorkoutStrength) {
                    titleStr = formatStr(locale, weightFormat, name, e.reps, weights[j], weightUnit);
                } else {
                    titleStr = formatStr(locale, repsFormat, name, e.reps);
                }
            } else if (e.type == ExerciseDuration) {
                if (e.reps > 120) {
                    titleStr = formatStr(locale, durationMinsFormat, name, e.reps / 60.f);
                } else {
                    titleStr = formatStr(locale, durationSecsFormat, name, e.reps);
                }
            } else {
                titleStr = formatStr(locale, distanceFormat, name, e.reps, (5 * e.reps) >> 2);
            }
            CFRelease(nameKey);
            CFRelease(name);

            e.title = CFStringCreateMutableCopy(NULL, 64, titleStr);
            CFRetain(e.title);
            CFRelease(titleStr);
            if (circuit.type == CircuitDecrement && e.type == ExerciseReps) {
                CFStringRef ten = formatStr(locale, CFSTR("%d"), 10);
                e.tRange = findNumber(e.title, locale, ten, NULL);
                CFRelease(ten);
            }

            memcpy(&circuit.exercises[j], &e, sizeof(ExerciseEntry));
        }

        if (circuit.type == CircuitDecrement) circuit.completedReps = circuit.exercises[0].reps;
        memcpy(&workout.activities[i], &circuit, sizeof(Circuit));
    }
    CFRelease(locale);

    Workout *w = malloc(sizeof(Workout));
    memcpy(w, &workout, sizeof(Workout));
    CFRelease(data->root);
    return w;
}

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int newValue) { weekInPlan = newValue; }
#endif

void setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names) {
    WorkoutPlist plist;
    createRootAndLibDict(&plist);
    CFArrayRef currWeek = getCurrentWeekForPlan(&plist, plan);
    int index = 0;
    unsigned char type;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), kCFNumberCharType, &type);
        if (type > WorkoutHIC) continue;

        CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), kCFNumberIntType, &index);
        names[i] = createTitle(type, index);
    }
    CFRelease(plist.root);
}

Workout *getWeeklyWorkout(int index, unsigned char plan, const short *lifts) {
    WorkoutPlist plist;
    int arrayIdx;
    short sets, reps, weight;
    unsigned char type;
    createRootAndLibDict(&plist);
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeekForPlan(&plist, plan), index);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.type), kCFNumberCharType, &type);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.index), kCFNumberIntType, &arrayIdx);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("S")), kCFNumberShortType, &sets);
    CFNumberGetValue(CFDictionaryGetValue(day, Keys.reps), kCFNumberShortType, &reps);
    CFNumberGetValue(CFDictionaryGetValue(day, CFSTR("W")), kCFNumberShortType, &weight);

    WorkoutParams params = {arrayIdx, sets, reps, weight, type, (unsigned char)index};
    return buildWorkout(&plist, &params, lifts);
}

CFArrayRef createWorkoutNames(unsigned char type) {
    static const int counts[] = {2, 8, 1, 27};
    int len = counts[type];
    CFMutableArrayRef results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFStringRef title = createTitle(type, i);
        CFArrayAppendValue(results, title);
        CFRelease(title);
    }
    return results;
}

Workout *getWorkoutFromLibrary(WorkoutParams *params, const short *lifts) {
    WorkoutPlist plist;
    createRootAndLibDict(&plist);
    return buildWorkout(&plist, params, lifts);
}
