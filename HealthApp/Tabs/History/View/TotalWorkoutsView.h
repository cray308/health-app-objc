#ifndef TotalWorkoutsView_h
#define TotalWorkoutsView_h

#include "HistoryTabCoordinator.h"

extern Class TotalWorkoutsViewClass;
extern Ivar TotalWorkoutsViewDataRef;

typedef struct __totalWorkoutsVData {
    id chart;
    TotalWorkoutsChartViewModel *model;
} TotalWorkoutsViewData;

id totalWorkoutsView_init(TotalWorkoutsChartViewModel *model, id formatter);
void totalWorkoutsView_update(id self, int count, bool isSmall);

#endif /* TotalWorkoutsView_h */
