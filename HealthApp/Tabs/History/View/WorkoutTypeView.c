#include "WorkoutTypeView.h"
#include "SwiftBridging.h"

Class WorkoutTypeClass;
Ivar WorkoutTypeDataRef;

id workoutTypeView_init(WorkoutTypeChartViewModel *model, id formatter) {
    id self = createObjectWithFrame(WorkoutTypeClass, CGRectZero);
    WorkoutTypeData *data = malloc(sizeof(WorkoutTypeData));
    data->model = model;

    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)model->legendEntries, 4,
                                         &(CFArrayCallBacks){0});
    data->chart = createChartView(formatter, legendArr, 6);
    addSubview(self, data->chart);
    pin(data->chart, self, (Padding){0, 8, 0, 8}, 0);
    setHeight(data->chart, 425);
    object_setIvar(self, WorkoutTypeDataRef, (id) data);
    CFRelease(legendArr);
    return self;
}

void workoutTypeView_update(id self, int count, bool isSmall) {
    WorkoutTypeData *ptr = (WorkoutTypeData *) object_getIvar(self, WorkoutTypeDataRef);
    replaceDataSetEntries(ptr->model->dataSets[0], ptr->model->entries[0]->arr, count);
    for (int i = 1; i < 5; ++i) {
        updateDataSet(isSmall, count, ptr->model->dataSets[i], ptr->model->entries[i]->arr);
    }
    updateChart(isSmall, ptr->chart, ptr->model->chartData, ptr->model->yMax);
}
