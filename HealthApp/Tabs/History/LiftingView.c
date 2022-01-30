#include "LiftingView.h"
#include <stdlib.h>
#include "SwiftBridging.h"

Class LiftViewClass;
Ivar LiftViewDataRef;

id liftingView_init(LiftChartModel *model, id formatter) {
    id self = createNew(LiftViewClass);
#ifndef __clang_analyzer__
    LiftViewData *data = malloc(sizeof(LiftViewData));
    data->model = model;
    data->chart = createChartView(formatter, (long []){0, 1, 2, 3}, 4, 0);
    disableAutoresizing(data->chart);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 550, false);
    object_setIvar(self, LiftViewDataRef, (id) data);
#endif
    return self;
}

void liftingView_update(id self, int count, int index) {
    LiftViewData *ptr = (LiftViewData *) object_getIvar(self, LiftViewDataRef);
    for (int i = 0; i < 4; ++i) {
        replaceDataSetEntries(ptr->model->dataSets[i], ptr->model->dataArrays[index][i], count);
    }
    updateChart(ptr->chart, ptr->model->chartData, ptr->model->maxes[index]);
}
