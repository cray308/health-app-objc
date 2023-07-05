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
    CFBundleRef bundle = CFBundleGetMainBundle();
    CFURLRef url = CFBundleCopyResourceURL(bundle, CFSTR("WorkoutData"), CFSTR("plist"), NULL);
    CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, url);
    CFReadStreamOpen(stream);
    uint8_t *buf = malloc(9000);
    CFDataRef plistData = CFDataCreate(NULL, buf, CFReadStreamRead(stream, buf, 9000));
    free(buf);
    CFReadStreamClose(stream);
    data->root = CFPropertyListCreateWithData(NULL, plistData, 0, NULL, NULL);
    data->lib = CFDictionaryGetValue(data->root, CFSTR("L"));
    CFRelease(url);
    CFRelease(stream);
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

static Workout *buildWorkout(WorkoutPlist *data, WorkoutParams const *params,
                             int const *lifts, CFMutableStringRef **headers) {
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
            for (int i = 1; i < 4; ++i) {
                weights[i] = lifts[i];
            }
        }

        if (isMetric(locale)) {
            for (int i = 0; i < 4; ++i) {
                weights[i] *= ToKg;
            }
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
    CFStringRef hintFormat = localize(CFSTR("exerciseProgress"));
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
    Workout *workout = calloc(1, sizeof(Workout));
    workout->size = (int)CFArrayGetCount(activities);
    workout->circuits = calloc((unsigned)workout->size, sizeof(Circuit));
    workout->nameIdx = params->index;
    workout->type = params->type;
    workout->day = params->day;
    workout->testMax = params->type == WorkoutStrength && params->index == StrengthIndexTestMax;

    *headers = calloc((unsigned)workout->size, sizeof(CFMutableStringRef));
    bool multiple = workout->size > 1;

    for (int i = 0; i < workout->size; ++i) {
        CFDictionaryRef cDict = CFArrayGetValueAtIndex(activities, i);
        CFArrayRef exercises = CFDictionaryGetValue(cDict, CFSTR("E"));
        Circuit *c = &workout->circuits[i];
        c->size = (int)CFArrayGetCount(exercises);
        c->exercises = calloc((unsigned)c->size, sizeof(Exercise));
        getDictValue(cDict, EMKeys.type, kCFNumberCharType, &c->type);
        c->reps = customCircuitReps;
        if (!c->reps) getDictValue(cDict, EMKeys.reps, kCFNumberShortType, &c->reps);
#if TARGET_OS_SIMULATOR
        if (c->type == CircuitAMRAP) c->reps = 1;
#endif

        CFStringRef cHeader = NULL;
        short exerciseSets = customSets;

        if (params->type == WorkoutHIC && c->type == CircuitRounds && c->size == 1) {
            exerciseSets = c->reps;
            c->reps = 1;
        }

        if (c->type == CircuitAMRAP) {
            if (multiple) {
                cHeader = formatStr(locale, amrapMultipleFormat, i + 1, workout->size, c->reps);
            } else {
                cHeader = formatStr(locale, amrapFormat, c->reps);
            }
        } else if (c->reps > 1) {
            if (multiple) {
                cHeader = formatStr(locale, roundsMultipleFormat, i + 1, workout->size, 1, c->reps);
            } else {
                cHeader = formatStr(locale, roundsFormat, 1, c->reps);
            }
            c->range = findNumber(cHeader, locale, one, rounds1);
        } else if (multiple) {
            cHeader = formatStr(locale, circuitProgressFormat, i + 1, workout->size);
        }

        if (cHeader) {
            (*headers)[i] = CFStringCreateMutableCopy(NULL, 80, cHeader);
            CFRelease(cHeader);
        }

        bool addHint = c->size > 1;

        for (int j = 0; j < c->size; ++j) {
            CFDictionaryRef exDict = CFArrayGetValueAtIndex(exercises, j);
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

            if (addHint) e->hint = formatStr(locale, hintFormat, j + 1, c->size);

            int rest, nameIdx;
            getDictValue(exDict, CFSTR("B"), kCFNumberIntType, &rest);
            if (rest) e->rest = formatStr(locale, restFormat, rest);

            getDictValue(exDict, EMKeys.index, kCFNumberIntType, &nameIdx);
            CFStringRef nameKey = formatStr(NULL, CFSTR("exNames%02d"), nameIdx);
            CFStringRef name = localize(nameKey), exTitle;

            if (e->type == ExerciseReps) {
                if (workout->type == WorkoutStrength) {
                    exTitle = formatStr(locale, weightFormat, name, e->reps, weights[j], weightUnit);
                } else {
                    exTitle = formatStr(locale, repsFormat, name, e->reps);
                }
            } else if (e->type == ExerciseDuration) {
                if (e->reps > 120) {
                    exTitle = formatStr(locale, durationMinsFormat, name, e->reps / 60.f);
                } else {
                    exTitle = formatStr(locale, durationSecsFormat, name, e->reps);
                }
            } else {
                exTitle = formatStr(locale, distanceFormat, name, e->reps, (5 * e->reps) >> 2);
            }
            e->title = CFStringCreateMutableCopy(NULL, 64, exTitle);
            CFRelease(nameKey);
            CFRelease(name);
            CFRelease(exTitle);
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
    CFRelease(hintFormat);
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
    int index;
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

Workout *getWeeklyWorkout(int index, uint8_t plan, int const *lifts, CFMutableStringRef **headers) {
    WorkoutPlist plist;
    createPlist(&plist);
    WorkoutParams params = {.day = (uint8_t)index};
    CFDictionaryRef day = CFArrayGetValueAtIndex(getCurrentWeek(&plist, plan), index);
    getDictValue(day, EMKeys.type, kCFNumberCharType, &params.type);
    getDictValue(day, EMKeys.index, kCFNumberIntType, &params.index);
    getDictValue(day, CFSTR("S"), kCFNumberShortType, &params.sets);
    getDictValue(day, EMKeys.reps, kCFNumberShortType, &params.reps);
    getDictValue(day, CFSTR("W"), kCFNumberShortType, &params.weight);
    return buildWorkout(&plist, &params, lifts, headers);
}

CFArrayRef createWorkoutNames(uint8_t type) {
    int len = (int []){2, 8, 1, 27}[type];
    CFMutableArrayRef results = CFArrayCreateMutable(NULL, len, &kCFTypeArrayCallBacks);
    for (int i = 0; i < len; ++i) {
        CFStringRef title = createTitle(type, i);
        CFArrayAppendValue(results, title);
        CFRelease(title);
    }
    return results;
}

Workout *getWorkoutFromLibrary(WorkoutParams const *params,
                               int const *lifts, CFMutableStringRef **headers) {
    WorkoutPlist plist;
    createPlist(&plist);
    return buildWorkout(&plist, params, lifts, headers);
}
