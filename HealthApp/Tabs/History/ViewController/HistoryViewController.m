//
//  HistoryViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryViewController.h"
#include "AppCoordinator.h"
#include "ViewControllerHelpers.h"
#include "CalendarDateHelpers.h"

#define SmallDataSetCutoff 7
#define HistoryChartColors { \
    [UIColor colorNamed:@"chartBlue"], [UIColor colorNamed:@"chartGreen"], \
    [UIColor colorNamed:@"chartOrange"], [UIColor colorNamed:@"chartPink"] \
}

ChartLegendEntry *createLegendEntry(UIColor *color) {
    ChartLegendEntry *entry = [[ChartLegendEntry alloc] initWithLabel:@""];
    entry.formColor = color;
    entry.formLineDashPhase = NAN;
    entry.formSize = 20;
    entry.labelColor = UIColor.labelColor;
    return entry;
}

ChartDataEntry **getChartEntriesFromArray(Array_chartData *a) {
    return (ChartDataEntry **) a->arr;
}

void setupChartCommon(LineChartView *v, UIView *parent,
                      NSObject<ChartAxisValueFormatter> *xAxisFormatter,
                      CFArrayRef legendEntries, int height) {
    v.translatesAutoresizingMaskIntoConstraints = false;
    v.noDataText = @"No data is available";
    v.leftAxis.axisMinimum = 0;
    v.rightAxis.enabled = false;

    v.legend.horizontalAlignment = ChartLegendHorizontalAlignmentCenter;
    v.legend.orientation = ChartLegendOrientationVertical;
    v.legend.yEntrySpace = 10;
    v.legend.font = [UIFont systemFontOfSize:16];
    [v.legend setCustomWithEntries:(__bridge NSArray*)legendEntries];
    v.legend.enabled = false;

    v.xAxis.labelPosition = XAxisLabelPositionBottom;
    v.xAxis.gridLineWidth = 1.5;
    v.xAxis.labelFont = [UIFont systemFontOfSize:12];
    v.xAxis.granularityEnabled = true;
    v.xAxis.avoidFirstLastClippingEnabled = true;
    v.xAxis.labelRotationAngle = 45;
    v.xAxis.valueFormatter = xAxisFormatter;

    [parent addSubview:v];
    activateConstraints((id []){
        [v.topAnchor constraintEqualToAnchor:parent.topAnchor],
        [v.bottomAnchor constraintEqualToAnchor:parent.bottomAnchor],
        [v.leadingAnchor constraintEqualToAnchor:parent.leadingAnchor constant:8],
        [v.trailingAnchor constraintEqualToAnchor:parent.trailingAnchor constant:-8],
        [v.heightAnchor constraintEqualToConstant:height]
    }, 5);
}

void setupDataSetCommon(LineChartDataSet *dataSet, UIColor *color) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)((id[]){color}), 1, kCocoaArrCallbacks);
    dataSet.colors = (__bridge NSArray*)arr;
    dataSet.axisDependency = AxisDependencyLeft;
    dataSet.valueFont = [UIFont boldSystemFontOfSize:11];
    dataSet.valueTextColor = UIColor.labelColor;
    [dataSet setCircleColor:color];
    dataSet.circleHoleColor = color;
    dataSet.circleRadius = 2;
    CFRelease(arr);
}

void disableLineChartView(LineChartView *v) {
    v.legend.enabled = false;
    v.data = nil;
    [v notifyDataSetChanged];
}

void updateLineChartLegendWidth(LineChartView *v, int width) {
    if (width && width != v.legend.xEntrySpace + 16) {
        v.legend.xEntrySpace = width - 16;
    }
}

NSString *getDurationStringForAreaChart(CFStringRef *str, int minutes) {
    CFRelease(*str);
    if (!minutes) {
        *str = CFStringCreateCopy(NULL, CFSTR(""));
    } else if (minutes < 60) {
        *str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%dm"), minutes);
    } else {
        *str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%dh %dm"), minutes / 60, minutes % 60);
    }
    return (__bridge NSString*) *str;
}

void updateDataSet(bool isSmall, int count, LineChartDataSet *dataSet, ChartDataEntry **entries,
                   ChartLegendEntry *legendEntry, CFStringRef labelString) {
    dataSet.drawCirclesEnabled = isSmall;
    CFArrayRef array = CFArrayCreate(NULL, (const void **)entries, count, kCocoaArrCallbacks);
    [dataSet replaceEntries:(__bridge NSArray*)array];
    legendEntry.label = (__bridge NSString*) labelString;
    CFRelease(labelString);
    CFRelease(array);
}

