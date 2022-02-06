#include "LiftingView.h"
#include "SwiftBridging.h"

Class LiftViewClass;

id liftingView_init(LiftChartModel *model, id formatter) {
    id self = createNew(LiftViewClass);
    LiftView *data = (LiftView *) ((char *)self + ViewSize);
    data->model = model;
    data->chart = createChartView(formatter, (long []){0, 1, 2, 3}, 4, 0);
    disableAutoresizing(data->chart);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 550, false);
    return self;
}

void liftingView_update(id self, int count, int index, int ref) {
    LiftView *ptr = (LiftView *) ((char *)self + ViewSize);
    for (int i = 0; i < 4; ++i) {
        replaceDataSetEntries(ptr->model->dataSets[i], &ptr->model->entries[i][ref], count);
    }
    updateChart(ptr->chart, ptr->model->chartData, ptr->model->maxes[index]);
}
