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
    self.navigationItem.title = (__bridge NSString*) localize(CFSTR("titles1"));

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

    [self updateSelectedSegment:rangePicker];
    appCoordinator->loadedViewControllers |= LoadedViewController_History;
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
    [totalWorkoutsChart updateWithCount:count isSmall:viewModel->isSmall];
    [workoutTypeChart updateWithCount:count isSmall:viewModel->isSmall];
    [liftChart updateWithCount:count isSmall:viewModel->isSmall];
}

- (NSString *_Nonnull) stringForValue: (double)value axis: (AxisBase *_Nullable)axis {
    return (__bridge NSString*) historyViewModel_getXAxisLabel(viewModel, (int) value);
}
@end
