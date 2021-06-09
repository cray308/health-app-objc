//
//  HomeViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewController.h"
#import "HomeViewModel.h"
#import "HomeTabCoordinator.h"
#import "AddWorkoutCoordinator.h"
#import "AppCoordinator.h"
#import "NavBarCoinsView.h"
#import "ChartSeparatorView.h"
#import "ConfettiEmitterView.h"
#import "HomeBarChartView.h"
#import "HomePieChartView.h"
#import "ViewControllerHelpers.h"

@interface HomeViewController() {
    HomeViewModel *viewModel;
    NavBarCoinsView *navBarCoinsView;
    UILabel *greetingLabel;
    UILabel *weeklyTokensText;
    HomeBarChartView *barChart;
    HomePieChartView *pieChart;
}

@end

@implementation HomeViewController

- (id) initWithViewModel: (HomeViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [navBarCoinsView release];
    [greetingLabel release];
    [weeklyTokensText release];
    [barChart release];
    [pieChart release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemBackgroundColor;
    self.navigationItem.title = @"Home";
    [self setupSubviews];
    appCoordinator_setTabToLoaded(viewModel->delegate->delegate, LoadedViewController_Home);
    [self updateGreeting];
    homeViewModel_fetchData(viewModel);
    [self updateCharts];
}

- (void) viewWillAppear: (BOOL)animated {
    [super viewWillAppear:animated];
    if (viewModel->delegate->childCoordinator) {
        addWorkoutCoordinator_free(viewModel->delegate->childCoordinator);
        viewModel->delegate->childCoordinator = NULL;
    }
    if (homeViewModel_updateTimeOfDay(viewModel)) {
        [self updateGreeting];
    }
}

- (void) setupSubviews {
    greetingLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    greetingLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle1];
    greetingLabel.textAlignment = NSTextAlignmentCenter;
    greetingLabel.adjustsFontSizeToFitWidth = true;
    greetingLabel.textColor = UIColor.labelColor;

    UIButton *addWorkoutsButton = [UIButton buttonWithType:UIButtonTypeSystem];
    addWorkoutsButton.translatesAutoresizingMaskIntoConstraints = false;
    [addWorkoutsButton setTitle:@"Add Workouts" forState:UIControlStateNormal];
    [addWorkoutsButton setTitleColor:UIColor.labelColor forState: UIControlStateNormal];
    addWorkoutsButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    addWorkoutsButton.backgroundColor = UIColor.systemBlueColor;
    addWorkoutsButton.layer.cornerRadius = 10;
    [addWorkoutsButton addTarget:self action: @selector(didPushAddWorkouts) forControlEvents:UIControlEventTouchUpInside];

    UIView *addWorkoutBtnContainer = [[UIView alloc] initWithFrame:CGRectZero];
    [addWorkoutBtnContainer addSubview:addWorkoutsButton];

    weeklyTokensText = [[UILabel alloc] initWithFrame:CGRectZero];
    weeklyTokensText.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
    weeklyTokensText.textAlignment = NSTextAlignmentCenter;
    weeklyTokensText.adjustsFontSizeToFitWidth = true;
    weeklyTokensText.textColor = UIColor.labelColor;

    ChartSeparatorView *barChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Cumulative Tokens This Week"];
    barChart = [[HomeBarChartView alloc] initWithViewModel:viewModel->barChartViewModel];
    ChartSeparatorView *pieChartSeparator = [[ChartSeparatorView alloc] initWithTitle:@"Workout Time By Intensity"];
    pieChart = [[HomePieChartView alloc] initWithViewModel:viewModel->pieChartViewModel];

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[
        greetingLabel, addWorkoutBtnContainer, weeklyTokensText, barChartSeparator, barChart, pieChartSeparator, pieChart
    ]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 5;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = UIEdgeInsetsMake(10, 0, 0, 0);

    UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:CGRectZero];
    scrollView.translatesAutoresizingMaskIntoConstraints = false;
    scrollView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    scrollView.bounces = true;
    scrollView.showsVerticalScrollIndicator = true;
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:greetingLabel];

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

        [greetingLabel.heightAnchor constraintEqualToConstant:50],

        [addWorkoutsButton.topAnchor constraintEqualToAnchor:addWorkoutBtnContainer.topAnchor],
        [addWorkoutsButton.bottomAnchor constraintEqualToAnchor:addWorkoutBtnContainer.bottomAnchor],
        [addWorkoutsButton.centerXAnchor constraintEqualToAnchor:addWorkoutBtnContainer.centerXAnchor],
        [addWorkoutsButton.heightAnchor constraintEqualToConstant:70],
        [addWorkoutsButton.widthAnchor constraintEqualToConstant:170],

        [weeklyTokensText.heightAnchor constraintEqualToConstant:30]
    ]];

    navBarCoinsView = [[NavBarCoinsView alloc] init];
    UIBarButtonItem *leftBarButtonItem = [[UIBarButtonItem alloc] initWithCustomView:navBarCoinsView];
    self.navigationItem.leftBarButtonItem = leftBarButtonItem;

    [addWorkoutBtnContainer release];
    [barChartSeparator release];
    [pieChartSeparator release];
    [vStack release];
    [scrollView release];
    [leftBarButtonItem release];
}

- (void) updateNavBarCoins: (NSString *)text {
    [navBarCoinsView updateTokens:text];
}

- (void) didPushAddWorkouts {
    homeCoordinator_navigateToAddWorkouts(viewModel->delegate);
}

- (void) updateCharts {
    NSString *tokensText = homeViewModel_getCurrentTokensText(viewModel);
    weeklyTokensText.text = tokensText;
    [tokensText release];
    [barChart updateChart];
    [pieChart updateChart];
}

- (void) runNavBarAnimation {
    [navBarCoinsView runAnimation];
}

- (void) showConfetti {
    AlertDetails *details = homeViewModel_getAlertDetailsForMeetingTokenGoal();
    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil];

    CGRect frame = CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height);
    ConfettiEmitterView *confettiView = [[ConfettiEmitterView alloc] initWithFrame:frame];
    [self.view addSubview:confettiView];
    [confettiView startAnimation];

    dispatch_time_t endTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t) NSEC_PER_SEC * 5);
    dispatch_after(endTime, dispatch_get_main_queue(), ^ (void) {
        [confettiView removeFromSuperview];
        [confettiView release];
        viewController_showAlert(self, details, okAction, NULL);
    });
}

- (void) updateGreeting {
    NSString *greeting = homeViewModel_getGreeting(viewModel);
    greetingLabel.text = greeting;
    [greeting release];
}

@end
