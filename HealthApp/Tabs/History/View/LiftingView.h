#ifndef LiftingView_h
#define LiftingView_h

#include "HistoryVC.h"

extern Class LiftViewClass;
extern Ivar LiftViewDataRef;

typedef struct {
    id chart;
    LiftChartModel *model;
} LiftViewData;

id liftingView_init(LiftChartModel *model, id formatter);
void liftingView_update(id self, int count, int index);

#endif /* LiftingView_h */
