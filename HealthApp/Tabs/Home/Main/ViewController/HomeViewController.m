//
//  HomeViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewController.h"
#import "Divider.h"
#import "ViewControllerHelpers.h"
#import "AppDelegate.h"
#include "AppUserData.h"
#include <math.h>

UIView *createConfettiView(CGRect frame) {
    UIView *this = [[UIView alloc] initWithFrame:frame];
    this.backgroundColor = [UIColor.systemGrayColor colorWithAlphaComponent:0.8];

    UIColor const* colors[] = {UIColor.systemRedColor, UIColor.systemBlueColor, UIColor.systemGreenColor,
        UIColor.systemYellowColor};
    UIImage const* images[] = {[UIImage imageNamed:@"Box"], [UIImage imageNamed:@"Triangle"],
        [UIImage imageNamed:@"Circle"], [UIImage imageNamed:@"Spiral"]};
    int const velocities[] = {100, 90, 150, 200};

    CAEmitterCell *cells[16];
    for (int i = 0; i < 16; ++i) {
        CAEmitterCell *cell = [[CAEmitterCell alloc] init];
        cell.birthRate = 4;
        cell.lifetime = 14;
        cell.velocity = velocities[arc4random_uniform(4)];
        cell.emissionLongitude = M_PI;
        cell.emissionRange = 0.5;
        cell.spin = 3.5;
        cell.color = colors[i / 4].CGColor;
        cell.contents = (id) images[i % 4].CGImage;
        cell.scaleRange = 0.25;
        cell.scale = 0.1;
        cells[i] = cell;
    }

    CAEmitterLayer *particleLayer = [[CAEmitterLayer alloc] init];
    [this.layer addSublayer:particleLayer];
    particleLayer.emitterPosition = CGPointMake(frame.size.width / 2, 0);
    particleLayer.emitterShape = kCAEmitterLayerLine;
    particleLayer.emitterSize = CGSizeMake(frame.size.width - 16, 1);
    particleLayer.emitterCells = [NSArray arrayWithObjects:cells count:16];

    [particleLayer release];
    for (int i = 0; i < 16; ++i) [cells[i] release];
    return this;
}

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
    HomeTabCoordinator *delegate;
    UILabel *greetingLabel;
    UIStackView *weeklyWorkoutsStack;
}
@end

@implementation HomeViewController
- (id) initWithDelegate: (HomeTabCoordinator *)_delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    delegate = _delegate;
    viewModel = &_delegate->viewModel;
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
    [self updateGreeting];
    homeViewModel_fetchData(viewModel);
    [self createWorkoutsList];

    AppDelegate *app = (AppDelegate *) UIApplication.sharedApplication.delegate;
    if (app) app->coordinator.loadedViewControllers |= LoadedViewController_Home;
}

- (void) viewWillAppear: (BOOL)animated {
    [super viewWillAppear:animated];
    homeCoordinator_checkForChildCoordinator(delegate);
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
    weeklyWorkoutsStack.layoutMargins = (UIEdgeInsets){.top = 5, .left = 8, .bottom = 5, .right = 8};

    UIStackView *customWorkoutStack = [[UIStackView alloc] initWithFrame:CGRectZero];
    customWorkoutStack.axis = UILayoutConstraintAxisVertical;
    customWorkoutStack.spacing = 20;
    [customWorkoutStack setLayoutMarginsRelativeArrangement:true];
    customWorkoutStack.layoutMargins = (UIEdgeInsets){.top = 5, .left = 8, .bottom = 5, .right = 8};
    {
        UIView *divider = createDivider();
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
            [btn addTarget:self
                    action:@selector(customWorkoutButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
            [customWorkoutStack addArrangedSubview:btn];
        }
        [headerLabel release];
        [divider release];
    }

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[
        greetingLabel, weeklyWorkoutsStack, customWorkoutStack]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 5;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = (UIEdgeInsets){.top = 10, .bottom = 16};

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

    UIView *divider = createDivider();
    [weeklyWorkoutsStack addArrangedSubview:divider];

    CFStringRef *weekdays = viewModel->weekdays;
    CFStringRef *names = viewModel->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (!names[i]) continue;
        DayWorkoutButton *dayBtn = [[DayWorkoutButton alloc] initWithTitle:(__bridge NSString*)names[i]
                                                                       day:(__bridge NSString*)weekdays[i]];
        dayBtn.tag = i;
        dayBtn->button.tag = i;
        [dayBtn->button addTarget:self action:@selector(workoutButtonTapped:)
                 forControlEvents:UIControlEventTouchUpInside];
        [weeklyWorkoutsStack addArrangedSubview:dayBtn];
        [dayBtn release];
    }

    [NSLayoutConstraint activateConstraints:@[[headerLabel.heightAnchor constraintEqualToConstant:40]]];
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
        bool enabled = !(completed & (1 << day));
        [v->button setEnabled:enabled];
        v->checkbox.backgroundColor = enabled ? UIColor.systemGrayColor : UIColor.systemGreenColor;
    }
}

- (void) workoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromPlan(delegate, (int) btn.tag);
}

- (void) customWorkoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromCustomButton(delegate, (int) btn.tag);
}

- (void) showConfetti {
    AlertDetails details = {CFSTR("Nicely done!"), CFSTR("Great job meeting your workout goal this week.")};
    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil];

    UIView *confettiView = createConfettiView(
        (CGRect){.size = {.width = self.view.frame.size.width, .height = self.view.frame.size.height}});
    [self.view addSubview:confettiView];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 5), dispatch_get_main_queue(), ^ (void) {
        [confettiView removeFromSuperview];
        [confettiView release];
        viewController_showAlert(self, &details, okAction, NULL);
    });
}

- (void) updateGreeting {
    greetingLabel.text = (__bridge NSString*) viewModel->greetings[viewModel->timeOfDay];
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
