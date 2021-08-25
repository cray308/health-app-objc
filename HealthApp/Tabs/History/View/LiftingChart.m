//
//  LiftingChart.m
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#import "LiftingChart.h"
#include "SwiftBridging.h"

@implementation LiftingChart
- (id) initWithViewModel: (HistoryLiftChartViewModel *)viewModel
               formatter: (id<ChartAxisValueFormatter>) xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self->viewModel = viewModel;
    for (int i = 0; i < 4; ++i)
        ((LineChartDataSet *) viewModel->dataSets[i]).lineWidth = 2;
    chartView = createChartView(self, xAxisFormatter, viewModel->legendEntries, 4, 550);
    return self;
}

- (void) updateWithCount: (int)count isSmall: (bool)isSmall {
    for (int i = 0; i < 4; ++i) {
        updateDataSet(isSmall, count, viewModel->dataSets[i], viewModel->entries[i]->arr);
    }
    updateChart(isSmall, count, chartView, viewModel->chartData, viewModel->yMax);
}
@end
