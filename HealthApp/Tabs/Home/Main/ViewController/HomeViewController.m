//
//  HomeViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewController.h"
#include "ViewControllerHelpers.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#import "DayWorkoutButton.h"

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

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = @"Home";

    greetingLabel = createLabel(NULL, UIFontTextStyleTitle1, NSTextAlignmentCenter);
    weeklyWorkoutsStack = createStackView(NULL, 0, 1, 5, 0, (HAEdgeInsets){5, 8, 5, 8});
    [weeklyWorkoutsStack setHidden:true];

    UIView *divider = createDivider();
    UILabel *headerLabel = createLabel(CFSTR("Add Custom Workout"), UIFontTextStyleTitle2, 4);
    UIStackView *customWorkoutStack = createStackView((id []){divider, headerLabel}, 2, 1, 20, 0,
                                                      (HAEdgeInsets){5, 8, 5, 8});

    CFStringRef buttonTitles[] = {
        CFSTR("Test Max"), CFSTR("Endurance"), CFSTR("Strength"), CFSTR("SE"), CFSTR("HIC")
    };
    for (int i = 0; i < 5; ++i) {
        UIButton *btn = createButton(buttonTitles[i], UIColor.labelColor,
                                     UIColor.secondaryLabelColor, UIFontTextStyleHeadline,
                                     UIColor.secondarySystemGroupedBackgroundColor, true, true, i,
                                     self, @selector(customWorkoutButtonTapped:));
        [btn.heightAnchor constraintEqualToConstant:50].active = true;
        [customWorkoutStack addArrangedSubview:btn];
    }

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

    [customWorkoutStack release];
    [headerLabel release];
    [divider release];
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
    if (homeViewModel_updateTimeOfDay(viewModel))
        [self updateGreeting];
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

    UIView *divider = createDivider();
    UILabel *headerLabel = createLabel(CFSTR("Workouts this week"), UIFontTextStyleTitle2, 4);
    [weeklyWorkoutsStack addArrangedSubview:headerLabel];
    [weeklyWorkoutsStack addArrangedSubview:divider];

    for (int i = 0; i < 7; ++i) {
        if (!viewModel->workoutNames[i]) continue;
        UIView *dayBtn = [[DayWorkoutButton alloc]
                          initWithTitle:viewModel->workoutNames[i] day:viewModel->weekdays[i] tag:i
                          target:self action:@selector(workoutButtonTapped:)];
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
        bool enabled = !(completed & (1 << (int) v.tag));
        enableButton(v->button, enabled);
        setBackground(v->checkbox, enabled ? UIColor.systemGrayColor : UIColor.systemGreenColor);
    }
}

- (void) workoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromPlan(delegate, (int) btn.tag);
}

- (void) customWorkoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromCustomButton(delegate, (int) btn.tag);
}

- (void) updateGreeting {
    CFStringRef greeting = CFStringCreateWithFormat(NULL, NULL, CFSTR("Good %s!"),
                                                    viewModel->timeNames[viewModel->timeOfDay]);
    setLabelText(greetingLabel, greeting);
    CFRelease(greeting);
}
@end
