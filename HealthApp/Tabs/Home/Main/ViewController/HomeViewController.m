//
//  HomeViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewController.h"
#import "ViewControllerHelpers.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#include <math.h>

UIView *createConfettiView(CGRect frame) {
    UIView *this = [[UIView alloc] initWithFrame:frame];
    this.backgroundColor = [UIColor.systemGrayColor colorWithAlphaComponent:0.8];

    UIColor const* colors[] = {
        UIColor.systemRedColor, UIColor.systemBlueColor,
        UIColor.systemGreenColor, UIColor.systemYellowColor
    };
    UIImage const* images[] = {
        [UIImage imageNamed:@"Box"], [UIImage imageNamed:@"Triangle"],
        [UIImage imageNamed:@"Circle"], [UIImage imageNamed:@"Spiral"]
    };
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

    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, kCocoaArrCallbacks);
    CAEmitterLayer *particleLayer = [[CAEmitterLayer alloc] init];
    [this.layer addSublayer:particleLayer];
    particleLayer.emitterPosition = CGPointMake(frame.size.width / 2, 0);
    particleLayer.emitterShape = kCAEmitterLayerLine;
    particleLayer.emitterSize = CGSizeMake(frame.size.width - 16, 1);
    particleLayer.emitterCells = (__bridge NSArray*)array;

    [particleLayer release];
    for (int i = 0; i < 16; ++i) [cells[i] release];
    CFRelease(array);
    return this;
}

@interface DayWorkoutButton: UIView
- (id) initWithTitle: (CFStringRef)title day: (CFStringRef)day tag: (int)tag;
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

    greetingLabel = createLabel(NULL, UIFontTextStyleTitle1, true, NSTextAlignmentCenter);

    weeklyWorkoutsStack = createStackView(NULL, 0, 1, 5, 0, (HAEdgeInsets){5, 8, 5, 8});

    UIStackView *customWorkoutStack = createStackView(NULL, 0, 1, 20, 0,
                                                      (HAEdgeInsets){5, 8, 5, 8});

    UIView *divider = createDivider();
    [customWorkoutStack addArrangedSubview:divider];


    UILabel *headerLabel = createLabel(CFSTR("Add Custom Workout"), UIFontTextStyleTitle2,
                                       true, NSTextAlignmentNatural);
    [customWorkoutStack addArrangedSubview:headerLabel];

    CFStringRef buttonTitles[] = {
        CFSTR("Test Max"), CFSTR("Endurance"), CFSTR("Strength"), CFSTR("SE"), CFSTR("HIC")
    };
    for (int i = 0; i < 5; ++i) {
        UIButton *btn = createButton(buttonTitles[i], UIColor.labelColor,
                                     UIColor.secondaryLabelColor, UIFontTextStyleHeadline,
                                     UIColor.secondarySystemGroupedBackgroundColor,
                                     true, false, true, i);
        [btn.heightAnchor constraintEqualToConstant:50].active = true;
        [btn addTarget:self action:@selector(customWorkoutButtonTapped:)
      forControlEvents:UIControlEventTouchUpInside];
        [customWorkoutStack addArrangedSubview:btn];
    }
    [headerLabel release];
    [divider release];

    id subviews[] = {greetingLabel, weeklyWorkoutsStack, customWorkoutStack};
    UIStackView *vStack = createStackView(subviews, 3, 1, 5, 0, (HAEdgeInsets){10, 0, 16, 0});

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:greetingLabel];

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

        [greetingLabel.heightAnchor constraintEqualToConstant:50]
    }, 10);

    [weeklyWorkoutsStack setHidden:true];
    [vStack release];
    [scrollView release];

    [self updateGreeting];
    homeViewModel_fetchData(viewModel);
    [self createWorkoutsList];

    appCoordinatorShared->loadedViewControllers |= LoadedViewController_Home;
}

- (void) viewWillAppear: (BOOL)animated {
    [super viewWillAppear:animated];
    homeCoordinator_checkForChildCoordinator(delegate);
    if (homeViewModel_updateTimeOfDay(viewModel)) {
        [self updateGreeting];
    }
}

