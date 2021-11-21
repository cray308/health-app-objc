#ifndef TotalWorkoutsView_h
#define TotalWorkoutsView_h

#include "HistoryTabCoordinator.h"

extern Class TotalWorkoutsClass;
extern Ivar TotalWorkoutsDataRef;

typedef struct __totalWorkoutsData {
    id chart;
    TotalWorkoutsChartViewModel *model;
} TotalWorkoutsData;

id totalWorkoutsView_init(TotalWorkoutsChartViewModel *model, id formatter);
void totalWorkoutsView_update(id self, int count, bool isSmall);

#endif /* TotalWorkoutsView_h */
