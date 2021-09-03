//
//  Exercise.c
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#include "Exercise.h"
#include "AppUserData.h"
#include "CocoaHelpers.h"

#define freeExerciseEntry(x) CFRelease((x).name)
#define freeExerciseGroup(x) array_free(exEntry, (x).exercises)
#define copyStringRef(x, y) x = CFStringCreateCopy(NULL, y)
#define deleteStringRef(x) CFRelease(x)

gen_array_source(exEntry, ExerciseEntry, DSDefault_shallowCopy, freeExerciseEntry)
gen_array_source(exGroup, ExerciseGroup, DSDefault_shallowCopy, freeExerciseGroup)
gen_array_source(str, CFStringRef, copyStringRef, deleteStringRef)

static CFStringRef const repsKey = CFSTR("reps");
static CFStringRef const typeKey = CFSTR("type");
static CFStringRef const indexKey = CFSTR("index");
static CFStringRef const titleKey = CFSTR("title");

CFDictionaryRef createRootAndLibDict(CFDictionaryRef *lib) {
    CFStringRef path = ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef))objc_msgSend)
    (objc_staticMethod(objc_getClass("NSBundle"), sel_getUid("mainBundle")),
     sel_getUid("pathForResource:ofType:"), CFSTR("WorkoutData"), CFSTR("plist"));
    CFDictionaryRef root = ((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)
    (allocClass("NSDictionary"), sel_getUid("initWithContentsOfFile:"), path);
    *lib = CFDictionaryGetValue(root, CFSTR("library"));
    return root;
}

CFArrayRef getLibraryArrayForType(CFDictionaryRef libDict, unsigned char type) {
    static CFStringRef const keys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
    if (type > 3) return NULL;
    return CFDictionaryGetValue(libDict, keys[type]);
}

CFArrayRef getCurrentWeekForPlan(CFDictionaryRef root, unsigned char plan, int week) {
    CFDictionaryRef plans = CFDictionaryGetValue(root, CFSTR("plans"));
    CFArrayRef weeks = CFDictionaryGetValue(plans, plan == 0 ? CFSTR("bb") : CFSTR("cc"));
    if (week >= CFArrayGetCount(weeks)) return NULL;
    return CFArrayGetValueAtIndex(weeks, week);
}

