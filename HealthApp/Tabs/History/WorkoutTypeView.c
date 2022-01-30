#include "WorkoutTypeView.h"
#include <stdlib.h>
#include "SwiftBridging.h"

Class WorkoutTypeViewClass;
Ivar WorkoutTypeViewDataRef;

id workoutTypeView_init(WorkoutTypeChartModel *model, id formatter) {
    id self = createNew(WorkoutTypeViewClass);
#ifndef __clang_analyzer__
    WorkoutTypeViewData *data = malloc(sizeof(WorkoutTypeViewData));
    data->model = model;
    data->chart = createChartView(formatter, (long []){0, 1, 2, 3}, 4, 6);
    disableAutoresizing(data->chart);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 425, false);
    object_setIvar(self, WorkoutTypeViewDataRef, (id) data);
#endif
    return self;
}

void workoutTypeView_update(id self, int count, int index) {
    WorkoutTypeViewData *ptr = (WorkoutTypeViewData *) object_getIvar(self, WorkoutTypeViewDataRef);
    replaceDataSetEntries(ptr->model->dataSets[0], ptr->model->dataArrays[index][0], count);
    for (int i = 1; i < 5; ++i) {
        replaceDataSetEntries(ptr->model->dataSets[i], ptr->model->dataArrays[index][i], count);
    }
    updateChart(ptr->chart, ptr->model->chartData, ptr->model->maxes[index]);
}
