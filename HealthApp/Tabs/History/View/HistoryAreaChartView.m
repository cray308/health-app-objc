//
//  HistoryAreaChartView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryAreaChartView.h"
#import "ChartHelpers.h"

#define SmallDataSetCutoff 7

@interface HistoryAreaChartView() {
    int currentLegendWidth;
    HistoryAreaChartViewModel *viewModel;
    LineChartView *chartView;
    LineChartData *chartData;
    LineChartDataSet *dataSets[4];
    NSArray<ChartLegendEntry*> *legendEntries;
    NSDateComponentsFormatter *yAxisFormatter;
}

@end

@implementation HistoryAreaChartView

- (id) initWithViewModel: (HistoryAreaChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;
    yAxisFormatter = [[NSDateComponentsFormatter alloc] init];
    yAxisFormatter.allowedUnits = NSCalendarUnitHour | NSCalendarUnitMinute;
    yAxisFormatter.allowsFractionalUnits = true;
    yAxisFormatter.unitsStyle = NSDateComponentsFormatterUnitsStyleAbbreviated;

    UIColor *colors[] = {[UIColor colorNamed:ActivityLevelLowColorName],
        [UIColor colorNamed:ActivityLevelMediumColorName], [UIColor colorNamed:ActivityLevelHighColorName]};

    legendEntries = @[createLegendEntry(@"", colors[0]), createLegendEntry(@"", colors[1]),
                      createLegendEntry(@"", colors[2])];

    dataSets[0] = [[LineChartDataSet alloc] initWithEntries:@[]];
    for (int i = 1; i < 4; ++i) {
        dataSets[i] = [[LineChartDataSet alloc] initWithEntries:@[]];
        dataSets[i].colors = @[colors[i - 1]];
        dataSets[i].fillColor = colors[i - 1];
        dataSets[i].drawFilledEnabled = true;
        dataSets[i].fillAlpha = 0.75;
        dataSets[i].axisDependency = AxisDependencyLeft;
        dataSets[i].valueFont = [UIFont boldSystemFontOfSize:11];
        dataSets[i].valueTextColor = UIColor.labelColor;
        dataSets[i].mode = LineChartModeLinear;
        dataSets[i].drawFilledEnabled = true;
        AreaChartFormatter *fillFormatter = [[AreaChartFormatter alloc] initWithBoundaryDataSet:dataSets[i - 1]];
        dataSets[i].fillFormatter = fillFormatter;
        [dataSets[i] setCircleColor:colors[i - 1]];
        dataSets[i].circleHoleColor = colors[i - 1];
        dataSets[i].circleRadius = 2;
        [fillFormatter release];
    }
    chartData = [[LineChartData alloc] initWithDataSets:@[dataSets[3], dataSets[2], dataSets[1]]];
    [chartData setValueFormatter:self];

    [self setupSubviews];
    return self;
}

- (void) dealloc {
    for (int i = 0; i < 4; ++i) { [dataSets[i] release]; }
    [yAxisFormatter release];
    [chartData release];
    [chartView release];
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No data is available";

    chartView.leftAxis.axisMinimum = 0;
    chartView.leftAxis.valueFormatter = self;
    chartView.rightAxis.enabled = false;

    chartView.legend.horizontalAlignment = ChartLegendHorizontalAlignmentCenter;
    chartView.legend.verticalAlignment = ChartLegendVerticalAlignmentBottom;
    chartView.legend.orientation = ChartLegendOrientationVertical;

    chartView.legend.yEntrySpace = 10;
    chartView.legend.font = [UIFont systemFontOfSize:16];
    chartView.legend.textColor = UIColor.labelColor;
    [chartView.legend setCustomWithEntries:legendEntries];
    chartView.legend.enabled = false;

    chartView.xAxis.labelPosition = XAxisLabelPositionBottom;
    chartView.xAxis.gridLineWidth = 1.5;
    chartView.xAxis.labelFont = [UIFont systemFontOfSize:12];
    chartView.xAxis.granularityEnabled = true;
    chartView.xAxis.avoidFirstLastClippingEnabled = true;
    chartView.xAxis.labelRotationAngle = 45;
    chartView.xAxis.valueFormatter = sharedHistoryXAxisFormatter;

    LineChartRenderer *renderer = [[CustomLineChartRenderer alloc] initWithDataProvider:chartView animator:chartView.chartAnimator viewPortHandler:chartView.viewPortHandler];
    chartView.renderer = renderer;

    [self addSubview:chartView];
    [NSLayoutConstraint activateConstraints:@[
        [chartView.topAnchor constraintEqualToAnchor:self.topAnchor],
        [chartView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [chartView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [chartView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [chartView.heightAnchor constraintEqualToConstant:425]
    ]];

    [renderer release];
}

- (void) layoutSubviews {
    int width = self.bounds.size.width;
    if (width && width != currentLegendWidth) {
        currentLegendWidth = width - 16;
        chartView.legend.xEntrySpace = currentLegendWidth;
    }
}

- (void) updateChart: (bool)shouldAnimate {
    int entryCount = array_size(viewModel->entries[0]);
    if (!entryCount) {
        chartView.legend.enabled = false;
        chartView.data = nil;
        [chartView notifyDataSetChanged];
        return;
    }

    bool isSmallDataSet = entryCount < SmallDataSetCutoff;
    [dataSets[0] replaceEntries:[NSArray arrayWithObjects:viewModel->entries[0]->arr count:entryCount]];
    for (int i = 1; i < 4; ++i) {
        dataSets[i].drawCirclesEnabled = isSmallDataSet; // don't draw circles for larger datasets
        [dataSets[i] replaceEntries:[NSArray arrayWithObjects:viewModel->entries[i]->arr count:entryCount]];
    }
    [chartData setDrawValues:isSmallDataSet];

    chartView.leftAxis.axisMaximum = 1.1 * viewModel->maxActivityTime;
    chartView.xAxis.forceLabelsEnabled = isSmallDataSet;
    chartView.xAxis.labelCount = isSmallDataSet ? entryCount : (SmallDataSetCutoff - 1);

    historyAreaChartViewModel_updateLegend(viewModel, legendEntries);
    chartView.legend.enabled = true;

    chartView.data = chartData;
    [chartView.data notifyDataChanged];
    [chartView notifyDataSetChanged];

    if (shouldAnimate) {
        [chartView animateWithXAxisDuration:isSmallDataSet ? 1.5 : 2.5];
    }
}

- (NSString * _Nonnull) stringForValue: (double)value axis: (ChartAxisBase * _Nullable)axis {
    return [yAxisFormatter stringFromTimeInterval:60 * value];
}

- (NSString * _Nonnull) stringForValue: (double)value entry: (ChartDataEntry * _Nonnull)entry
                          dataSetIndex: (NSInteger)dataSetIndex
                       viewPortHandler: (ChartViewPortHandler * _Nullable)viewPortHandler {
    return [yAxisFormatter stringFromTimeInterval:60 * value];
}

@end
