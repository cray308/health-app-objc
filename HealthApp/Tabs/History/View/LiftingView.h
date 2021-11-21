#ifndef LiftingView_h
#define LiftingView_h

#include "HistoryTabCoordinator.h"

extern Class LiftVClass;
extern Ivar LiftVDataRef;

typedef struct __liftVData {
    id chart;
    LiftChartViewModel *model;
} LiftVData;

id liftingView_init(LiftChartViewModel *model, id formatter);
void liftingView_update(id self, int count, bool isSmall);

#endif /* LiftingView_h */
