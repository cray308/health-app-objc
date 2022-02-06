#ifndef WorkoutTypeView_h
#define WorkoutTypeView_h

#include "HistoryVC.h"

extern Class WorkoutTypeViewClass;

typedef struct {
    id chart;
    WorkoutTypeChartModel *model;
} WorkoutTypeView;

id workoutTypeView_init(WorkoutTypeChartModel *model, id formatter);
void workoutTypeView_update(id self, int count, int index, int ref);

#endif /* WorkoutTypeView_h */
