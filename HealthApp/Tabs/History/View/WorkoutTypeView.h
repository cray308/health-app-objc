#ifndef WorkoutTypeView_h
#define WorkoutTypeView_h

#include "HistoryTabCoordinator.h"

extern Class WorkoutTypeViewClass;
extern Ivar WorkoutTypeViewDataRef;

typedef struct __workoutTypeVData {
    id chart;
    WorkoutTypeChartViewModel *model;
} WorkoutTypeViewData;

id workoutTypeView_init(WorkoutTypeChartViewModel *model, id formatter);
void workoutTypeView_update(id self, int count, bool isSmall);

#endif /* WorkoutTypeView_h */