void updateChart(bool isSmall, int count,
                 LineChartView *view, LineChartData *data, double axisMax) {
    [data setDrawValues:isSmall];

    view.leftAxis.axisMaximum = axisMax;
    view.xAxis.forceLabelsEnabled = isSmall;
    view.xAxis.labelCount = isSmall ? count : (SmallDataSetCutoff - 1);
    view.legend.enabled = true;

    view.data = data;
    [view.data notifyDataChanged];
    [view notifyDataSetChanged];
    [view animateWithXAxisDuration:isSmall ? 1.5 : 2.5];
}

UIView *createChartSeparator(CFStringRef title) {
    UIView *view = createView(nil, false);

    UIView *topDivider = createDivider();
    [view addSubview:topDivider];

    UILabel *chartTitleLabel = createLabel(title, nil, true, NSTextAlignmentCenter);
    chartTitleLabel.font = [UIFont systemFontOfSize:20];
    [view addSubview:chartTitleLabel];

    activateConstraints((id []){
        [topDivider.leadingAnchor constraintEqualToAnchor:view.leadingAnchor],
        [topDivider.trailingAnchor constraintEqualToAnchor:view.trailingAnchor],
        [topDivider.topAnchor constraintEqualToAnchor:view.topAnchor],

        [chartTitleLabel.leadingAnchor constraintEqualToAnchor:view.leadingAnchor],
        [chartTitleLabel.trailingAnchor constraintEqualToAnchor:view.trailingAnchor],
        [chartTitleLabel.topAnchor constraintEqualToAnchor:topDivider.bottomAnchor constant:5],
        [chartTitleLabel.heightAnchor constraintEqualToConstant:40],
        [chartTitleLabel.bottomAnchor constraintEqualToAnchor:view.bottomAnchor]
    }, 8);
    [topDivider release];
    [chartTitleLabel release];
    return view;
}

@interface TotalWorkoutsChartView: UIView
- (id) initWithFormatter: (NSObject<ChartAxisValueFormatter>*) xAxisFormatter;
@end

@interface DurationAreaChartView: UIView<ChartAxisValueFormatter, ChartValueFormatter>
- (id) initWithFormatter: (NSObject<ChartAxisValueFormatter>*) xAxisFormatter;
@end

@interface LiftChartView: UIView
- (id) initWithFormatter: (NSObject<ChartAxisValueFormatter>*) xAxisFormatter;
@end

@interface TotalWorkoutsChartView() {
    @public ChartDefaultValueFormatter *valueFormatter;
    @public LineChartView *chartView;
    @public LineChartData *chartData;
    @public LineChartDataSet *dataSet;
    @public ChartLimitLine *limitLine;
    @public ChartLegendEntry *legendEntries[1];
}
@end

@interface DurationAreaChartView() {
    CFStringRef currString;
    @public LineChartView *chartView;
    @public LineChartData *chartData;
    @public LineChartDataSet *dataSets[5];
    @public ChartLegendEntry *legendEntries[4];
}
@end

@interface LiftChartView() {
    @public LineChartView *chartView;
    @public LineChartData *chartData;
    @public LineChartDataSet *dataSets[4];
    @public ChartLegendEntry *legendEntries[4];
}
@end

@interface HistoryViewController() {
    HistoryViewModel *viewModel;
    struct formatter {
        char wordMonths[12][4];
        char numMonths[12][3];
        XAxisFormatType formatType;
        CFStringRef currString;
    } formatter;
    UISegmentedControl *rangePicker;
    TotalWorkoutsChartView *gradientChart;
    DurationAreaChartView *areaChart;
    LiftChartView *liftChart;
}
@end

@implementation HistoryViewController
- (id) initWithDelegate: (HistoryTabCoordinator *)_delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = &_delegate->viewModel;

    memcpy(formatter.wordMonths, (char [][4]){
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}, 48);
    memcpy(formatter.numMonths,
           (char [][3]){"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"}, 36);
    formatter.currString = CFStringCreateCopy(NULL, CFSTR(""));
    return self;
}

