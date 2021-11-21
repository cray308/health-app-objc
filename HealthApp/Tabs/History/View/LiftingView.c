#include "LiftingView.h"
#include "SwiftBridging.h"

Class LiftViewClass;
Ivar LiftViewDataRef;

id liftingView_init(LiftChartViewModel *model, id formatter) {
    id self = createObjectWithFrame(LiftViewClass, CGRectZero);
    LiftViewData *data = malloc(sizeof(LiftViewData));
    data->model = model;

    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)model->legendEntries, 4,
                                         &(CFArrayCallBacks){0});
    data->chart = createChartView(formatter, legendArr, 0);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 550);
    object_setIvar(self, LiftViewDataRef, (id) data);
    CFRelease(legendArr);
    return self;
}

void liftingView_update(id self, int count, bool isSmall) {
    LiftViewData *ptr = (LiftViewData *) object_getIvar(self, LiftViewDataRef);
    for (int i = 0; i < 4; ++i) {
        updateDataSet(isSmall, count, ptr->model->dataSets[i], ptr->model->entries[i]->arr);
    }
    updateChart(isSmall, ptr->chart, ptr->model->chartData, ptr->model->yMax);
}
