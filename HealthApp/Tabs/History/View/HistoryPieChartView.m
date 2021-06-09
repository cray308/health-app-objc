//
//  HistoryPieChartView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryPieChartView.h"
#import "ChartHelpers.h"

@interface HistoryPieChartView() {
    int currentLegendWidth;
    HistoryPieChartViewModel *viewModel;
    PieChartView *chartView;
    ChartDefaultValueFormatter *valueFormatter;
    PieChartData *chartData;
    PieChartDataSet *dataSet;
    NSArray<ChartLegendEntry *> *legendEntries;
}

@end

@implementation HistoryPieChartView

- (id) initWithViewModel: (HistoryPieChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;

    NSNumberFormatter *tempFormatter = [[NSNumberFormatter alloc] init];
    tempFormatter.numberStyle = NSNumberFormatterPercentStyle;
    tempFormatter.maximumFractionDigits = 1;
    tempFormatter.multiplier = @100.0;
    tempFormatter.zeroSymbol = @"";
    valueFormatter = [[ChartDefaultValueFormatter alloc] initWithFormatter:tempFormatter];
    [tempFormatter release];

    NSArray<UIColor*> *colors = @[[UIColor colorNamed:@"historyPieSunday"], [UIColor colorNamed:@"historyPieMonday"],
                                  [UIColor colorNamed:@"historyPieTuesday"], [UIColor colorNamed:@"historyPieWednesday"],
                                  [UIColor colorNamed:@"historyPieThursday"], [UIColor colorNamed:@"historyPieFriday"],
                                  [UIColor colorNamed:@"historyPieSaturday"]];
    legendEntries = @[createLegendEntry(@"", colors[0]), createLegendEntry(@"", colors[1]),
                      createLegendEntry(@"", colors[2]), createLegendEntry(@"", colors[3]),
                      createLegendEntry(@"", colors[4]), createLegendEntry(@"", colors[5]),
                      createLegendEntry(@"", colors[6])];

    dataSet = [[PieChartDataSet alloc] initWithEntries:[NSArray arrayWithObjects:viewModel->entries count:7]];
    dataSet.colors = colors;
    dataSet.valueFont = [UIFont systemFontOfSize:16];
    dataSet.valueTextColor = UIColor.labelColor;

    chartData = [[PieChartData alloc] initWithDataSet:dataSet];

    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [valueFormatter release];
    [dataSet release];
    [chartData release];
    [chartView release];
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[PieChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No data is available";
    chartView.legend.horizontalAlignment = ChartLegendHorizontalAlignmentCenter;
    chartView.legend.verticalAlignment = ChartLegendVerticalAlignmentBottom;
    chartView.legend.orientation = ChartLegendOrientationVertical;
    chartView.legend.yEntrySpace = 10;
    chartView.legend.font = [UIFont systemFontOfSize:16];
    [chartView.legend setCustomWithEntries:legendEntries];
    chartView.legend.enabled = false;
    [self addSubview:chartView];

    [NSLayoutConstraint activateConstraints:@[
        [chartView.topAnchor constraintEqualToAnchor:self.topAnchor],
        [chartView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [chartView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [chartView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [chartView.heightAnchor constraintEqualToConstant:550]
    ]];
}

- (void) layoutSubviews {
    int width = self.bounds.size.width;
    if (width && width != currentLegendWidth) {
        currentLegendWidth = width - 16;
        chartView.legend.xEntrySpace = currentLegendWidth;
    }
}

- (void) updateChart {
    if (!viewModel->activitySum) {
        chartView.legend.enabled = false;
        chartView.data = nil;
        [chartView notifyDataSetChanged];
        return;
    }

    [dataSet replaceEntries: [NSArray arrayWithObjects:viewModel->entries count:7]];
    historyPieChartViewModel_updateLegend(viewModel, legendEntries);
    chartView.legend.enabled = true;
    chartView.data = chartData;
    [chartView.data setValueFormatter:valueFormatter];
}

@end