- (void) dealloc {
    CFRelease(formatter.currString);
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

    CFStringRef segments[] = {CFSTR("6 Months"), CFSTR("1 Year"), CFSTR("2 Years")};
    rangePicker = createSegmentedControl(segments, 3, 0);
    [rangePicker addTarget:self action:@selector(updateSelectedSegment:)
          forControlEvents:UIControlEventValueChanged];

    UIView *gradientChartSeparator = createChartSeparator(CFSTR("Workouts Each Week"));
    gradientChart = [[TotalWorkoutsChartView alloc] initWithFormatter:self];
    UIView *areaChartSeparator = createChartSeparator(CFSTR("Activity Time Each Week (By Type)"));
    areaChart = [[DurationAreaChartView alloc] initWithFormatter:self];
    UIView *liftChartSeparator = createChartSeparator(CFSTR("Lift Progress"));
    liftChart = [[LiftChartView alloc] initWithFormatter:self];

    id subviews[] = {
        rangePicker, gradientChartSeparator, gradientChart, areaChartSeparator, areaChart,
        liftChartSeparator, liftChart
    };
    UIStackView *vStack = createStackView(subviews, 7, 1, 5, 0, (HAEdgeInsets){10, 8, 10, 8});

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:rangePicker];

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [scrollView.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:guide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:guide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [rangePicker.heightAnchor constraintEqualToConstant:30]
    }, 10);

    [vStack release];
    [gradientChartSeparator release];
    [areaChartSeparator release];
    [liftChartSeparator release];
    [scrollView release];

    historyViewModel_fetchData(viewModel);
    formatter.formatType = historyViewModel_formatDataForTimeRange(viewModel, 0);
    [self updateCharts];

    appCoordinatorShared->loadedViewControllers |= LoadedViewController_History;
}

- (void) updateSelectedSegment: (UISegmentedControl *)sender {
    int index = (int) sender.selectedSegmentIndex;
    formatter.formatType = historyViewModel_formatDataForTimeRange(viewModel, index);
    [self updateCharts];
}

- (void) performForegroundUpdate {
    rangePicker.selectedSegmentIndex = 0;
    formatter.formatType = historyViewModel_formatDataForTimeRange(viewModel, 0);
    [self updateCharts];
}

- (void) updateCharts {
    if (!viewModel->data->size) {
        disableLineChartView(gradientChart->chartView);
        disableLineChartView(areaChart->chartView);
        disableLineChartView(liftChart->chartView);
        return;
    }

    const int count = viewModel->gradientChartViewModel.entries->size;
    const bool isSmall = count < SmallDataSetCutoff;
    char buf[10];

    {
        HistoryGradientChartViewModel *vm = &viewModel->gradientChartViewModel;
        gradientChart->limitLine.limit = vm->avgWorkouts;
        ChartDataEntry **entries = getChartEntriesFromArray(vm->entries);
        CFStringRef label = CFStringCreateWithFormat(NULL, NULL, CFSTR("Avg Workouts (%.2f)"),
                                                     vm->avgWorkouts);
        updateDataSet(isSmall, count, gradientChart->dataSet, entries,
                      gradientChart->legendEntries[0], label);
        updateChart(isSmall, count, gradientChart->chartView,
                    gradientChart->chartData, max(1.1 * vm->maxWorkouts, 7));
        [gradientChart->chartData setValueFormatter:gradientChart->valueFormatter];
    }
    {
        HistoryAreaChartViewModel *vm = &viewModel->areaChartViewModel;
        ChartDataEntry **entries = getChartEntriesFromArray(vm->entries[0]);
        CFArrayRef array = CFArrayCreate(NULL, (const void **)entries, count, kCocoaArrCallbacks);
        [areaChart->dataSets[0] replaceEntries:(__bridge NSArray*)array];

        for (int i = 1; i < 5; ++i) {
            entries = getChartEntriesFromArray(vm->entries[i]);
            int average = vm->totalByType[i - 1] / count;
            if (average > 59) {
                sprintf(buf, "%d h %d m", average / 60, average % 60);
            } else {
                sprintf(buf, "%d m", average);
            }
            CFStringRef label = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s (Avg: %s)"),
                                                         vm->names[i - 1], buf);
            updateDataSet(isSmall, count, areaChart->dataSets[i], entries,
                          areaChart->legendEntries[i - 1], label);
        }
        updateChart(isSmall, count, areaChart->chartView,
                    areaChart->chartData, 1.1 * vm->maxActivityTime);
        CFRelease(array);
    }
    {
        HistoryLiftChartViewModel *vm = &viewModel->liftChartViewModel;
        for (int i = 0; i < 4; ++i) {
            ChartDataEntry **entries = getChartEntriesFromArray(vm->entries[i]);
            double average = (double) vm->totalByExercise[i] / (double) count;
            CFStringRef label = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s (Avg: %.1f)"),
                                                         vm->names[i], average);
            updateDataSet(isSmall, count, liftChart->dataSets[i], entries,
                          liftChart->legendEntries[i], label);
        }
        updateChart(isSmall, count, liftChart->chartView, liftChart->chartData,
                    1.1 * vm->maxWeight);
    }
}

