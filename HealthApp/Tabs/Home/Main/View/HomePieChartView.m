//
//  HomePieChartView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomePieChartView.h"
#import "ChartHelpers.h"
#import "StringHelpers.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

@interface HomePieChartView() {
    int currentLegendWidth;
    HomePieChartViewModel *viewModel;
    PieChartView *chartView;
    ChartDefaultValueFormatter *valueFormatter;
    PieChartData *chartData;
    PieChartData *emptyData;
    PieChartDataSet *dataSet;
    PieChartDataSet *emptyDataSet;
    NSArray<ChartLegendEntry *> *legendEntries;
}

@end

@implementation HomePieChartView

- (id) initWithViewModel: (HomePieChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;
    
    NSNumberFormatter *tempFormatter = [[NSNumberFormatter alloc] init];
    tempFormatter.numberStyle = NSNumberFormatterPercentStyle;
    tempFormatter.maximumFractionDigits = 1;
    tempFormatter.multiplier = @100.0;
    tempFormatter.zeroSymbol = @"";
    valueFormatter = [[ChartDefaultValueFormatter alloc] initWithFormatter:tempFormatter];
    [tempFormatter release];

    NSArray<UIColor*> *colors = @[[UIColor colorNamed:ActivityLevelLowColorName],
                                  [UIColor colorNamed:ActivityLevelMediumColorName],
                                  [UIColor colorNamed:ActivityLevelHighColorName]];
    legendEntries = @[createLegendEntry(@"", colors[0]), createLegendEntry(@"", colors[1]),
                      createLegendEntry(@"", colors[2])];

    dataSet = [[PieChartDataSet alloc] initWithEntries:[NSArray arrayWithObjects:viewModel->entries count:3]];
    dataSet.colors = [NSArray arrayWithArray:colors];
    dataSet.valueFont = [UIFont systemFontOfSize:16];
    dataSet.valueTextColor = UIColor.labelColor;

    PieChartDataEntry *emptyEntry = [[PieChartDataEntry alloc] initWithValue:1 label:@"No Data"];
    emptyDataSet = [[PieChartDataSet alloc] initWithEntries:@[emptyEntry]];
    emptyDataSet.colors = @[UIColor.systemGrayColor];
    emptyDataSet.drawValuesEnabled = false;

    chartData = [[PieChartData alloc] initWithDataSet:dataSet];
    emptyData = [[PieChartData alloc] initWithDataSet:emptyDataSet];

    [self setupSubviews];
    [emptyEntry release];
    return self;
}

- (void) dealloc {
    [valueFormatter release];
    [dataSet release];
    [emptyDataSet release];
    [chartData release];
    [emptyData release];
    [chartView release];
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[PieChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No activity is available for this week";
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
        [chartView.heightAnchor constraintEqualToConstant:450]
    ]];
    chartView.data = chartData;
}

- (void) layoutSubviews {
    int width = self.bounds.size.width;
    if (width && width != currentLegendWidth) {
        currentLegendWidth = width - 16;
        chartView.legend.xEntrySpace = currentLegendWidth;
    }
}

- (void) updateChart {
    if (!viewModel->totalMinutes) {
        chartView.legend.enabled = false;
        chartView.data = emptyData;
        chartView.drawEntryLabelsEnabled = true;
        return;
    }

    [dataSet replaceEntries:[NSArray arrayWithObjects:viewModel->entries count:3]];
    homePieChartViewModel_updateLegend(viewModel, legendEntries);
    chartView.legend.enabled = true;
    chartView.drawEntryLabelsEnabled = false;
    chartView.data = chartData;
    [chartView.data setValueFormatter:valueFormatter];
}

@end
