#include "ExerciseManager.h"
#include "HealthDataManager.h"
#include "StringUtils.h"

int Bodyweight = 165;

typedef struct {
    CFDictionaryRef root;
    CFArrayRef lib;
} WorkoutPlist;

static int weekInPlan;
static CFStringRef weightUnit;
static struct {
    CFStringRef reps;
    CFStringRef type;
    CFStringRef index;
} const EMKeys = {CFSTR("R"), CFSTR("T"), CFSTR("I")};

void initExerciseData(int week) {
    weekInPlan = week;
    getMassData(&Bodyweight, &weightUnit);
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
    CFLocaleRef locale = copyLocale();
    float weights[4] = {[3] = 0};
    short customSets = 1, customReps = 0, customCircuitReps = 0;
    if (params->type == WorkoutStrength) {
        float multiplier = params->weight / 100.f;

        weights[0] = lifts[0] * multiplier;
        if (params->index < StrengthIndexTestMax) {
            weights[1] = lifts[LiftBench] * multiplier;
            if (params->index == StrengthIndexMain) {
                float weight = ((lifts[LiftPullup] + Bodyweight) * multiplier) - Bodyweight;
                weights[2] = max(weight, 0);
            } else {
                weights[2] = lifts[LiftDeadlift] * multiplier;
            }
        } else {
            for (int i = 1; i < 4; ++i) weights[i] = lifts[i];
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

    CFStringRef one = getOneStr(locale), restFormat = localize(CFSTR("exerciseTitleRest"));
    CFStringRef repsFormat = localize(CFSTR("exerciseTitleReps"));
    CFStringRef setsFormat = localize(CFSTR("exerciseHeader"));
    CFStringRef weightFormat = localize(CFSTR("exerciseTitleRepsWithWeight"));
    CFStringRef durationMinsFormat = localize(CFSTR("exerciseTitleDurationMinutes"));
    CFStringRef durationSecsFormat = localize(CFSTR("exerciseTitleDurationSeconds"));
    CFStringRef distanceFormat = localize(CFSTR("exerciseTitleDistance"));
    CFStringRef roundsFormat = localize(CFSTR("circuitHeaderRounds"));
    CFStringRef amrapFormat = localize(CFSTR("circuitHeaderAMRAP"));
    CFStringRef circuitProgressFormat = localize(CFSTR("circuitProgressHint"));
    CFStringRef roundsMultipleFormat = localize(CFSTR("circuitHeaderRoundsMultiple"));
    CFStringRef amrapMultipleFormat = localize(CFSTR("circuitHeaderAMRAPMultiple"));

    CFStringRef sets1, rounds1;
    CFStringRef *refs[] = {&sets1, &rounds1}, substrs[] = {CFSTR("sets1"), CFSTR("rounds1")};
    for (int i = 0; i < 2; ++i) {
        CFStringRef sub = localize(substrs[i]);
        CFMutableStringRef s = CFStringCreateMutableCopy(NULL, CFStringGetLength(sub) + 2, sub);
        if (CFStringHasPrefix(s, CFSTR("\U0000202e")) || CFStringHasPrefix(s, CFSTR("\U0000202f")))
            CFStringDelete(s, (CFRange){0, 1});
        if (CFStringHasSuffix(s, CFSTR("\U0000202c")))
            CFStringDelete(s, (CFRange){CFStringGetLength(s) - 1, 1});
        *refs[i] = CFStringCreateCopy(NULL, s);
        CFRelease(sub);
        CFRelease(s);
    }

    CFArrayRef lib = CFArrayGetValueAtIndex(data->lib, params->type);
    CFArrayRef activities = CFArrayGetValueAtIndex(lib, params->index);
    int nActivities = (int)CFArrayGetCount(activities);
    customAssert(nActivities > 0)

    Workout *workout = calloc(1, sizeof(Workout));
    workout->size = nActivities;
    workout->circuits = calloc((unsigned)nActivities, sizeof(Circuit));
    workout->nameIdx = params->index;
    workout->type = params->type;
    workout->day = params->day;
    workout->testMax = params->type == WorkoutStrength && params->index == StrengthIndexTestMax;
    bool multiple = nActivities > 1;

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(activities, i);
        Circuit *c = &workout->circuits[i];
        CFStringRef cHeader = NULL;
        short exerciseSets = customSets;

        getDictValue(act, EMKeys.type, kCFNumberCharType, &c->type);
        c->reps = customCircuitReps;
        if (!c->reps) getDictValue(act, EMKeys.reps, kCFNumberShortType, &c->reps);
#if TARGET_OS_SIMULATOR
        if (c->type == CircuitAMRAP) c->reps = 1;
#endif

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("E"));
        int nExercises = (int)CFArrayGetCount(foundExercises);
        customAssert(nExercises > 0)

        if (params->type == WorkoutHIC && c->type == CircuitRounds && nExercises == 1) {
            exerciseSets = c->reps;
            c->reps = 1;
        }

        c->exercises = calloc((unsigned)nExercises, sizeof(Exercise));
        c->size = nExercises;

        if (c->type == CircuitAMRAP) {
            if (multiple) {
                cHeader = formatStr(locale, amrapMultipleFormat, i + 1, nActivities, c->reps);
            } else {
                cHeader = formatStr(locale, amrapFormat, c->reps);
            }
        } else if (c->reps > 1) {
            if (multiple) {
                cHeader = formatStr(locale, roundsMultipleFormat, i + 1, nActivities, 1, c->reps);
            } else {
                cHeader = formatStr(locale, roundsFormat, 1, c->reps);
            }
            c->range = findNumber(cHeader, locale, one, rounds1);
        } else if (multiple) {
            cHeader = formatStr(locale, circuitProgressFormat, i + 1, nActivities);
        }

        if (cHeader) {
            c->header = CFStringCreateMutableCopy(NULL, 80, cHeader);
            CFRetain(c->header);
            CFRelease(cHeader);
        }

        for (int j = 0; j < nExercises; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(foundExercises, j);
            Exercise *e = &c->exercises[j];

            getDictValue(exDict, EMKeys.type, kCFNumberCharType, &e->type);
            e->sets = exerciseSets;
            e->reps = customReps;
            if (!e->reps) getDictValue(exDict, EMKeys.reps, kCFNumberShortType, &e->reps);
#if TARGET_OS_SIMULATOR
            if (e->type == ExerciseDuration) e->reps = workout->type == WorkoutHIC ? 5 : 60;
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
                if (workout->type == WorkoutStrength) {
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
    CFRelease(locale);
    CFRelease(one);
    CFRelease(restFormat);
    CFRelease(repsFormat);
    CFRelease(setsFormat);
    CFRelease(weightFormat);
    CFRelease(durationMinsFormat);
    CFRelease(durationSecsFormat);
    CFRelease(distanceFormat);
    CFRelease(roundsFormat);
    CFRelease(amrapFormat);
    CFRelease(circuitProgressFormat);
    CFRelease(roundsMultipleFormat);
    CFRelease(amrapMultipleFormat);
    CFRelease(sets1);
    CFRelease(rounds1);
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

    for (int i = 0; i < 6; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);
        getDictValue(day, EMKeys.type, kCFNumberCharType, &type);
        if (type > WorkoutHIC) continue;

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
