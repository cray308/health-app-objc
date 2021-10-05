//
//  ExerciseEntry.c
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#include "ExerciseEntry.h"

#define freeExerciseEntry(x) CFRelease((x).name)

static CFStringRef exerciseHeader;
static CFStringRef exerciseTitleRest;
static CFStringRef exerciseTitleReps;
static CFStringRef exerciseTitleRepsWithWeight;
static CFStringRef exerciseTitleDurationMinutes;
static CFStringRef exerciseTitleDurationSeconds;
static CFStringRef exerciseTitleDistance;

gen_array_source(exEntry, ExerciseEntry, DSDefault_shallowCopy, freeExerciseEntry)

void initExerciseStrings(void) {
    exerciseHeader = localize(CFSTR("exerciseHeader"));
    exerciseTitleRest = localize(CFSTR("exerciseTitleRest"));
    exerciseTitleReps = localize(CFSTR("exerciseTitleReps"));
    exerciseTitleRepsWithWeight = localize(CFSTR("exerciseTitleRepsWithWeight"));
    exerciseTitleDurationMinutes = localize(CFSTR("exerciseTitleDurationMinutes"));
    exerciseTitleDurationSeconds = localize(CFSTR("exerciseTitleDurationSeconds"));
    exerciseTitleDistance = localize(CFSTR("exerciseTitleDistance"));
}

CFStringRef exerciseEntry_createSetsTitle(ExerciseEntry *e) {
    if (e->sets == 1) return NULL;
    int completed = e->completedSets == e->sets ? e->sets : e->completedSets + 1;
    return CFStringCreateWithFormat(NULL, NULL, exerciseHeader, completed, e->sets);
}

CFStringRef exerciseEntry_createTitle(ExerciseEntry *e) {
    if (e->state == ExerciseStateResting)
        return CFStringCreateWithFormat(NULL, NULL, exerciseTitleRest, e->rest);
    switch (e->type) {
        case ExerciseReps:
            if (e->weight > 1) {
                return CFStringCreateWithFormat(NULL, NULL, exerciseTitleRepsWithWeight,
                                                e->name, e->reps, e->weight);
            }
            return CFStringCreateWithFormat(NULL, NULL, exerciseTitleReps, e->name, e->reps);

        case ExerciseDuration:
            if (e->reps > 120) {
                return CFStringCreateWithFormat(NULL, NULL, exerciseTitleDurationMinutes,
                                                e->name, e->reps / 60.f);
            }
            return CFStringCreateWithFormat(NULL, NULL,
                                            exerciseTitleDurationSeconds, e->name, e->reps);

        default:
            return CFStringCreateWithFormat(NULL, NULL, exerciseTitleDistance,
                                            e->reps, (5 * e->reps) / 4);
    }
}

bool exerciseEntry_cycle(ExerciseEntry *e) {
    bool completed = false;
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) {
                startWorkoutTimer(e->timer, e->reps);
                scheduleNotification(e->reps, TimerExercise);
            }
            break;

        case ExerciseStateActive:
            if (e->rest) {
                e->state = ExerciseStateResting;
                break;
            }

        case ExerciseStateResting:
            if (++e->completedSets == e->sets) {
                e->state = ExerciseStateCompleted;
                ++e->timer->exercise;
                completed = true;
            } else {
                e->state = ExerciseStateActive;
            }
        default:
            break;
    }
    return completed;
}
