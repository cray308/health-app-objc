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
    Container customContainer, planContainer;
}
@end

id homeVC_init(HomeTabCoordinator *delegate) {
    HomeViewController *this = [[HomeViewController alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    this->model = &delegate->model;
    return this;
}

void homeVC_updateWorkoutsList(HomeViewController *vc) {
    if (!(homeViewModel_hasWorkoutsForThisWeek(vc->model) && vc->planContainer.views->size)) return;
    for (unsigned i = 0; i < vc->planContainer.views->size; ++i) {
        StatusButton *v = vc->planContainer.views->arr[i];
        bool enable = !(userData->completedWorkouts & (1 << (int) v.tag));
        enableButton(v->button, enable);
        setBackground(v->box, enable ? UIColor.systemGrayColor : UIColor.systemGreenColor);
        statusButton_updateAccessibility(v, vc->model->stateNames[enable]);
    }
}

void homeVC_createWorkoutsList(HomeViewController *vc) {
    array_clear(object, vc->planContainer.views);
    if (!homeViewModel_hasWorkoutsForThisWeek(vc->model)) {
        hideView(vc->planContainer.view, true);
        return;
    }

    CFStringRef days[7]; fillStringArray(days, CFSTR("dayNames%d"), 7);
    for (int i = 0; i < 7; ++i) {
        if (!vc->model->workoutNames[i]) continue;
        StatusButton *btn = statusButton_init(NULL, 0, i, vc, sel_getUid("workoutButtonTapped:"));
        setLabelText(btn->headerLabel, days[i]);
        setButtonTitle(btn->button, vc->model->workoutNames[i], 0);
        container_add(&vc->planContainer, btn);
    }

    hideView(vc->planContainer.view, false);
    homeVC_updateWorkoutsList(vc);
}

@implementation HomeViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = _nsstr(localize(CFSTR("titles0")));

    greetingLabel = createLabel(NULL, TextTitle1, NSTextAlignmentCenter, true);
    UIStackView *vStack = createStackView((id[]){greetingLabel}, 1, 1, 20, (Padding){10, 0, 16, 0});
    CFStringRef titles[5]; fillStringArray(titles, CFSTR("homeWorkoutType%d"), 5);
    CFStringRef headers[2]; fillStringArray(headers, CFSTR("homeHeader%d"), 2);
    [vStack addArrangedSubview:createContainer(&planContainer, headers[0], 0, 0, 1)];
    [vStack addArrangedSubview:createContainer(&customContainer, headers[1], 0, 4, 1)];
    hideView(planContainer.view, true);

    for (int i = 0; i < 5; ++i) {
        UIView *btn = statusButton_init(titles[i], 1, i, self, @selector(customButtonTapped:));
        statusButton_updateAccessibility(btn, NULL);
        container_add(&customContainer, btn);
    }

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];

    pin(scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
    pin(vStack, scrollView, (Padding){0}, 0);
    setEqualWidths(vStack, scrollView);

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
