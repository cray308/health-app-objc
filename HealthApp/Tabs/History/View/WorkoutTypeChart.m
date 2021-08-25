//
//  WorkoutTypeChart.m
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#import "WorkoutTypeChart.h"
#include "SwiftBridging.h"

@implementation WorkoutTypeChart
- (id) initWithViewModel: (HistoryWorkoutTypeChartViewModel *)viewModel
               formatter: (id<ChartAxisValueFormatter>)xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self->viewModel = viewModel;

    for (int i = 1; i < 5; ++i) {
        LineChartDataSet *dataSet = viewModel->dataSets[i];
        dataSet.fillColor = ((ChartLegendEntry*) viewModel->legendEntries[i - 1]).formColor;
        dataSet.drawFilledEnabled = true;
        dataSet.fillAlpha = 0.75;
        AreaChartFormatter *fillFormatter = [[AreaChartFormatter alloc]
                                             initWithBoundaryDataSet:viewModel->dataSets[i - 1]];
        dataSet.fillFormatter = fillFormatter;
        [fillFormatter release];
    }
    [((LineChartData*) viewModel->chartData) setValueFormatter:self];

    chartView = createChartView(self, xAxisFormatter, viewModel->legendEntries, 4, 425);
    chartView.leftAxis.valueFormatter = self;

    LineChartRenderer *renderer = [[CustomLineChartRenderer alloc]
                                   initWithDataProvider:chartView animator:chartView.chartAnimator
                                   viewPortHandler:chartView.viewPortHandler];
    chartView.renderer = renderer;
    [renderer release];
    return self;
}

- (void) updateWithCount: (int)count isSmall: (bool)isSmall {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)viewModel->entries[0]->arr,
                                     count, &kCocoaArrCallbacks);
    [((LineChartDataSet *) viewModel->dataSets[0]) replaceEntries:(__bridge NSArray*)array];
    for (int i = 1; i < 5; ++i) {
        updateDataSet(isSmall, count, viewModel->dataSets[i], viewModel->entries[i]->arr);
    }
    updateChart(isSmall, count, chartView, viewModel->chartData, viewModel->yMax);
    CFRelease(array);
}

- (NSString * _Nonnull) stringForValue: (double)value axis: (ChartAxisBase * _Nullable)axis {
    return (__bridge NSString*) workoutTypeViewModel_getDuration(viewModel, value);
}

- (NSString * _Nonnull) stringForValue: (double)value entry: (ChartDataEntry * _Nonnull)entry
                          dataSetIndex: (NSInteger)dataSetIndex
                       viewPortHandler: (ChartViewPortHandler * _Nullable)viewPortHandler {
    return (__bridge NSString*) workoutTypeViewModel_getDuration(viewModel, value);
}
@end