- (void) createWorkoutsList {
    CFArrayRef subviews = (__bridge CFArrayRef) weeklyWorkoutsStack.arrangedSubviews;
    for (int i = 0; i < (int) CFArrayGetCount(subviews); ++i) {
        UIView *v = CFArrayGetValueAtIndex(subviews, i);
        [v removeFromSuperview];
    }

    if (!homeViewModel_hasWorkoutsForThisWeek(viewModel)) {
        [weeklyWorkoutsStack setHidden:true];
        return;
    }

    UILabel *headerLabel = createLabel(CFSTR("Workouts this week"), UIFontTextStyleTitle2,
                                       true, NSTextAlignmentNatural);
    [weeklyWorkoutsStack addArrangedSubview:headerLabel];

    UIView *divider = createDivider();
    [weeklyWorkoutsStack addArrangedSubview:divider];

    CFStringRef *weekdays = viewModel->weekdays;
    CFStringRef *names = viewModel->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (!names[i]) continue;
        DayWorkoutButton *dayBtn = [[DayWorkoutButton alloc] initWithTitle:names[i]
                                                                       day:weekdays[i] tag:i];
        [dayBtn->button addTarget:self action:@selector(workoutButtonTapped:)
                 forControlEvents:UIControlEventTouchUpInside];
        [weeklyWorkoutsStack addArrangedSubview:dayBtn];
        [dayBtn release];
    }

    activateConstraints((id []){[headerLabel.heightAnchor constraintEqualToConstant:40]}, 1);
    [headerLabel release];
    [divider release];
    [weeklyWorkoutsStack setHidden:false];
    [self updateWorkoutsList];
}

- (void) updateWorkoutsList {
    CFArrayRef subviews = (__bridge CFArrayRef) weeklyWorkoutsStack.arrangedSubviews;
    int count = (int) CFArrayGetCount(subviews);
    if (!(homeViewModel_hasWorkoutsForThisWeek(viewModel) && count > 2)) return;

    const unsigned char completed = appUserDataShared->completedWorkouts;

    for (int i = 2; i < count; ++i) {
        DayWorkoutButton *v = (DayWorkoutButton *) CFArrayGetValueAtIndex(subviews, i);
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
    UIAlertController *ctrl = [UIAlertController
                               alertControllerWithTitle:@"Nicely done!"
                               message:@"Great job meeting your workout goal this week."
                               preferredStyle:UIAlertControllerStyleAlert];

    [ctrl addAction:[UIAlertAction actionWithTitle:@"OK"
                                             style:UIAlertActionStyleDefault handler:nil]];

    CGSize size = self.view.frame.size;
    UIView *confettiView = createConfettiView((CGRect){{0}, size});
    [self.view addSubview:confettiView];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 5),
                   dispatch_get_main_queue(), ^ (void) {
        [confettiView removeFromSuperview];
        [confettiView release];
        [self presentViewController:ctrl animated:true completion:nil];
    });
}

- (void) updateGreeting {
    CFStringRef greeting = CFStringCreateWithFormat(NULL, NULL, CFSTR("Good %s!"),
                                                    viewModel->timeNames[viewModel->timeOfDay]);
    greetingLabel.text = (__bridge NSString*) greeting;
    CFRelease(greeting);
}
@end

#pragma mark - Day Workout Button

@implementation DayWorkoutButton
- (id) initWithTitle: (CFStringRef)title day: (CFStringRef)day tag: (int)tag {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self.tag = tag;
    button = createButton(title, UIColor.labelColor, UIColor.secondaryLabelColor,
                          UIFontTextStyleHeadline, UIColor.secondarySystemGroupedBackgroundColor,
                          true, false, true, tag);


    UILabel *topLabel = createLabel(day, UIFontTextStyleSubheadline, true, NSTextAlignmentNatural);
    checkbox = createView(UIColor.systemGrayColor, true);

    [self addSubview:topLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    activateConstraints((id []){
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
    }, 13);
    [topLabel release];
    return self;
}

- (void) dealloc {
    [checkbox release];
    [super dealloc];
}
@end
