//
//  HomeBarChartView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeBarChartView.h"
#import "CalendarDateHelpers.h"

@interface HomeBarChartView() {
    HomeBarChartViewModel *viewModel;
    BarChartView *chartView;
    BarChartDataSet *dataSet;
    BarChartData *chartData;
    ChartDefaultValueFormatter *valueFormatter;
    ChartIndexAxisValueFormatter *xAxisFormatter;
    ChartLimitLine *limitLine;
}

@end

@implementation HomeBarChartView

- (id) initWithViewModel: (HomeBarChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;
    NSString **dayNames = calendar_getWeekDaySymbols(true);
    NSArray<NSString*> *xAxisLabels = [[NSArray alloc] initWithObjects:dayNames count:7];
    xAxisFormatter = [[ChartIndexAxisValueFormatter alloc] initWithValues:xAxisLabels];
    [xAxisLabels release];
    for (int i = 0; i < 7; ++i) { [dayNames[i] release]; }
    free(dayNames);
    limitLine = [[ChartLimitLine alloc] initWithLimit:0];
    limitLine.lineColor = UIColor.systemRedColor;

    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    formatter.numberStyle = NSNumberFormatterNoStyle;
    formatter.maximumFractionDigits = 0;
    formatter.multiplier = @1.0;
    valueFormatter = [[ChartDefaultValueFormatter alloc] initWithFormatter:formatter];
    [formatter release]; // ***

    dataSet = [[BarChartDataSet alloc] initWithEntries:@[]];
    dataSet.colors = @[UIColor.systemGreenColor];
    dataSet.valueFont = [UIFont systemFontOfSize:14];
    dataSet.valueTextColor = UIColor.labelColor;

    chartData = [[BarChartData alloc] initWithDataSet:dataSet];
    [chartData setValueFormatter:valueFormatter];

    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [limitLine release];
    [xAxisFormatter release];
    [valueFormatter release];
    [dataSet release];
    [chartData release];
    [chartView release];
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[BarChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No data is available";
    chartView.legend.enabled = false;
    chartView.leftAxis.axisMinimum = 0;
    chartView.rightAxis.enabled = false;
    chartView.xAxis.drawGridLinesEnabled = false;
    chartView.xAxis.labelCount = 7;
    chartView.xAxis.valueFormatter = xAxisFormatter;
    chartView.xAxis.labelPosition = XAxisLabelPositionBottom;
    chartView.xAxis.labelFont = [UIFont systemFontOfSize:14];
    chartView.xAxis.labelTextColor = UIColor.labelColor;
    [self addSubview:chartView];

    [NSLayoutConstraint activateConstraints:@[
        [chartView.topAnchor constraintEqualToAnchor:self.topAnchor],
        [chartView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [chartView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [chartView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [chartView.heightAnchor constraintEqualToConstant:350]
    ]];
    chartView.data = chartData;
    [chartView.leftAxis addLimitLine:limitLine];
}

- (void) updateChart {
    [dataSet replaceEntries:[NSArray arrayWithObjects:viewModel->cumulativeArray count:7]];

    double axisMax = 0;
    homeBarChartViewModel_updateLeftAxisData(viewModel, limitLine, &axisMax);
    chartView.leftAxis.axisMaximum = axisMax;

    chartView.data = chartData;
    [chartView.data notifyDataChanged];
    [chartView notifyDataSetChanged];
}

@end
