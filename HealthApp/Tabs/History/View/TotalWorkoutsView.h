#ifndef TotalWorkoutsView_h
#define TotalWorkoutsView_h

#include "HistoryTabCoordinator.h"

extern Class TotalWorkoutsViewClass;
extern Ivar TotalWorkoutsViewDataRef;

typedef struct __totalWorkoutsVData {
    id chart;
    TotalWorkoutsChartModel *model;
} TotalWorkoutsViewData;

id totalWorkoutsView_init(TotalWorkoutsChartModel *model, id formatter);
void totalWorkoutsView_update(id self, int count, int index);

#endif /* TotalWorkoutsView_h */
