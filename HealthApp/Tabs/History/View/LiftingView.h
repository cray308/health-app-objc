#ifndef LiftingView_h
#define LiftingView_h

#include "HistoryTabCoordinator.h"

extern Class LiftViewClass;
extern Ivar LiftViewDataRef;

typedef struct __liftVData {
    id chart;
    LiftChartViewModel *model;
} LiftViewData;

id liftingView_init(LiftChartViewModel *model, id formatter);
void liftingView_update(id self, int count, bool isSmall);

#endif /* LiftingView_h */
