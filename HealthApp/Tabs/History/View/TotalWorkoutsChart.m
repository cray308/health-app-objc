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
               formatter: (id<ChartAxisValueFormatter>) xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self->viewModel = viewModel;

    CGColorRef colorVals[] = {UIColor.systemRedColor.CGColor, UIColor.clearColor.CGColor};
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CFArrayRef colorsArray = CFArrayCreate(NULL, (const void **) colorVals, 2,
                                           &kCFTypeArrayCallBacks);
    CGGradientRef chartGradient = CGGradientCreateWithColors(colorSpace, colorsArray,
                                                             (CGFloat[]){0.8, 0});
    ChartLinearGradientFill *fill = [[ChartLinearGradientFill alloc] initWithGradient:chartGradient
                                                                                angle:90];

    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    valueFormatter = [[ChartDefaultValueFormatter alloc] initWithFormatter:formatter];

    limitLine = [[ChartLimitLine alloc] initWithLimit:0];
    CFNumberRef lineDashLengths[] = {
        CFNumberCreate(NULL, kCFNumberDoubleType, &(double){5}),
        CFNumberCreate(NULL, kCFNumberDoubleType, &(double){5})
    };
    CFArrayRef lineArr = CFArrayCreate(NULL, (const void **)lineDashLengths,
                                       2, &kCFTypeArrayCallBacks);
    limitLine.lineDashLengths = (__bridge NSArray*)lineArr;
    limitLine.lineColor = ((ChartLegendEntry*) viewModel->legendEntries[0]).formColor;

    LineChartDataSet *dataSet = viewModel->dataSet;
    dataSet.fill = fill;
    dataSet.drawFilledEnabled = true;
    dataSet.fillAlpha = 0.75;

    chartView = createChartView(self, xAxisFormatter, viewModel->legendEntries, 1, 390);
    [chartView.leftAxis addLimitLine:limitLine];

    CFRelease(colorSpace);
    CFRelease(chartGradient);
    CFRelease(colorsArray);
    CFRelease(lineArr);
    CFRelease(lineDashLengths[0]);
    CFRelease(lineDashLengths[1]);
    [fill release];
    [formatter release];
    return self;
}

- (void) updateWithCount: (int)count isSmall: (bool)isSmall {
    limitLine.limit = viewModel->avgWorkouts;
    updateDataSet(isSmall, count, viewModel->dataSet, viewModel->entries->arr);
    updateChart(isSmall, count, chartView, viewModel->chartData, viewModel->yMax);
    [((LineChartData *) viewModel->chartData) setValueFormatter:valueFormatter];
}
@end
