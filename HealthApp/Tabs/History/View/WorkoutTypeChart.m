//
//  WorkoutTypeChart.m
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#import "WorkoutTypeChart.h"
#include "SwiftBridging.h"

id workoutTypeChart_init(WorkoutTypeChartViewModel *model, id formatter) {
    WorkoutTypeChart *this = [[WorkoutTypeChart alloc] initWithFrame:CGRectZero];
    this->model = model;

    for (int i = 1; i < 5; ++i) {
        LineChartDataSet *dataSet = model->dataSets[i];
        dataSet.fillColor = ((LegendEntry*) model->legendEntries[i - 1]).formColor;
        dataSet.drawFilledEnabled = true;
        dataSet.fillAlpha = 0.75;
        AreaChartFormatter *f = [[AreaChartFormatter alloc] initWithDataSet:model->dataSets[i - 1]];
        dataSet.fillFormatter = f;
        [f release];
    }
    [((LineChartData*) model->chartData) setValueFormatter:this];

    this->chartView = createChartView(this, formatter, model->legendEntries, 4, 425);
    this->chartView.leftAxis.valueFormatter = this;

    ChartRenderer *renderer = [[AreaChartRenderer alloc] initWithView:this->chartView];
    this->chartView.renderer = renderer;
    [renderer release];
    return this;
}

void workoutTypeChart_update(WorkoutTypeChart *this, int count, bool isSmall) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)this->model->entries[0]->arr,
                                     count, &kCocoaArrCallbacks);
    [((LineChartDataSet *) this->model->dataSets[0]) replaceEntries:_nsarr(array)];
    for (int i = 1; i < 5; ++i) {
        updateDataSet(isSmall, count, this->model->dataSets[i], this->model->entries[i]->arr);
    }
    updateChart(isSmall, this->chartView, this->model->chartData, this->model->yMax);
    CFRelease(array);
}

@implementation WorkoutTypeChart
- (NSString *) stringForValue: (double)value axis: (AxisBase *)axis {
    return _nsstr(workoutTypeViewModel_getDuration(model, value));
}

- (NSString *) stringForValue: (double)value entry: (ChartDataEntry *)entry
                 dataSetIndex: (NSInteger)dataSetIndex
              viewPortHandler: (ViewPortHandler *)viewPortHandler {
    return _nsstr(workoutTypeViewModel_getDuration(model, value));
}
@end
