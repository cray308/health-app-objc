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
#import "ChartSeparatorView.h"
#import "HistoryPieChartView.h"
#import "HistoryGradientChartView.h"
#import "HistoryAreaChartView.h"

@interface HistoryViewController() {
    HistoryViewModel *viewModel;
    UIScrollView *scrollView;
    UISegmentedControl *rangePicker;
    HistoryGradientChartView *gradientChart;
    HistoryAreaChartView *areaChart;
    HistoryPieChartView *pieChart;
}

@end

@implementation HistoryViewController

- (id) initWithViewModel: (HistoryViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [scrollView release];
    [rangePicker release];
    [gradientChart release];
    [areaChart release];
    [pieChart release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemBackgroundColor;
    self.navigationItem.title = @"Workout History";
    [self setupSubviews];
    appCoordinator_setTabToLoaded(viewModel->delegate->delegate, LoadedViewController_History);
    historyViewModel_fetchData(viewModel, 0);
    [self updateChartsWithAnimation:true];
}

- (void) setupSubviews {
    rangePicker = [[UISegmentedControl alloc] initWithItems:@[@"Month", @"6 Months", @"Year"]];
    rangePicker.layer.cornerRadius = 5;
    rangePicker.tintColor = UIColor.systemGray2Color;
    rangePicker.selectedSegmentIndex = 0;
    [rangePicker addTarget:self action:@selector(updateSelectedSegment:) forControlEvents:UIControlEventValueChanged];

    ChartSeparatorView *gradientChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Tokens Each Week"];
    gradientChart = [[HistoryGradientChartView alloc] initWithViewModel:viewModel->gradientChartViewModel];
    ChartSeparatorView *areaChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Activity Time Each Week (By Intensity)"];
    areaChart = [[HistoryAreaChartView alloc] initWithViewModel:viewModel->areaChartViewModel];
    ChartSeparatorView *pieChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Activity Time Per Day"];
    pieChart = [[HistoryPieChartView alloc] initWithViewModel:viewModel->pieChartViewModel];

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[
        rangePicker, gradientChartSeparator, gradientChart, areaChartSeparator, areaChart, pieChartSeparator, pieChart
    ]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 5;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = UIEdgeInsetsMake(10, 8, 10, 8);

    scrollView = [[UIScrollView alloc] initWithFrame:CGRectZero];
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
    [pieChartSeparator release];
}

- (void) updateSelectedSegment: (UISegmentedControl *)sender {
    historyViewModel_fetchData(viewModel, (int) sender.selectedSegmentIndex);
    [self updateChartsWithAnimation:true];
}

- (void) performForegroundUpdate {
    rangePicker.selectedSegmentIndex = 0;
    historyViewModel_fetchData(viewModel, 0);
    [self updateChartsWithAnimation:true];
}

- (void) updateChartsWithAnimation: (bool)shouldAnimate {
    [gradientChart updateChart:shouldAnimate];
    [areaChart updateChart:shouldAnimate];
    [pieChart updateChart];
}

@end
