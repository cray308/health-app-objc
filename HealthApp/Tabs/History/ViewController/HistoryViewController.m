//
//  HistoryViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryViewController.h"
#import "HistoryViewModel.h"
#import "HistoryTabCoordinator.h"
#import "AppCoordinator.h"
#import "Divider.h"
#import "ChartHelpers.h"

#define SmallDataSetCutoff 7

ChartLegendEntry *createLegendEntry(UIColor *color) {
    ChartLegendEntry *entry = [[ChartLegendEntry alloc] initWithLabel:@""];
    entry.formColor = color;
    entry.formLineDashPhase = NAN;
    entry.formSize = 20;
    entry.labelColor = UIColor.labelColor;
    return entry;
}

void disableLineChartView(LineChartView *v) {
    v.legend.enabled = false;
    v.data = nil;
    [v notifyDataSetChanged];
}

@interface ChartSeparatorView: UIView
- (id) initWithTitle: (NSString *)title;
@end

@interface TotalWorkoutsChartView: UIView
- (id) initWithViewModel: (HistoryGradientChartViewModel *)model;
- (void) updateChart;
@end

@interface DurationAreaChartView: UIView<ChartAxisValueFormatter, ChartValueFormatter>
- (id) initWithViewModel: (HistoryAreaChartViewModel *)model;
- (void) updateChart;
@end

@interface LiftChartView: UIView
- (id) initWithViewModel: (HistoryLiftChartViewModel *)model;
- (void) updateChart;
@end

@interface ChartSeparatorView()
@end

@interface TotalWorkoutsChartView() {
    HistoryGradientChartViewModel *viewModel;
    @public LineChartView *chartView;
    LineChartData *chartData;
    LineChartDataSet *dataSet;
    ChartLimitLine *limitLine;
    NSArray<ChartLegendEntry*> *legendEntries;
}
@end

@interface DurationAreaChartView() {
    HistoryAreaChartViewModel *viewModel;
    @public LineChartView *chartView;
    LineChartData *chartData;
    LineChartDataSet *dataSets[5];
    NSArray<ChartLegendEntry*> *legendEntries;
    NSDateComponentsFormatter *yAxisFormatter;
}
@end

@interface LiftChartView() {
    HistoryLiftChartViewModel *viewModel;
    @public LineChartView *chartView;
    LineChartData *chartData;
    LineChartDataSet *dataSets[4];
    NSArray<ChartLegendEntry *> *legendEntries;
}
@end

@interface HistoryViewController() {
    HistoryViewModel *viewModel;
    UISegmentedControl *rangePicker;
    TotalWorkoutsChartView *gradientChart;
    DurationAreaChartView *areaChart;
    LiftChartView *liftChart;
}
@end

@implementation HistoryViewController
- (id) initWithViewModel: (HistoryViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [rangePicker release];
    [gradientChart release];
    [areaChart release];
    [liftChart release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemBackgroundColor;
    self.navigationItem.title = @"Workout History";
    [self setupSubviews];
    appCoordinator_setTabToLoaded(viewModel->delegate->delegate, LoadedViewController_History);
    historyViewModel_fetchData(viewModel);
    historyViewModel_formatDataForTimeRange(viewModel, 0);
    [self updateCharts];
}

- (void) setupSubviews {
    rangePicker = [[UISegmentedControl alloc] initWithItems:@[@"6 Months", @"1 Year", @"2 Years"]];
    rangePicker.layer.cornerRadius = 5;
    rangePicker.tintColor = UIColor.systemGray2Color;
    rangePicker.selectedSegmentIndex = 0;
    [rangePicker addTarget:self action:@selector(updateSelectedSegment:) forControlEvents:UIControlEventValueChanged];

    ChartSeparatorView *gradientChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Workouts Each Week"];
    gradientChart = [[TotalWorkoutsChartView alloc] initWithViewModel:viewModel->gradientChartViewModel];
    ChartSeparatorView *areaChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Activity Time Each Week (By Type)"];
    areaChart = [[DurationAreaChartView alloc] initWithViewModel:viewModel->areaChartViewModel];
    ChartSeparatorView *liftChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Lift Progress"];
    liftChart = [[LiftChartView alloc] initWithViewModel:viewModel->liftChartViewModel];

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[
        rangePicker, gradientChartSeparator, gradientChart, areaChartSeparator, areaChart, liftChartSeparator, liftChart
    ]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 5;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = UIEdgeInsetsMake(10, 8, 10, 8);

    UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:CGRectZero];
    scrollView.translatesAutoresizingMaskIntoConstraints = false;
    scrollView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    scrollView.bounces = true;
    scrollView.showsVerticalScrollIndicator = true;
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:rangePicker];

    [NSLayoutConstraint activateConstraints:@[
        [scrollView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [rangePicker.heightAnchor constraintEqualToConstant:30]
    ]];

    [vStack release];
    [gradientChartSeparator release];
    [areaChartSeparator release];
    [liftChartSeparator release];
    [scrollView release];
}

