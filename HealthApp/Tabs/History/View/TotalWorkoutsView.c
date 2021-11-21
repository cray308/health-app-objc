#include "TotalWorkoutsView.h"
#include "SwiftBridging.h"

extern void setLineLimit(id v, float limit);

Class TotalWorkoutsViewClass;
Ivar TotalWorkoutsViewDataRef;

id totalWorkoutsView_init(TotalWorkoutsChartViewModel *model, id formatter) {
    id self = createObjectWithFrame(TotalWorkoutsViewClass, CGRectZero);
    TotalWorkoutsViewData *data = malloc(sizeof(TotalWorkoutsViewData));
    data->model = model;

    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)model->legendEntries, 1,
                                         &(CFArrayCallBacks){0});
    data->chart = createChartView(formatter, legendArr, 1);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 390);
    object_setIvar(self, TotalWorkoutsViewDataRef, (id) data);
    CFRelease(legendArr);
    return self;
}

void totalWorkoutsView_update(id self, int count, bool isSmall) {
    TotalWorkoutsViewData *ptr =
    (TotalWorkoutsViewData *) object_getIvar(self, TotalWorkoutsViewDataRef);
    setLineLimit(ptr->chart, ptr->model->avgWorkouts);
    updateDataSet(isSmall, count, ptr->model->dataSet, ptr->model->entries->arr);
    updateChart(isSmall, ptr->chart, ptr->model->chartData, ptr->model->yMax);
}
