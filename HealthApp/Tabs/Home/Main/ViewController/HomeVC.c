#include "HomeVC.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "ContainerView.h"
#include "StatusView.h"
#include "ViewControllerHelpers.h"

Class HomeVCClass;
Ivar HomeVCDataRef;

static bool hasWorkoutsForThisWeek(HomeViewModel *model) {
    for (int i = 0; i < 7; ++i) {
        if (model->workoutNames[i]) return true;
    }
    return false;
}

id homeVC_init(void *delegate) {
    id self = createVC(HomeVCClass);
    HomeVCData *data = malloc(sizeof(HomeVCData));
    data->delegate = delegate;
    data->model = &((HomeTabCoordinator *) delegate)->model;
    object_setIvar(self, HomeVCDataRef, (id) data);
    return self;
}

void homeVC_updateWorkoutsList(id self) {
    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    ContainerViewData *planData =
    (ContainerViewData *) object_getIvar(data->planContainer, ContainerViewDataRef);
    if (!(hasWorkoutsForThisWeek(data->model) && planData->views->size)) return;

    for (unsigned i = 0; i < planData->views->size; ++i) {
        id v = planData->views->arr[i];
        StatusViewData *ptr = (StatusViewData *) object_getIvar(v, StatusViewDataRef);
        int tag = getTag(v);
        bool enable = !(userData->completedWorkouts & (1 << tag));
        enableButton(ptr->button, enable);
        setBackground(ptr->box, createColor(enable ? ColorGray : ColorGreen));
        statusView_updateAccessibility(v, data->model->stateNames[enable]);
    }
}

void homeVC_createWorkoutsList(id self) {
    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    ContainerViewData *planData =
    (ContainerViewData *) object_getIvar(data->planContainer, ContainerViewDataRef);

    array_clear(object, planData->views);
    CFArrayRef views = getArray(planData->stack, sel_getUid("arrangedSubviews"));
    int count = (int) CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        id v = (id) CFArrayGetValueAtIndex(views, i);
        removeView(v);
    }

    if (!hasWorkoutsForThisWeek(data->model)) {
        hideView(data->planContainer, true);
        return;
    }

    SEL btnTap = sel_getUid("buttonTapped:");
    CFStringRef days[7];
    fillStringArray(days, CFSTR("dayNames%d"), 7);
    for (int i = 0; i < 7; ++i) {
        if (!data->model->workoutNames[i]) continue;
        id btn = statusView_init(NULL, false, i, self, btnTap);
        StatusViewData *ptr = (StatusViewData *) object_getIvar(btn, StatusViewDataRef);
        setLabelText(ptr->headerLabel, days[i]);
        setButtonTitle(ptr->button, data->model->workoutNames[i], 0);
        containerView_add(data->planContainer, btn);
    }

    hideView(data->planContainer, false);
    homeVC_updateWorkoutsList(self);
}

void homeVC_updateColors(id self) {
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
}

void homeVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, localize(CFSTR("titles0")));

    CFStringRef titles[5], headers[2];
    fillStringArray(titles, CFSTR("homeWorkoutType%d"), 5);
    fillStringArray(headers, CFSTR("homeHeader%d"), 2);

    data->greetingLabel = createLabel(NULL, TextTitle1, 1, true);
    data->planContainer = containerView_init(headers[0], 0, 0, true);
    id customContainer = containerView_init(headers[1], 0, 4, true);
    id vStack = createStackView((id[]){data->greetingLabel, data->planContainer, customContainer},
                                3, 1, 20, (Padding){10, 0, 16, 0});
    hideView(data->planContainer, true);

    SEL btnTap = sel_getUid("customButtonTapped:");
    for (int i = 0; i < 5; ++i) {
        id btn = statusView_init(titles[i], true, i, self, btnTap);
        statusView_updateAccessibility(btn, NULL);
        containerView_add(customContainer, btn);
    }

    id scrollView = createScrollView();
    addSubview(view, scrollView);
    id guide = getLayoutGuide(view);
    pin(scrollView, guide, (Padding){0}, 0);
    addVStackToScrollView(vStack, scrollView);

    releaseObj(customContainer);
    releaseObj(vStack);
    releaseObj(scrollView);

    setLabelText(data->greetingLabel, data->model->timeNames[data->model->timeOfDay]);
    homeViewModel_fetchData(data->model);
    homeVC_createWorkoutsList(self);
    appCoordinator->loadedViewControllers |= LoadedVC_Home;
}

void homeVC_viewWillAppear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    homeCoordinator_checkForChildCoordinator(data->delegate);
    if (homeViewModel_updateTimeOfDay(data->model)) {
        setLabelText(data->greetingLabel, data->model->timeNames[data->model->timeOfDay]);
    }
}

void homeVC_workoutButtonTapped(id self, SEL _cmd _U_, id btn) {
    void *delegate = ((HomeVCData *) object_getIvar(self, HomeVCDataRef))->delegate;
    homeCoordinator_addWorkoutFromPlan(delegate, getTag(btn));
}

void homeVC_customButtonTapped(id self, SEL _cmd _U_, id btn) {
    void *delegate = ((HomeVCData *) object_getIvar(self, HomeVCDataRef))->delegate;
    homeCoordinator_addWorkoutFromCustomButton(delegate, getTag(btn));
}
