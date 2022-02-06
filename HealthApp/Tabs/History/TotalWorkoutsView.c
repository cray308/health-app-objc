#include "TotalWorkoutsView.h"
#include "SwiftBridging.h"

extern void setLineLimit(id v, float limit);

Class TotalWorkoutsViewClass;

id totalWorkoutsView_init(TotalWorkoutsChartModel *model, id formatter) {
    id self = createNew(TotalWorkoutsViewClass);
    TotalWorkoutsView *data = (TotalWorkoutsView *) ((char *)self + ViewSize);
    data->model = model;
    data->chart = createChartView(formatter, (long []){4}, 1, 1);
    disableAutoresizing(data->chart);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 390, false);
    return self;
}

void totalWorkoutsView_update(id self, int count, int index, int ref) {
    TotalWorkoutsView *ptr = (TotalWorkoutsView *) ((char *)self + ViewSize);
    setLineLimit(ptr->chart, ptr->model->avgs[index]);
    replaceDataSetEntries(ptr->model->dataSet, &ptr->model->entries[ref], count);
    updateChart(ptr->chart, ptr->model->chartData, ptr->model->maxes[index]);
}
