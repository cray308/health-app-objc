#ifndef WorkoutTypeView_h
#define WorkoutTypeView_h

#include "HistoryTabCoordinator.h"

extern Class WorkoutTypeClass;
extern Ivar WorkoutTypeDataRef;

typedef struct __workoutTypeData {
    id chart;
    WorkoutTypeChartViewModel *model;
} WorkoutTypeData;

id workoutTypeView_init(WorkoutTypeChartViewModel *model, id formatter);
void workoutTypeView_update(id self, int count, bool isSmall);

#endif /* WorkoutTypeView_h */
