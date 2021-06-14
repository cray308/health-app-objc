//
//  Exercise.c
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#include "Exercise.h"
#include "array.h"

#define freeExerciseEntry(x) CFRelease((x).name)
#define freeExerciseGroup(x) array_free(exEntry, (x).exercises)

gen_array(exEntry, ExerciseEntry, DSDefault_shallowCopy, freeExerciseEntry)
gen_array(exGroup, ExerciseGroup, DSDefault_shallowCopy, freeExerciseGroup)

void workout_setup_activities(Workout *w) {
    w->activities = array_new(exGroup);
}

void workout_add_activity(Workout *w, ExerciseGroup *g) {
    array_push_back(exGroup, w->activities, *g);
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

void workout_setSetsAndRepsForExercises(Workout *w, unsigned int sets, unsigned int reps) {
    ExerciseEntry *e;
    Array_exEntry *exercises = w->activities->arr[0].exercises;
    array_iter(exercises, e) {
        e->sets = sets;
        e->reps = reps;
    }
}

void workout_setWeightsForExercises(Workout *w, unsigned int *weights, int size) {
    ExerciseEntry *entries = w->activities->arr[0].exercises->arr;
    for (int i = 0; i < size; ++i) {
        entries[i].weight = weights[i];
    }
}

void exerciseGroup_setup_exercises(ExerciseGroup *g) {
    g->exercises = array_new(exEntry);
}

void exerciseGroup_add_exercise(ExerciseGroup *g, ExerciseEntry *e) {
    array_push_back(exEntry, g->exercises, *e);
}

ExerciseEntry *exerciseGroup_getExercise(ExerciseGroup *g, int i) {
    return array_at(exEntry, g->exercises, i);
}

int exerciseGroup_getNumberOfExercises(ExerciseGroup *g) {
    return array_size(g->exercises);
}