void buildWorkoutFromDict(CFDictionaryRef dict, int index,
                          unsigned char type, int sets, int reps, int weight, Workout *w) {
    CFArrayRef foundActivities = CFDictionaryGetValue(dict, CFSTR("activities"));
    CFNumberRef number;
    CFStringRef str;
    unsigned int tempInt = 0;

    int nActivities = (int) CFArrayGetCount(foundActivities);

    w->type = type;
    w->activities = array_new(exGroup);
    str = CFDictionaryGetValue(dict, titleKey);
    w->title = CFStringCreateCopy(NULL, str);

    if (nActivities <= 0) return;

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        ExerciseGroup activities = { .exercises = array_new(exEntry) };

        number = CFDictionaryGetValue(act, typeKey);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        activities.type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(act, repsKey);
        CFNumberGetValue(number, kCFNumberIntType, &activities.reps);

        CFArrayRef foundExercises = CFDictionaryGetValue(act, CFSTR("exercises"));
        for (int j = 0; j < CFArrayGetCount(foundExercises); ++j) {
            CFDictionaryRef ex = CFArrayGetValueAtIndex(foundExercises, j);
            ExerciseEntry exercise = { .sets = 1 };

            number = CFDictionaryGetValue(ex, typeKey);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            exercise.type = (unsigned char) tempInt;
            number = CFDictionaryGetValue(ex, repsKey);
            CFNumberGetValue(number, kCFNumberIntType, &exercise.reps);
            number = CFDictionaryGetValue(ex, CFSTR("rest"));
            CFNumberGetValue(number, kCFNumberIntType, &exercise.rest);
            str = CFDictionaryGetValue(ex, CFSTR("name"));
            exercise.name = CFStringCreateCopy(NULL, str);
            array_push_back(exEntry, activities.exercises, exercise);
        }

        if (activities.type == ExerciseContainerTypeDecrement) {
            activities.completedReps = activities.exercises->arr[0].reps;
        }
        array_push_back(exGroup, w->activities, activities);
    }

    Array_exEntry *exercises = w->activities->arr[0].exercises;
    ExerciseEntry *e;
    switch (type) {
        case WorkoutTypeStrength: ;
            int nExercises = exercises->size;
            double weightMultiplier = (double) weight / 100.0;
            array_iter(exercises, e) {
                e->sets = sets;
                e->reps = reps;
            }
            ExerciseEntry *entries = exercises->arr;
            entries[0].weight = (int) (weightMultiplier * (double) appUserDataShared->liftMaxes[0]);

            if (nExercises >= 3 && index <= 1) {
                entries[1].weight = (int)
                (weightMultiplier * (double) appUserDataShared->liftMaxes[LiftTypeBench]);
                if (index == 0) {
                    entries[2].weight = (int)
                    (weightMultiplier * (double) appUserDataShared->liftMaxes[LiftTypePullup]);
                } else {
                    entries[2].weight = (int)
                    (weightMultiplier * (double) appUserDataShared->liftMaxes[LiftTypeDeadlift]);
                }
            } else if (nExercises >= 4 && index == 2) {
                for (int i = 1; i < 4; ++i) {
                    entries[i].weight = (appUserDataShared->liftMaxes[i]);
                }
            }
            break;
        case WorkoutTypeSE:
            w->activities->arr[0].reps = sets;
            array_iter(exercises, e) {
                e->reps = reps;
            }
            break;
        case WorkoutTypeEndurance: ;
            int duration = reps * 60;
            array_iter(exercises, e) {
                e->reps = duration;
            }
        default:
            break;
    }
    w->group = &w->activities->arr[0];
    w->entry = &w->group->exercises->arr[0];
    for (int i = 0; i < 2; ++i) {
        memcpy(&w->timers[i], &(WorkoutTimer){.info = {.type = i}}, sizeof(WorkoutTimer));
        pthread_mutex_init(&w->timers[i].lock, NULL);
        pthread_cond_init(&w->timers[i].cond, NULL);
    }
}

void exerciseManager_setWeeklyWorkoutNames(unsigned char plan, int week, CFStringRef *names) {
    CFDictionaryRef lib;
    CFDictionaryRef root = createRootAndLibDict(&lib);

    CFArrayRef currWeek = getCurrentWeekForPlan(root, plan, week);
    if (!currWeek) goto cleanup;
    int index = 0, tempInt = 0;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);

        CFNumberRef number = CFDictionaryGetValue(day, typeKey);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        unsigned char type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(day, indexKey);
        CFNumberGetValue(number, kCFNumberIntType, &index);

        CFArrayRef libArr = getLibraryArrayForType(lib, type);
        if (!libArr) continue;

        CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
        CFStringRef str = CFDictionaryGetValue(foundWorkout, titleKey);
        names[i] = CFStringCreateCopy(NULL, str);
    }
cleanup:
    CFRelease(root);
}

Workout *exerciseManager_getWeeklyWorkoutAtIndex(unsigned char plan, int week, int index) {
    Workout *w = NULL;
    CFDictionaryRef lib;
    CFDictionaryRef root = createRootAndLibDict(&lib);

    CFArrayRef currWeek = getCurrentWeekForPlan(root, plan, week);
    if (!currWeek) goto cleanup;
    CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, index);
    int idx = 0, sets = 0, reps = 0, weight = 0;
    unsigned char type = 0;

    CFNumberRef number = CFDictionaryGetValue(day, typeKey);
    CFNumberGetValue(number, kCFNumberIntType, &idx);
    type = (unsigned char) idx;

    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!libArr) goto cleanup;

    number = CFDictionaryGetValue(day, indexKey);
    CFNumberGetValue(number, kCFNumberIntType, &idx);
    number = CFDictionaryGetValue(day, CFSTR("sets"));
    CFNumberGetValue(number, kCFNumberIntType, &sets);
    number = CFDictionaryGetValue(day, repsKey);
    CFNumberGetValue(number, kCFNumberIntType, &reps);
    number = CFDictionaryGetValue(day, CFSTR("weight"));
    CFNumberGetValue(number, kCFNumberIntType, &weight);

    w = calloc(1, sizeof(Workout));
    w->day = index;

    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, idx);
    buildWorkoutFromDict(foundWorkout, idx, type, sets, reps, weight, w);
