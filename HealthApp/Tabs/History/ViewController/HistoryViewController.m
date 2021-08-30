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
#include "HistoryChartHelpers.h"
#include "ViewControllerHelpers.h"

@interface HistoryViewController() {
    HistoryViewModel *viewModel;
    UISegmentedControl *rangePicker;
    TotalWorkoutsChart *totalWorkoutsChart;
    WorkoutTypeChart *workoutTypeChart;
    LiftingChart *liftChart;
}
@end

@implementation HistoryViewController
- (id) initWithDelegate: (HistoryTabCoordinator *)_delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = &_delegate->viewModel;
    return self;
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemBackgroundColor);
    self.navigationItem.title = @"Workout History";

    UIView *separators[] = {
        createChartSeparator(CFSTR("Workouts Each Week")),
        createChartSeparator(CFSTR("Activity Time Each Week (By Type)")),
        createChartSeparator(CFSTR("Lift Progress"))
    };
    CFStringRef segments[] = {CFSTR("6 Months"), CFSTR("1 Year"), CFSTR("2 Years")};

    rangePicker = createSegmentedControl(segments, 3, 0, self, @selector(updateSelectedSegment:));
    totalWorkoutsChart = [[TotalWorkoutsChart alloc]
                     initWithViewModel:&viewModel->totalWorkoutsViewModel formatter:self];
    workoutTypeChart = [[WorkoutTypeChart alloc]
                 initWithViewModel:&viewModel->workoutTypeViewModel formatter:self];
    liftChart = [[LiftingChart alloc]
                 initWithViewModel:&viewModel->liftViewModel formatter:self];

    id subviews[] = {
        rangePicker, separators[0], totalWorkoutsChart, separators[1], workoutTypeChart,
        separators[2], liftChart
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
    [scrollView release];
    for (int i = 0; i < 3; ++i)
        [separators[i] release];

    historyViewModel_fetchData(viewModel);
    historyViewModel_formatDataForTimeRange(viewModel, 0);
    [self updateCharts];
    appCoordinatorShared->loadedViewControllers |= LoadedViewController_History;
}

- (void) updateSelectedSegment: (UISegmentedControl *)sender {
    historyViewModel_formatDataForTimeRange(viewModel, (int) sender.selectedSegmentIndex);
    [self updateCharts];
}

- (void) performForegroundUpdate {
    rangePicker.selectedSegmentIndex = 0;
    [self updateSelectedSegment:rangePicker];
}

- (void) updateCharts {
    if (!viewModel->data->size) {
        disableLineChartView(totalWorkoutsChart->chartView);
        disableLineChartView(workoutTypeChart->chartView);
        disableLineChartView(liftChart->chartView);
        return;
    }

    const int count = viewModel->totalWorkoutsViewModel.entries->size;
    const bool isSmall = !viewModel->formatter.formatType;
    [totalWorkoutsChart updateWithCount:count isSmall:isSmall];
    [workoutTypeChart updateWithCount:count isSmall:isSmall];
    [liftChart updateWithCount:count isSmall:isSmall];
}

- (NSString *_Nonnull) stringForValue: (double)value axis: (AxisBase *_Nullable)axis {
    return (__bridge NSString*) historyViewModel_getXAxisLabel(viewModel, (int) value);
}
@end
