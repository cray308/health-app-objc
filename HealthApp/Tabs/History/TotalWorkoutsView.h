#ifndef TotalWorkoutsView_h
#define TotalWorkoutsView_h

#include "HistoryVC.h"

extern Class TotalWorkoutsViewClass;

typedef struct {
    id chart;
    TotalWorkoutsChartModel *model;
} TotalWorkoutsView;

id totalWorkoutsView_init(TotalWorkoutsChartModel *model, id formatter);
void totalWorkoutsView_update(id self, int count, int index, int ref);

#endif /* TotalWorkoutsView_h */
