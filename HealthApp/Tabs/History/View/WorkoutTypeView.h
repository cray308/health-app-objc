#ifndef WorkoutTypeView_h
#define WorkoutTypeView_h

#include "HistoryVC.h"

extern Class WorkoutTypeViewClass;
extern Ivar WorkoutTypeViewDataRef;

typedef struct __workoutTypeVData {
    id chart;
    WorkoutTypeChartModel *model;
} WorkoutTypeViewData;

id workoutTypeView_init(WorkoutTypeChartModel *model, id formatter);
void workoutTypeView_update(id self, int count, int index);

#endif /* WorkoutTypeView_h */