- (void) updateSelectedSegment: (UISegmentedControl *)sender {
    historyViewModel_formatDataForTimeRange(viewModel, (int) sender.selectedSegmentIndex);
    [self updateCharts];
}

- (void) performForegroundUpdate {
    rangePicker.selectedSegmentIndex = 0;
    historyViewModel_formatDataForTimeRange(viewModel, 0);
    [self updateCharts];
}

- (void) updateCharts {
    if (!historyViewModel_shouldShowCharts(viewModel)) {
        disableLineChartView(gradientChart->chartView);
        disableLineChartView(areaChart->chartView);
        disableLineChartView(liftChart->chartView);
        return;
    }
    [gradientChart updateChart];
    [areaChart updateChart];
    [liftChart updateChart];
}
@end

#pragma mark - Separator View

@implementation ChartSeparatorView
- (id) initWithTitle: (NSString *)title {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;

    Divider *topDivider = [[Divider alloc] init];
    [self addSubview:topDivider];

    UILabel *chartTitleLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    chartTitleLabel.translatesAutoresizingMaskIntoConstraints = false;
    chartTitleLabel.text = title;
    chartTitleLabel.font = [UIFont systemFontOfSize:20];
    chartTitleLabel.adjustsFontSizeToFitWidth = true;
    chartTitleLabel.textColor = UIColor.labelColor;
    chartTitleLabel.textAlignment = NSTextAlignmentCenter;
    [self addSubview:chartTitleLabel];

    [NSLayoutConstraint activateConstraints:@[
        [topDivider.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [topDivider.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [topDivider.topAnchor constraintEqualToAnchor:self.topAnchor],

        [chartTitleLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [chartTitleLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [chartTitleLabel.topAnchor constraintEqualToAnchor:topDivider.bottomAnchor constant:5],
        [chartTitleLabel.heightAnchor constraintEqualToConstant:40],
        [chartTitleLabel.bottomAnchor constraintEqualToAnchor:self.bottomAnchor]
    ]];
    [topDivider release];
    [chartTitleLabel release];
    return self;
}
@end

#pragma mark - Total Workouts Chart

@implementation TotalWorkoutsChartView
- (id) initWithViewModel: (HistoryGradientChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CFArrayRef colorsArray = CFArrayCreate(NULL, (const void **)(CGColorRef []){
        UIColor.systemRedColor.CGColor, UIColor.clearColor.CGColor}, 2, &kCFTypeArrayCallBacks);
    CGGradientRef chartGradient = CGGradientCreateWithColors(colorSpace, colorsArray, (CGFloat []){0.8, 0});
    ChartLinearGradientFill *fill = [[ChartLinearGradientFill alloc] initWithGradient:chartGradient angle:90];

    ChartLegendEntry *entry = createLegendEntry(UIColor.systemTealColor);
    legendEntries = @[entry];
    limitLine = [[ChartLimitLine alloc] initWithLimit:0];
    limitLine.lineDashLengths = @[@5, @5];
    limitLine.lineColor = UIColor.systemTealColor;

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
    dataSet.mode = LineChartModeLinear;

    chartData = [[LineChartData alloc] initWithDataSet:dataSet];

    CFRelease(colorSpace);
    CFRelease(chartGradient);
    CFRelease(colorsArray);
    [fill release];
    [entry release];

    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [chartView release];
    [chartData release];
    [dataSet release];
    [limitLine release];
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No data is available";

    chartView.leftAxis.axisMinimum = 0;
    chartView.rightAxis.enabled = false;
    [chartView.leftAxis addLimitLine:limitLine];

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
    if (width && width != chartView.legend.xEntrySpace + 16) {
        chartView.legend.xEntrySpace = width - 16;
    }
}

- (void) updateChart {
    historyViewModel_applyUpdatesForTotalWorkouts(viewModel, chartView, chartData, dataSet, limitLine, legendEntries);
}
@end

#pragma mark - Duration Area Chart

@implementation DurationAreaChartView
- (id) initWithViewModel: (HistoryAreaChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;
    yAxisFormatter = [[NSDateComponentsFormatter alloc] init];
    yAxisFormatter.allowedUnits = NSCalendarUnitHour | NSCalendarUnitMinute;
    yAxisFormatter.allowsFractionalUnits = true;
    yAxisFormatter.unitsStyle = NSDateComponentsFormatterUnitsStyleAbbreviated;

    UIColor *colors[] = {[UIColor colorNamed:ActivityLevelLowColorName], [UIColor colorNamed:ActivityLevelMediumColorName],
        [UIColor colorNamed:ActivityLevelHighColorName], [UIColor colorNamed:@"historyPieThursday"]};
    ChartLegendEntry *entries[4] = {createLegendEntry(colors[0]), createLegendEntry(colors[1]), createLegendEntry(colors[2]),
        createLegendEntry(colors[3])};

    legendEntries = @[entries[0], entries[1], entries[2], entries[3]];

    dataSets[0] = [[LineChartDataSet alloc] initWithEntries:@[]];
    for (int i = 1; i < 5; ++i) {
        dataSets[i] = [[LineChartDataSet alloc] initWithEntries:@[]];
        dataSets[i].colors = @[colors[i - 1]];
        dataSets[i].fillColor = colors[i - 1];
        dataSets[i].drawFilledEnabled = true;
        dataSets[i].fillAlpha = 0.75;
        dataSets[i].axisDependency = AxisDependencyLeft;
        dataSets[i].valueFont = [UIFont boldSystemFontOfSize:11];
        dataSets[i].valueTextColor = UIColor.labelColor;
        dataSets[i].mode = LineChartModeLinear;
        AreaChartFormatter *fillFormatter = [[AreaChartFormatter alloc] initWithBoundaryDataSet:dataSets[i - 1]];
        dataSets[i].fillFormatter = fillFormatter;
        [dataSets[i] setCircleColor:colors[i - 1]];
        dataSets[i].circleHoleColor = colors[i - 1];
        dataSets[i].circleRadius = 2;
        [fillFormatter release];
        [entries[i - 1] release];
    }
    chartData = [[LineChartData alloc] initWithDataSets:@[dataSets[4], dataSets[3], dataSets[2], dataSets[1]]];
    [chartData setValueFormatter:self];

    [self setupSubviews];
    return self;
}

- (void) dealloc {
    for (int i = 0; i < 5; ++i) { [dataSets[i] release]; }
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
    if (width && width != chartView.legend.xEntrySpace + 16) {
        chartView.legend.xEntrySpace = width - 16;
    }
}

- (void) updateChart {
    historyViewModel_applyUpdatesForDurations(viewModel, chartView, chartData, dataSets, legendEntries);
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

#pragma mark - Lift Line Chart

@implementation LiftChartView
- (id) initWithViewModel: (HistoryLiftChartViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;

    UIColor *colors[] = {[UIColor colorNamed:ActivityLevelLowColorName],
        [UIColor colorNamed:ActivityLevelMediumColorName], [UIColor colorNamed:ActivityLevelHighColorName],
        [UIColor colorNamed:@"historyPieThursday"]
    };

    legendEntries = @[createLegendEntry(colors[0]), createLegendEntry(colors[1]),
                      createLegendEntry(colors[2]), createLegendEntry(colors[3])];

    for (int i = 0; i < 4; ++i) {
        dataSets[i] = [[LineChartDataSet alloc] initWithEntries:@[]];
        dataSets[i].colors = @[colors[i]];
        dataSets[i].axisDependency = AxisDependencyLeft;
        dataSets[i].valueFont = [UIFont boldSystemFontOfSize:11];
        dataSets[i].valueTextColor = UIColor.labelColor;
        dataSets[i].mode = LineChartModeLinear;
        dataSets[i].lineWidth = 2;
        [dataSets[i] setCircleColor:colors[i]];
        dataSets[i].circleHoleColor = colors[i];
        dataSets[i].circleRadius = 2;
    }
    chartData = [[LineChartData alloc] initWithDataSets:@[dataSets[0], dataSets[1], dataSets[2], dataSets[3]]];
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    for (int i = 0; i < 4; ++i) [dataSets[i] release];
    [chartData release];
    [chartView release];
    [super dealloc];
}

- (void) setupSubviews {
    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    chartView.translatesAutoresizingMaskIntoConstraints = false;
    chartView.noDataText = @"No data is available";

    chartView.leftAxis.axisMinimum = 0;
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
    if (width && width != chartView.legend.xEntrySpace + 16) {
        chartView.legend.xEntrySpace = width - 16;
    }
}

- (void) updateChart {
    historyViewModel_applyUpdatesForLifts(viewModel, chartView, chartData, dataSets, legendEntries);
}
@end
