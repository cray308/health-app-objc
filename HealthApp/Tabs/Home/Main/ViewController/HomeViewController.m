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
#import "Divider.h"
#import "AppCoordinator.h"
#import "ConfettiEmitterView.h"
#import "HomeWorkoutDayButton.h"
#import "ViewControllerHelpers.h"
#import "CalendarDateHelpers.h"
#import "AppUserData.h"

@interface HomeViewController() {
    HomeViewModel *viewModel;
    UILabel *greetingLabel;
    UIStackView *weeklyWorkoutsStack;
}

@end

@implementation HomeViewController

- (id) initWithViewModel: (HomeViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [greetingLabel release];
    [weeklyWorkoutsStack release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;
    self.navigationItem.title = @"Home";
    [self setupSubviews];
    appCoordinator_setTabToLoaded(viewModel->delegate->delegate, LoadedViewController_Home);
    [self updateGreeting];
    homeViewModel_fetchData(viewModel);
    [self createWorkoutsList];
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

    weeklyWorkoutsStack = [[UIStackView alloc] initWithFrame:CGRectZero];
    weeklyWorkoutsStack.axis = UILayoutConstraintAxisVertical;
    weeklyWorkoutsStack.spacing = 5;
    [weeklyWorkoutsStack setLayoutMarginsRelativeArrangement:true];
    weeklyWorkoutsStack.layoutMargins = UIEdgeInsetsMake(5, 4, 4, 0);

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[greetingLabel, weeklyWorkoutsStack]];
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

        [greetingLabel.heightAnchor constraintEqualToConstant:50]
    ]];

    [weeklyWorkoutsStack setHidden:true];
    [vStack release];
    [scrollView release];
}

- (void) createWorkoutsList {
    NSArray<UIView *> *subviews = weeklyWorkoutsStack.arrangedSubviews;
    for (size_t i = 0; i < subviews.count; ++i) {
        [subviews[i] removeFromSuperview];
    }

    if (!(viewModel->workouts && viewModel->workouts->size)) {
        [weeklyWorkoutsStack setHidden:true];
        return;
    }

    UILabel *headerLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    headerLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle2];
    headerLabel.text = @"Workouts this week";
    headerLabel.adjustsFontSizeToFitWidth = true;
    headerLabel.textColor = UIColor.labelColor;
    [weeklyWorkoutsStack addArrangedSubview:headerLabel];

    Divider *divider = [[Divider alloc] init];
    [weeklyWorkoutsStack addArrangedSubview:divider];

    Array_workout *workouts = viewModel->workouts;
    Workout *w;
    NSString **weekDays = calendar_getWeekDaySymbols(false);
    int i = 0;

    array_iter(workouts, w) {
        HomeWorkoutDayButton *dayBtn = [[HomeWorkoutDayButton alloc] initWithTitle:w->title day:weekDays[w->day]];
        UIButton *btn = [dayBtn getButton];
        btn.tag = i++;
        [btn addTarget:self action:@selector(workoutButtonTapped:) forControlEvents:UIControlEventTouchUpInside];

        [weeklyWorkoutsStack addArrangedSubview:dayBtn];
        [dayBtn release];
    }

    for (i = 0; i < 7; ++i) {
        [weekDays[i] release];
    }
    free(weekDays);

    [NSLayoutConstraint activateConstraints:@[
        [headerLabel.heightAnchor constraintEqualToConstant:40],
    ]];
    [headerLabel release];
    [divider release];
    [weeklyWorkoutsStack setHidden:false];
    [self updateWorkoutsList];
}

- (void) updateWorkoutsList {
    NSArray<UIView *> *subviews = weeklyWorkoutsStack.arrangedSubviews;
    if (!(viewModel->workouts && viewModel->workouts->size && subviews.count > 2)) return;

    const unsigned char completed = appUserDataShared->completedWorkouts;
    Array_workout *workouts = viewModel->workouts;
    Workout *w;
    int i = 2;

    array_iter(workouts, w) {
        HomeWorkoutDayButton *btn = (HomeWorkoutDayButton *) subviews[i++];
        if (!btn) continue;
        [btn setEnabled:!(completed & (1 << w->day))];
    }
}

- (void) workoutButtonTapped: (UIButton *)btn {
    homeCoordinator_navigateToAddWorkout(viewModel->delegate, (int) btn.tag);
}

- (void) showConfetti {
    AlertDetails *details = alertDetails_init(@"Nicely done!", @"Great job meeting your workout goal this week.");
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
