#ifndef LiftingView_h
#define LiftingView_h

#include "HistoryVC.h"

extern Class LiftViewClass;

typedef struct {
    id chart;
    LiftChartModel *model;
} LiftView;

id liftingView_init(LiftChartModel *model, id formatter);
void liftingView_update(id self, int count, int index, int ref);

#endif /* LiftingView_h */
