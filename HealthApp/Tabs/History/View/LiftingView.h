#ifndef LiftingView_h
#define LiftingView_h

#include "HistoryTabCoordinator.h"

extern Class LiftViewClass;
extern Ivar LiftViewDataRef;

typedef struct __liftVData {
    id chart;
    LiftChartModel *model;
} LiftViewData;

id liftingView_init(LiftChartModel *model, id formatter);
void liftingView_update(id self, int count, int index);

#endif /* LiftingView_h */
