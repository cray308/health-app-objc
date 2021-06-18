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
#import "ViewControllerHelpers.h"
#import "AppUserData.h"

static NSString *weekDays[] = {@"Monday", @"Tuesday", @"Wednesday", @"Thursday", @"Friday", @"Saturday", @"Sunday"};
static NSString *greetings[] = {@"Good morning!", @"Good afternoon!", @"Good evening!"};

@interface DayWorkoutButton: UIView
- (id) initWithTitle: (NSString *)title day: (NSString *)day;
@end

@interface DayWorkoutButton() {
    @public UIView *checkbox;
    @public UIButton *button;
}
@end

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
        addWorkoutCoordinator_stopWorkoutFromBackButtonPress(viewModel->delegate->childCoordinator);
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
    weeklyWorkoutsStack.layoutMargins = UIEdgeInsetsMake(5, 8, 5, 8);

    UIStackView *customWorkoutStack = [[UIStackView alloc] initWithFrame:CGRectZero];
    customWorkoutStack.axis = UILayoutConstraintAxisVertical;
    customWorkoutStack.spacing = 20;
    [customWorkoutStack setLayoutMarginsRelativeArrangement:true];
    customWorkoutStack.layoutMargins = UIEdgeInsetsMake(5, 8, 5, 8);
    {
        Divider *divider = [[Divider alloc] init];
        [customWorkoutStack addArrangedSubview:divider];

        UILabel *headerLabel = [[UILabel alloc] initWithFrame:CGRectZero];
        headerLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle2];
        headerLabel.text = @"Add Custom Workout";
        headerLabel.adjustsFontSizeToFitWidth = true;
        headerLabel.textColor = UIColor.labelColor;
        [customWorkoutStack addArrangedSubview:headerLabel];

        NSString *buttonTitles[] = {@"Test Max", @"Endurance", @"Strength", @"SE", @"HIC"};
        for (int i = 0; i < 5; ++i) {
            UIButton *btn = [UIButton buttonWithType:UIButtonTypeSystem];
            [btn setTitle:buttonTitles[i] forState:UIControlStateNormal];
            [btn setTitleColor:UIColor.labelColor forState: UIControlStateNormal];
            [btn setTitleColor:UIColor.secondaryLabelColor forState:UIControlStateDisabled];
            btn.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
            btn.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
            btn.layer.cornerRadius = 5;
            [btn.heightAnchor constraintEqualToConstant:50].active = true;
            btn.tag = i;
            [btn addTarget:self action:@selector(customWorkoutButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
            [customWorkoutStack addArrangedSubview:btn];
        }
        [headerLabel release];
        [divider release];
    }

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[greetingLabel, weeklyWorkoutsStack, customWorkoutStack]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 5;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = UIEdgeInsetsMake(10, 0, 16, 0);

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

    if (!homeViewModel_hasWorkoutsForThisWeek(viewModel)) {
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

    CFStringRef *names = viewModel->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (!names[i]) continue;
        DayWorkoutButton *dayBtn = [[DayWorkoutButton alloc] initWithTitle:(__bridge NSString*)names[i] day:weekDays[i]];
        dayBtn.tag = i;
        dayBtn->button.tag = i;
        [dayBtn->button addTarget:self action:@selector(workoutButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
        [weeklyWorkoutsStack addArrangedSubview:dayBtn];
        [dayBtn release];
    }

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
    if (!(homeViewModel_hasWorkoutsForThisWeek(viewModel) && subviews.count > 2)) return;

    const int len = (int) subviews.count;
    const unsigned char completed = appUserDataShared->completedWorkouts;

    for (int i = 2; i < len; ++i) {
        DayWorkoutButton *v = (DayWorkoutButton *) subviews[i];
        if (!v) continue;
        int day = (int) v.tag;
        unsigned char enabled = !(completed & (1 << day));
        [v->button setEnabled:enabled];
        v->checkbox.backgroundColor = enabled ? UIColor.systemGrayColor : UIColor.systemGreenColor;
    }
}

- (void) workoutButtonTapped: (UIButton *)btn {
    homeViewModel_handleDayWorkoutButtonTap(viewModel, (int) btn.tag);
}

- (void) customWorkoutButtonTapped: (UIButton *)btn {
    homeViewModel_handleCustomWorkoutButtonTap(viewModel, (int) btn.tag);
}

- (void) showConfetti {
    AlertDetails *details = alertDetails_init(CFSTR("Nicely done!"), CFSTR("Great job meeting your workout goal this week."));
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
    greetingLabel.text = greetings[viewModel->timeOfDay];
}
@end

#pragma mark - Day Workout Button

@implementation DayWorkoutButton
- (id) initWithTitle: (NSString *)title day: (NSString *)day {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    button = [UIButton buttonWithType:UIButtonTypeSystem];
    button.translatesAutoresizingMaskIntoConstraints = false;
    [button setTitle:title forState:UIControlStateNormal];
    [button setTitleColor:UIColor.labelColor forState: UIControlStateNormal];
    [button setTitleColor:UIColor.secondaryLabelColor forState:UIControlStateDisabled];
    button.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    button.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    button.layer.cornerRadius = 5;

    UILabel *topLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    topLabel.translatesAutoresizingMaskIntoConstraints = false;
    topLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
    topLabel.adjustsFontSizeToFitWidth = true;
    topLabel.text = day;
    topLabel.textColor = UIColor.labelColor;

    checkbox = [[UIView alloc] initWithFrame:CGRectZero];
    checkbox.translatesAutoresizingMaskIntoConstraints = false;
    checkbox.backgroundColor = UIColor.systemGrayColor;
    checkbox.layer.cornerRadius = 5;

    [self addSubview:topLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    [NSLayoutConstraint activateConstraints:@[
        [topLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [topLabel.heightAnchor constraintEqualToConstant:20],
        [topLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [topLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],

        [button.topAnchor constraintEqualToAnchor:topLabel.bottomAnchor constant:5],
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor constant:-5],

        [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
        [checkbox.widthAnchor constraintEqualToConstant:20],
        [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
    ]];
    [topLabel release];
    return self;
}

- (void) dealloc {
    [checkbox release];
    [super dealloc];
}
@end