cleanup:
    CFRelease(root);
    return w;
}

Array_str *exerciseManager_getWorkoutNamesForType(unsigned char type) {
    Array_str *results = NULL;
    CFDictionaryRef lib;
    CFDictionaryRef root = createRootAndLibDict(&lib);

    int len = 0;
    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!(libArr && (len = (int) CFArrayGetCount(libArr)))) goto cleanup;

    if (type == WorkoutTypeStrength) len = 2;

    results = array_new(str);
    array_reserve(str, results, len);
    for (int i = 0; i < len; ++i) {
        CFDictionaryRef week = CFArrayGetValueAtIndex(libArr, i);
        CFStringRef title = CFDictionaryGetValue(week, titleKey);
        array_push_back(str, results, title);
    }
cleanup:
    CFRelease(root);
    return results;
}

Workout *exerciseManager_getWorkoutFromLibrary(unsigned char type,
                                               int index, int sets, int reps, int weight) {
    Workout *w = NULL;
    CFDictionaryRef lib;
    CFDictionaryRef root = createRootAndLibDict(&lib);
    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!libArr) goto cleanup;
    w = calloc(1, sizeof(Workout));
    w->day = -1;
    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
    buildWorkoutFromDict(foundWorkout, index, type, sets, reps, weight, w);
cleanup:
    CFRelease(root);
    return w;
}

CFStringRef exerciseGroup_createHeader(ExerciseGroup *g) {
    if (g->type == ExerciseContainerTypeRounds && g->reps > 1) {
        int completed = g->completedReps == g->reps ? g->reps : g->completedReps + 1;
        return CFStringCreateWithFormat(NULL, NULL, CFSTR("Round %d of %d"), completed, g->reps);
    } else if (g->type == ExerciseContainerTypeAMRAP) {
        return CFStringCreateWithFormat(NULL, NULL, CFSTR("AMRAP %d mins"), g->reps);
    }
    return NULL;
}

CFStringRef exerciseEntry_createSetsTitle(ExerciseEntry *e) {
    if (e->sets == 1)
        return NULL;
    int completed = e->completedSets == e->sets ? e->sets : e->completedSets + 1;
    return CFStringCreateWithFormat(NULL, NULL, CFSTR("Set %d of %d"), completed, e->sets);
}

CFStringRef exerciseEntry_createTitle(ExerciseEntry *e) {
    if (e->state == ExerciseStateResting)
        return CFStringCreateWithFormat(NULL, NULL, CFSTR("Rest: %d s"), e->rest);
    switch (e->type) {
        case ExerciseTypeReps:
            if (e->weight > 1) {
                return CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ x %d @ %d lbs"),
                                                e->name, e->reps, e->weight);
            }
            return CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ x %d"), e->name, e->reps);

        case ExerciseTypeDuration:
            if (e->reps > 120) {
                double minutes = (double) e->reps / 60.0;
                return CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ for %.1f mins"),
                                                e->name, minutes);
            }
            return CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ for %d sec"), e->name, e->reps);

        default: ;
            int rowingDist = (5 * e->reps) / 4;
            return CFStringCreateWithFormat(NULL, NULL, CFSTR("Run/row %d/%d meters"),
                                            e->reps, rowingDist);
    }
}
