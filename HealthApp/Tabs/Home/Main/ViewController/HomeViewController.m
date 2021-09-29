//
//  HomeViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewController.h"
#include "ViewControllerHelpers.h"
#include "HomeTabCoordinator.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#import "StatusButton.h"

@interface HomeViewController() {
    @public HomeViewModel *model;
    @public HomeTabCoordinator *delegate;
    UILabel *greetingLabel;
    @public UIStackView *weeklyWorkoutsStack;
}
@end

id homeVC_init(void *delegate) {
    HomeViewController *this = [[HomeViewController alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    this->model = &this->delegate->model;
    return this;
}

void homeVC_updateWorkoutsList(HomeViewController *vc) {
    CFArrayRef subviews = _cfarr(vc->weeklyWorkoutsStack.arrangedSubviews);
    int count = (int) CFArrayGetCount(subviews);
    if (!(homeViewModel_hasWorkoutsForThisWeek(vc->model) && count > 2)) return;

    const unsigned char completed = userData->completedWorkouts;

    for (int i = 2; i < count; ++i) {
        StatusButton *v = (StatusButton *) CFArrayGetValueAtIndex(subviews, i);
        bool enable = !(completed & (1 << (int) v.tag));
        enableButton(v->button, enable);
        setBackground(v->box, enable ? UIColor.systemGrayColor : UIColor.systemGreenColor);
    }
}

void homeVC_createWorkoutsList(HomeViewController *vc) {
    CFArrayRef subviews = _cfarr(vc->weeklyWorkoutsStack.arrangedSubviews);
    for (int i = 0; i < (int) CFArrayGetCount(subviews); ++i)
        [((UIView *) CFArrayGetValueAtIndex(subviews, i)) removeFromSuperview];

    if (!homeViewModel_hasWorkoutsForThisWeek(vc->model)) {
        [vc->weeklyWorkoutsStack setHidden:true];
        return;
    }

    UIView *divider = createDivider();
    UILabel *headerLabel = createLabel(localize(CFSTR("weeklyWorkoutsHeader")), TextTitle2, 4);
    [vc->weeklyWorkoutsStack addArrangedSubview:headerLabel];
    [vc->weeklyWorkoutsStack addArrangedSubview:divider];

    for (int i = 0; i < 7; ++i) {
        if (!vc->model->workoutNames[i]) continue;
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("dayNames%d"), i);
        StatusButton *btn = statusButton_init(NULL, 0, i, vc, sel_getUid("workoutButtonTapped:"));
        setLabelText(btn->headerLabel, localize(key));
        setButtonTitle(btn->button, vc->model->workoutNames[i], 0);
        [vc->weeklyWorkoutsStack addArrangedSubview:btn];
        [btn release];
        CFRelease(key);
    }

    activateConstraints((id []){[headerLabel.heightAnchor constraintEqualToConstant:40]}, 1);
    [headerLabel release];
    [divider release];
    [vc->weeklyWorkoutsStack setHidden:false];
    homeVC_updateWorkoutsList(vc);
}

@implementation HomeViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = _nsstr(localize(CFSTR("titles0")));

    greetingLabel = createLabel(NULL, TextTitle1, NSTextAlignmentCenter);
    weeklyWorkoutsStack = createStackView(NULL, 0, 1, 0, (Padding){5, 8, 5, 8});
    [weeklyWorkoutsStack setHidden:true];

    UIView *divider = createDivider();
    UILabel *headerLabel = createLabel(localize(CFSTR("customWorkoutsHeader")), TextTitle2, 4);
    UIStackView *customWorkoutStack = createStackView((id []){divider, headerLabel}, 2, 1, 4,
                                               (Padding){5, 8, 5, 8});

    for (int i = 0; i < 5; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("homeWorkoutType%d"), i);
        UIView *btn = statusButton_init(localize(key), 1, i, self, @selector(customButtonTapped:));
        [customWorkoutStack addArrangedSubview:btn];
        CFRelease(key);
        [btn release];
    }

    id subviews[] = {greetingLabel, weeklyWorkoutsStack, customWorkoutStack};
    UIStackView *vStack = createStackView(subviews, 3, 1, 5, (Padding){10, 0, 16, 0});
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

    setLabelText(greetingLabel, model->timeNames[model->timeOfDay]);
    homeViewModel_fetchData(model);
    homeVC_createWorkoutsList(self);
    appCoordinator->loadedViewControllers |= LoadedViewController_Home;
}

- (void) viewWillAppear: (BOOL)animated {
    [super viewWillAppear:animated];
    homeCoordinator_checkForChildCoordinator(delegate);
    if (homeViewModel_updateTimeOfDay(model))
        setLabelText(greetingLabel, model->timeNames[model->timeOfDay]);
}

- (void) workoutButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromPlan(delegate, (int) btn.tag);
}

- (void) customButtonTapped: (UIButton *)btn {
    homeCoordinator_addWorkoutFromCustomButton(delegate, (int) btn.tag);
}
@end
