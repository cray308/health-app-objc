//
//  HistoryViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryViewController.h"
#import "SwiftBridging.h"
#import "TotalWorkoutsChart.h"
#import "WorkoutTypeChart.h"
#import "LiftingChart.h"
#include "AppCoordinator.h"
#include "HistoryTabCoordinator.h"
#include "HistoryChartHelpers.h"
#include "ViewControllerHelpers.h"

@interface HistoryViewController() {
    @public HistoryViewModel *model;
    @public UISegmentedControl *rangePicker;
    TotalWorkoutsChart *totalWorkoutsChart;
    WorkoutTypeChart *workoutTypeChart;
    LiftingChart *liftChart;
}
@end

@implementation HistoryViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemBackgroundColor);
    self.navigationItem.title = _nsstr(localize(CFSTR("titles1")));

    UIView *separators[] = {
        createChartSeparator(localize(CFSTR("chartHeaderTotalWorkouts"))),
        createChartSeparator(localize(CFSTR("chartHeaderWorkoutType"))),
        createChartSeparator(localize(CFSTR("chartHeaderLifts")))
    };
    CFStringRef segments[3];
    for (int i = 0; i < 3; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("historySegment%d"), i);
        segments[i] = localize(key);
        CFRelease(key);
    }

    rangePicker = createSegmentedControl(segments, 3, 0, self, @selector(updateSelectedSegment:));
    totalWorkoutsChart = totalWorkoutsChart_init(&model->totalWorkoutsModel, self);
    workoutTypeChart = workoutTypeChart_init(&model->workoutTypeModel, self);
    liftChart = liftingChart_init(&model->liftModel, self);

    id subviews[] = {
        rangePicker, separators[0], totalWorkoutsChart, separators[1], workoutTypeChart,
        separators[2], liftChart
    };
    UIStackView *vStack = createStackView(subviews, 7, 1, 5, (Padding){10, 8, 10, 8});
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
    [scrollView release];
    for (int i = 0; i < 3; ++i)
        [separators[i] release];

    [self updateSelectedSegment:rangePicker];
    appCoordinator->loadedViewControllers |= LoadedViewController_History;
}

- (void) updateSelectedSegment: (UISegmentedControl *)sender {
    historyViewModel_formatDataForTimeRange(model, (int) sender.selectedSegmentIndex);
    [self updateCharts];
}

- (void) updateCharts {
    if (!model->data->size) {
        disableLineChartView(totalWorkoutsChart->chartView);
        disableLineChartView(workoutTypeChart->chartView);
        disableLineChartView(liftChart->chartView);
        return;
    }

    const int count = model->totalWorkoutsModel.entries->size;
    totalWorkoutsChart_update(totalWorkoutsChart, count, model->isSmall);
    workoutTypeChart_update(workoutTypeChart, count, model->isSmall);
    liftingChart_update(liftChart, count, model->isSmall);
}

- (NSString *) stringForValue: (double)value axis: (AxisBase *)axis {
    return _nsstr(historyViewModel_getXAxisLabel(model, (int) value));
}
@end

id historyVC_init(void *delegate) {
    HistoryViewController *this = [[HistoryViewController alloc] initWithNibName:nil bundle:nil];
    this->model = &((HistoryTabCoordinator *) delegate)->model;
    return this;
}

void historyVC_refresh(HistoryViewController *vc) {
    vc->rangePicker.selectedSegmentIndex = 0;
    [vc updateSelectedSegment:vc->rangePicker];
}