- (NSString *_Nonnull) stringForValue: (double)value axis: (ChartAxisBase *_Nullable)axis {
    CFRelease(formatter.currString);
    const HistoryWeekDataModel *model = &viewModel->data->arr[(int) value];
    if (formatter.formatType == FormatShort) {
        formatter.currString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s %d"),
                                                        formatter.wordMonths[model->month],
                                                        model->day);
    } else {
        formatter.currString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s/%d/%d"),
                                                        formatter.numMonths[model->month],
                                                        model->day, model->year);
    }
    return (__bridge NSString*) formatter.currString;
}
@end

#pragma mark - Total Workouts Chart

@implementation TotalWorkoutsChartView
- (id) initWithFormatter: (NSObject<ChartAxisValueFormatter>*)xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;

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

    memcpy(legendEntries, (id []){createLegendEntry(UIColor.systemTealColor)},
           sizeof(ChartLegendEntry*));
    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)legendEntries, 1, kCocoaArrCallbacks);
    limitLine = [[ChartLimitLine alloc] initWithLimit:0];
    double first = 5, second = 5;
    CFNumberRef lineDashLengths[] = {
        CFNumberCreate(NULL, kCFNumberDoubleType, &first),
        CFNumberCreate(NULL, kCFNumberDoubleType, &second)
    };
    CFArrayRef lineArr = CFArrayCreate(NULL, (const void **)lineDashLengths,
                                       2, &kCFTypeArrayCallBacks);
    limitLine.lineDashLengths = (__bridge NSArray*)lineArr;
    limitLine.lineColor = UIColor.systemTealColor;

    CFArrayRef entryArr = CFArrayCreate(NULL, (const void **)((id []){}), 0, kCocoaArrCallbacks);
    dataSet = [[LineChartDataSet alloc] initWithEntries:(__bridge NSArray*)entryArr];
    setupDataSetCommon(dataSet, UIColor.systemRedColor);
    dataSet.fill = fill;
    dataSet.drawFilledEnabled = true;
    dataSet.fillAlpha = 0.75;

    chartData = [[LineChartData alloc] initWithDataSet:dataSet];

    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    setupChartCommon(chartView, self, xAxisFormatter, legendArr, 390);
    [chartView.leftAxis addLimitLine:limitLine];

    CFRelease(colorSpace);
    CFRelease(chartGradient);
    CFRelease(colorsArray);
    CFRelease(lineArr);
    CFRelease(entryArr);
    CFRelease(lineDashLengths[0]);
    CFRelease(lineDashLengths[1]);
    CFRelease(legendArr);
    [fill release];
    [formatter release];
    return self;
}

- (void) dealloc {
    [chartView release];
    [chartData release];
    [dataSet release];
    [limitLine release];
    [legendEntries[0] release];
    [valueFormatter release];
    [super dealloc];
}

- (void) layoutSubviews {
    updateLineChartLegendWidth(chartView, self.bounds.size.width);
}
@end

#pragma mark - Duration Area Chart

