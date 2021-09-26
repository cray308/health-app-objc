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
#import "StatusButton.h"

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
    self.navigationItem.title = (__bridge NSString*) localize(CFSTR("titles0"));

    greetingLabel = createLabel(NULL, UIFontTextStyleTitle1, NSTextAlignmentCenter);
    weeklyWorkoutsStack = createStackView(NULL, 0, 1, 0, 0, (HAEdgeInsets){5, 8, 5, 8});
    [weeklyWorkoutsStack setHidden:true];

    UIView *divider = createDivider();
    UILabel *headerLabel = createLabel(localize(CFSTR("customWorkoutsHeader")),
                                       UIFontTextStyleTitle2, 4);
    UIStackView *customWorkoutStack = createStackView((id []){divider, headerLabel}, 2, 1, 4, 0,
                                                      (HAEdgeInsets){5, 8, 5, 8});

    for (int i = 0; i < 5; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("homeWorkoutType%d"), i);
        UIView *btn = [[StatusButton alloc]
                       initWithButtonText:localize(key) hideHeader:true hideBox:true tag:i
                       target:self action:@selector(customWorkoutButtonTapped:)];
        [customWorkoutStack addArrangedSubview:btn];
        CFRelease(key);
        [btn release];
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
    appCoordinator->loadedViewControllers |= LoadedViewController_Home;
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
    UILabel *headerLabel = createLabel(localize(CFSTR("weeklyWorkoutsHeader")),
                                       UIFontTextStyleTitle2, 4);
    [weeklyWorkoutsStack addArrangedSubview:headerLabel];
    [weeklyWorkoutsStack addArrangedSubview:divider];

    for (int i = 0; i < 7; ++i) {
        if (!viewModel->workoutNames[i]) continue;
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("dayNames%d"), i);
        StatusButton *btn = [[StatusButton alloc]
                             initWithButtonText:NULL hideHeader:false hideBox:false tag:i
                             target:self action:@selector(workoutButtonTapped:)];
        [btn updateWithLabelText:localize(key) buttonText:viewModel->workoutNames[i]
                           state:StatusViewStateDisabled enable:true];
        [weeklyWorkoutsStack addArrangedSubview:btn];
        [btn release];
        CFRelease(key);
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

    const unsigned char completed = userData->completedWorkouts;

    for (int i = 2; i < count; ++i) {
        StatusButton *v = (StatusButton *) CFArrayGetValueAtIndex(subviews, i);
        bool enable = !(completed & (1 << (int) v.tag));
        [v updateState:enable ? StatusViewStateDisabled : StatusViewStateFinished enable:enable];
    }
}

- (void) workoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromPlan(delegate, (int) btn.tag);
}

- (void) customWorkoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromCustomButton(delegate, (int) btn.tag);
}

- (void) updateGreeting {
    setLabelText(greetingLabel, viewModel->timeNames[viewModel->timeOfDay]);
}
@end
