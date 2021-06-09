//
//  HistoryGradientChartView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryGradientChartView.h"
#import "ChartHelpers.h"

#define SmallDataSetCutoff 7

@interface HistoryGradientChartView() {
    int currentLegendWidth;
    HistoryGradientChartViewModel *viewModel;
    LineChartView *chartView;
    LineChartData *chartData;
    LineChartDataSet *dataSet;
    ChartLimitLine *limitLines[2];
    NSArray<ChartLegendEntry*> *legendEntries;
}

@end

@implementation HistoryGradientChartView

- (id) initWithViewModel: (HistoryGradientChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;

    UIColor *colors[] = {UIColor.systemGreenColor, UIColor.systemTealColor};

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CFArrayRef colorsArray = CFArrayCreate(NULL, (const void **)(CGColorRef []){
        UIColor.systemRedColor.CGColor, UIColor.clearColor.CGColor}, 2, &kCFTypeArrayCallBacks);
    CGGradientRef chartGradient = CGGradientCreateWithColors(colorSpace, colorsArray, (CGFloat []){0.8, 0});
    ChartLinearGradientFill *fill = [[ChartLinearGradientFill alloc] initWithGradient:chartGradient angle:90];

    legendEntries = @[createLegendEntry(@"", colors[0]), createLegendEntry(@"", colors[1])];

    memcpy(limitLines,
           (ChartLimitLine *[]){[[ChartLimitLine alloc] initWithLimit:0], [[ChartLimitLine alloc] initWithLimit:0]},
           2 * sizeof(UIColor *));
    limitLines[0].lineDashLengths = @[@5, @5];
    limitLines[0].lineColor = colors[0];
    limitLines[1].lineColor = colors[1];

    dataSet = [[LineChartDataSet alloc] initWithEntries:@[]];
    dataSet.fill = fill;
    dataSet.drawFilledEnabled = true;
    dataSet.fillAlpha = 0.75;
    dataSet.valueFont = [UIFont boldSystemFontOfSize:11];
    dataSet.valueTextColor = UIColor.labelColor;
    dataSet.colors = @[UIColor.systemRedColor];
    [dataSet setCircleColor:UIColor.systemRedColor];
    dataSet.circleHoleColor = UIColor.systemRedColor;
    dataSet.circleRadius = 2;

    chartData = [[LineChartData alloc] initWithDataSet:dataSet];

    CFRelease(colorSpace);
    CFRelease(chartGradient);
    CFRelease(colorsArray);
    [fill release];

    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [chartView release];
    [chartData release];
    [dataSet release];
    for (int i = 0; i < 2; ++i) { [limitLines[i] release]; }
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No data is available";

    chartView.leftAxis.axisMinimum = 0;
    chartView.rightAxis.enabled = false;
    [chartView.leftAxis addLimitLine:limitLines[0]];
    [chartView.leftAxis addLimitLine:limitLines[1]];

    chartView.legend.horizontalAlignment = ChartLegendHorizontalAlignmentCenter;
    chartView.legend.verticalAlignment = ChartLegendVerticalAlignmentBottom;
    chartView.legend.orientation = ChartLegendOrientationVertical;
    chartView.legend.yEntrySpace = 10;
    chartView.legend.font = [UIFont systemFontOfSize:16];
    [chartView.legend setCustomWithEntries:legendEntries];
    chartView.legend.enabled = false;

    chartView.xAxis.labelPosition = XAxisLabelPositionBottom;
    chartView.xAxis.drawLabelsEnabled = true;
    chartView.xAxis.drawGridLinesEnabled = true;
    chartView.xAxis.gridLineWidth = 1.5;
    chartView.xAxis.labelFont = [UIFont systemFontOfSize:12];
    chartView.xAxis.granularityEnabled = true;
    chartView.xAxis.granularity = 1;
    chartView.xAxis.avoidFirstLastClippingEnabled = true;
    chartView.xAxis.labelRotationAngle = 45;
    chartView.xAxis.valueFormatter = sharedHistoryXAxisFormatter;

    [self addSubview:chartView];
    [NSLayoutConstraint activateConstraints:@[
        [chartView.topAnchor constraintEqualToAnchor:self.topAnchor],
        [chartView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [chartView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [chartView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [chartView.heightAnchor constraintEqualToConstant:390]
    ]];
}

- (void) layoutSubviews {
    int width = self.bounds.size.width;
    if (width && width != currentLegendWidth) {
        currentLegendWidth = width - 16;
        chartView.legend.xEntrySpace = currentLegendWidth;
    }
}

- (void) updateChart: (bool)shouldAnimate {
    int entryCount = array_size(viewModel->entries);
    if (!entryCount) {
        chartView.legend.enabled = false;
        chartView.data = nil;
        [chartView notifyDataSetChanged];
        return;
    }

    bool isSmallDataSet = entryCount < SmallDataSetCutoff;
    dataSet.mode = isSmallDataSet ? LineChartModeLinear : LineChartModeCubicBezier;
    dataSet.drawCirclesEnabled = isSmallDataSet;
    [dataSet replaceEntries:[NSArray arrayWithObjects:viewModel->entries->arr count:entryCount]];
    [chartData setDrawValues:isSmallDataSet];

    double axisMax = 0;
    historyGradientChartViewModel_updateLeftAxisData(viewModel, limitLines, legendEntries, &axisMax);
    chartView.leftAxis.axisMaximum = axisMax;
    chartView.xAxis.forceLabelsEnabled = isSmallDataSet;
    chartView.xAxis.labelCount = isSmallDataSet ? entryCount : (SmallDataSetCutoff - 1);
    chartView.legend.enabled = true;

    chartView.data = chartData;
    [chartView.data notifyDataChanged];
    [chartView notifyDataSetChanged];

    if (shouldAnimate) {
        [chartView animateWithXAxisDuration:isSmallDataSet ? 1.5 : 2.5];
    }
}

@end