@implementation DurationAreaChartView
- (id) initWithFormatter: (NSObject<ChartAxisValueFormatter>*) xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;

    UIColor *colors[] = HistoryChartColors;
    memcpy(legendEntries, (id []){
        createLegendEntry(colors[0]), createLegendEntry(colors[1]),
        createLegendEntry(colors[2]), createLegendEntry(colors[3])
    }, 4 * sizeof(ChartLegendEntry*));
    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)legendEntries, 4, kCocoaArrCallbacks);

    CFArrayRef entryArr = CFArrayCreate(NULL, (const void **)((id []){}), 0, kCocoaArrCallbacks);
    dataSets[0] = [[LineChartDataSet alloc] initWithEntries:(__bridge NSArray*)entryArr];
    CFRelease(entryArr);
    for (int i = 1; i < 5; ++i) {
        entryArr = CFArrayCreate(NULL, (const void **)((id []){}), 0, kCocoaArrCallbacks);
        dataSets[i] = [[LineChartDataSet alloc] initWithEntries:(__bridge NSArray*)entryArr];
        setupDataSetCommon(dataSets[i], colors[i - 1]);
        dataSets[i].fillColor = colors[i - 1];
        dataSets[i].drawFilledEnabled = true;
        dataSets[i].fillAlpha = 0.75;
        AreaChartFormatter *fillFormatter = [[AreaChartFormatter alloc]
                                             initWithBoundaryDataSet:dataSets[i - 1]];
        dataSets[i].fillFormatter = fillFormatter;
        [fillFormatter release];
        CFRelease(entryArr);
    }
    id _ds[] = {dataSets[4], dataSets[3], dataSets[2], dataSets[1]};
    CFArrayRef datasetArr = CFArrayCreate(NULL, (const void **) _ds, 4, kCocoaArrCallbacks);
    chartData = [[LineChartData alloc] initWithDataSets:(__bridge NSArray*)datasetArr];
    [chartData setValueFormatter:self];

    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    setupChartCommon(chartView, self, xAxisFormatter, legendArr, 425);
    chartView.leftAxis.valueFormatter = self;

    LineChartRenderer *renderer = [[CustomLineChartRenderer alloc]
                                   initWithDataProvider:chartView animator:chartView.chartAnimator
                                   viewPortHandler:chartView.viewPortHandler];
    currString = CFStringCreateCopy(NULL, CFSTR(""));
    chartView.renderer = renderer;
    [renderer release];
    CFRelease(datasetArr);
    CFRelease(legendArr);
    return self;
}

- (void) dealloc {
    for (int i = 0; i < 4; ++i) {
        [dataSets[i] release];
        [legendEntries[i] release];
    }
    [dataSets[4] release];
    CFRelease(currString);
    [chartData release];
    [chartView release];
    [super dealloc];
}

- (void) layoutSubviews {
    updateLineChartLegendWidth(chartView, self.bounds.size.width);
}

- (NSString * _Nonnull) stringForValue: (double)value axis: (ChartAxisBase * _Nullable)axis {
    return getDurationStringForAreaChart(&currString, value);
}

- (NSString * _Nonnull) stringForValue: (double)value entry: (ChartDataEntry * _Nonnull)entry
                          dataSetIndex: (NSInteger)dataSetIndex
                       viewPortHandler: (ChartViewPortHandler * _Nullable)viewPortHandler {
    return getDurationStringForAreaChart(&currString, value);
}
@end

#pragma mark - Lift Line Chart

@implementation LiftChartView
- (id) initWithFormatter: (NSObject<ChartAxisValueFormatter>*) xAxisFormatter {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;

    UIColor *colors[] = HistoryChartColors;
    memcpy(legendEntries, (id []){
        createLegendEntry(colors[0]), createLegendEntry(colors[1]),
        createLegendEntry(colors[2]), createLegendEntry(colors[3])
    }, 4 * sizeof(ChartLegendEntry*));
    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)legendEntries, 4, kCocoaArrCallbacks);

    for (int i = 0; i < 4; ++i) {
        CFArrayRef entryArr = CFArrayCreate(NULL, (const void **)((id []){}),
                                            0, kCocoaArrCallbacks);
        dataSets[i] = [[LineChartDataSet alloc] initWithEntries:(__bridge NSArray*)entryArr];
        setupDataSetCommon(dataSets[i], colors[i]);
        dataSets[i].lineWidth = 2;
        CFRelease(entryArr);
    }
    CFArrayRef datasetArr = CFArrayCreate(NULL, (const void **)dataSets, 4, kCocoaArrCallbacks);
    chartData = [[LineChartData alloc] initWithDataSets:(__bridge NSArray*)datasetArr];

    chartView = [[LineChartView alloc] initWithFrame:CGRectZero];
    setupChartCommon(chartView, self, xAxisFormatter, legendArr, 550);
    CFRelease(datasetArr);
    CFRelease(legendArr);
    return self;
}

- (void) dealloc {
    for (int i = 0; i < 4; ++i) {
        [dataSets[i] release];
        [legendEntries[i] release];
    }
    [chartData release];
    [chartView release];
    [super dealloc];
}

- (void) layoutSubviews {
    updateLineChartLegendWidth(chartView, self.bounds.size.width);
}
@end
