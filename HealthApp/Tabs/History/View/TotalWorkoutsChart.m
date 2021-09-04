//
//  TotalWorkoutsChart.m
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#import "TotalWorkoutsChart.h"
#include "SwiftBridging.h"

@implementation TotalWorkoutsChart
- (id) initWithViewModel: (HistoryTotalWorkoutsChartViewModel *)viewModel
               formatter: (id<AxisValueFormatter>)xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self->viewModel = viewModel;

    limitLine = [[ChartLimitLine alloc] initWithLimit:0];
    limitLine.lineColor = ((LegendEntry*) viewModel->legendEntries[0]).formColor;

    LineChartDataSet *dataSet = viewModel->dataSet;
    NSObject<Fill> *fill = [[LinearGradientFill alloc] initWithEndColor:UIColor.systemRedColor];
    dataSet.fill = fill;
    dataSet.drawFilledEnabled = true;
    dataSet.fillAlpha = 0.75;

    chartView = createChartView(self, xAxisFormatter, viewModel->legendEntries, 1, 390);
    [chartView.leftAxis addLimitLine:limitLine];
    [fill release];
    return self;
}

- (void) updateWithCount: (int)count isSmall: (bool)isSmall {
    limitLine.limit = viewModel->avgWorkouts;
    updateDataSet(isSmall, count, viewModel->dataSet, viewModel->entries->arr);
    updateChart(isSmall, count, chartView, viewModel->chartData, viewModel->yMax);
    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    DefaultValueFormatter *valueFormatter = [[DefaultValueFormatter alloc]
                                             initWithFormatter:formatter];
    [((LineChartData *) viewModel->chartData) setValueFormatter:valueFormatter];
    [valueFormatter release];
    [formatter release];
}
@end
