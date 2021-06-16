//
//  Exercise.c
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#include "Exercise.h"
#include "array.h"
#include "AppUserData.h"

#define freeExerciseEntry(x) CFRelease((x).name)
#define freeExerciseGroup(x) array_free(exEntry, (x).exercises)

gen_array(exEntry, ExerciseEntry, DSDefault_shallowCopy, freeExerciseEntry)
gen_array(exGroup, ExerciseGroup, DSDefault_shallowCopy, freeExerciseGroup)

static CFStringRef repsKey = CFSTR("reps");
static CFStringRef typeKey = CFSTR("type");
static CFStringRef titleKey = CFSTR("title");
static CFStringRef activitiesKey = CFSTR("activities");
static CFStringRef exercisesKey = CFSTR("exercises");
static CFStringRef restKey = CFSTR("rest");
static CFStringRef nameKey = CFSTR("name");

static CFStringRef titleFormatWeight = CFSTR("%@ x %u @ %u lbs");
static CFStringRef titleFormatReps = CFSTR("%@ x %u");
static CFStringRef titleFormatDurationMins = CFSTR("%@ for %.1f mins");
static CFStringRef titleFormatDurationSec = CFSTR("%@ for %u sec");
static CFStringRef titleFormatDistance = CFSTR("%@ %u/%u meters");

static CFStringRef setsTextFormat = CFSTR("Set %u of %u");
static CFStringRef roundsTextFormat = CFSTR("Round %u of %u");
static CFStringRef amrapTextFormat = CFSTR("AMRAP %u mins");

void workout_buildFromDictionary(CFDictionaryRef dict, unsigned int index, unsigned char type, unsigned int sets, unsigned int reps, unsigned int weight, Workout *w) {
    CFArrayRef foundActivities = CFDictionaryGetValue(dict, activitiesKey);
    CFNumberRef number;
    CFStringRef str;
    unsigned int tempInt = 0;

    int nActivities = (int) CFArrayGetCount(foundActivities);
    if (nActivities <= 0) return;

    w->type = type;
    w->activities = array_new(exGroup);
    str = CFDictionaryGetValue(dict, titleKey);
    w->title = CFStringCreateCopy(NULL, str);

    for (int i = 0; i < nActivities; ++i) {
        CFDictionaryRef act = CFArrayGetValueAtIndex(foundActivities, i);
        ExerciseGroup activities = { .exercises = array_new(exEntry) };

        number = CFDictionaryGetValue(act, typeKey);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        activities.type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(act, repsKey);
        CFNumberGetValue(number, kCFNumberIntType, &activities.reps);

        CFArrayRef foundExercises = CFDictionaryGetValue(act, exercisesKey);
        for (int j = 0; j < CFArrayGetCount(foundExercises); ++j) {
            CFDictionaryRef ex = CFArrayGetValueAtIndex(foundExercises, j);
            ExerciseEntry exercise = { .sets = 1 };

            number = CFDictionaryGetValue(ex, typeKey);
            CFNumberGetValue(number, kCFNumberIntType, &tempInt);
            exercise.type = (unsigned char) tempInt;
            number = CFDictionaryGetValue(ex, repsKey);
            CFNumberGetValue(number, kCFNumberIntType, &exercise.reps);
            number = CFDictionaryGetValue(ex, restKey);
            CFNumberGetValue(number, kCFNumberIntType, &exercise.rest);
            str = CFDictionaryGetValue(ex, nameKey);
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
            int nExercises = (int) exercises->size;
            double weightMultiplier = (double) weight / 100.0;
            array_iter(exercises, e) {
                e->sets = sets;
                e->reps = reps;
            }
            ExerciseEntry *entries = exercises->arr;
            entries[0].weight = (unsigned int) (weightMultiplier * (double) appUserDataShared->squatMax);

            if (nExercises >= 3 && index <= 1) {
                entries[1].weight = (unsigned int) (weightMultiplier * (double) appUserDataShared->benchMax);
                if (index == 0) {
                    entries[2].weight = (unsigned int) (weightMultiplier * (double) appUserDataShared->pullUpMax);
                } else {
                    entries[2].weight = (unsigned int) (weightMultiplier * (double) appUserDataShared->deadliftMax);
                }
            } else if (nExercises >= 4 && index == 2) {
                entries[1].weight = (appUserDataShared->pullUpMax);
                entries[2].weight = (unsigned int) (appUserDataShared->benchMax);
                entries[3].weight = (unsigned int) (appUserDataShared->deadliftMax);
            }
            break;
        case WorkoutTypeSE:
            w->activities->arr[0].reps = sets;
            array_iter(exercises, e) {
                e->reps = reps;
            }
            break;
        case WorkoutTypeEndurance: ;
            unsigned int duration = reps * 60;
            array_iter(exercises, e) {
                e->reps = duration;
            }
            break;
        default:
            break;
    }
}

void workout_free(Workout *w) {
    array_free(exGroup, w->activities);
    CFRelease(w->title);
    free(w);
}

int workout_getNumberOfActivities(Workout *w) {
    return w->activities ? (int) w->activities->size : 0;
}

ExerciseGroup *workout_getExerciseGroup(Workout *w, int i) {
    return array_at(exGroup, w->activities, i);
}

ExerciseEntry *exerciseGroup_getExercise(ExerciseGroup *g, int i) {
    return array_at(exEntry, g->exercises, i);
}

int exerciseGroup_getNumberOfExercises(ExerciseGroup *g) {
    return array_size(g->exercises);
}

CFStringRef exercise_createTitleString(ExerciseEntry *e) {
    switch (e->type) {
        case ExerciseTypeReps:
            if (e->weight > 1) {
                return CFStringCreateWithFormat(NULL, NULL, titleFormatWeight, e->name, e->reps, e->weight);
            }
            return CFStringCreateWithFormat(NULL, NULL, titleFormatReps, e->name, e->reps);

        case ExerciseTypeDuration:
            if (e->reps > 120) {
                double minutes = (double) e->reps / 60.0;
                return CFStringCreateWithFormat(NULL, NULL, titleFormatDurationMins, e->name, minutes);
            }
            return CFStringCreateWithFormat(NULL, NULL, titleFormatDurationSec, e->name, e->reps);

        default: ;
            unsigned int rowingDist = (5 * e->reps) / 4;
            return CFStringCreateWithFormat(NULL, NULL, titleFormatDistance, e->name, e->reps, rowingDist);
    }
}

CFStringRef exercise_createSetsString(ExerciseEntry *e) {
    return (e->sets > 1) ? CFStringCreateWithFormat(NULL, NULL, setsTextFormat, e->completedSets + 1, e->sets) : NULL;
}

CFStringRef exerciseGroup_createHeaderText(ExerciseGroup *g) {
    if (g->type == ExerciseContainerTypeRounds && g->reps > 1) {
        return CFStringCreateWithFormat(NULL, NULL, roundsTextFormat, g->completedReps + 1, g->reps);
    } else if (g->type == ExerciseContainerTypeAMRAP) {
        return CFStringCreateWithFormat(NULL, NULL, amrapTextFormat, g->reps);
    }
    return NULL;
}
