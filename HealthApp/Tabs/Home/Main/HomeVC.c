#include "HomeVC.h"
#include <CoreGraphics/CGColor.h>
#include <dispatch/queue.h>
#include <math.h>
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "ContainerView.h"
#include "ExerciseManager.h"
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

extern id kCAEmitterLayerLine;

enum {
    CustomWorkoutTestMax,
    CustomWorkoutEndurance,
    CustomWorkoutStrength,
    CustomWorkoutSE,
    CustomWorkoutHIC
};

Class HomeVCClass;
Ivar HomeVCDataRef;

id homeVC_init(void) {
    id self = createVC(HomeVCClass);
    HomeVCData *data = malloc(sizeof(HomeVCData));
    fillStringArray(data->stateNames, CFSTR("homeState%d"), 2);
    fillStringArray(data->timeNames, CFSTR("timesOfDay%d"), 3);
    object_setIvar(self, HomeVCDataRef, (id) data);
    return self;
}

void homeVC_updateWorkoutsList(id self) {
    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    ContainerViewData *planData =
    (ContainerViewData *) object_getIvar(data->planContainer, ContainerViewDataRef);
    if (!data->numWorkouts) return;

    for (int i = 0; i < data->numWorkouts; ++i) {
        id v = planData->views->arr[i];
        StatusViewData *ptr = (StatusViewData *) object_getIvar(v, StatusViewDataRef);
        int tag = getTag(v);
        bool enable = !(userData->completedWorkouts & (1 << tag));
        enableButton(ptr->button, enable);
        setBackground(ptr->box, createColor(enable ? ColorGray : ColorGreen));
        statusView_updateAccessibility(v, data->stateNames[enable]);
    }
}

void homeVC_createWorkoutsList(id self) {
    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    ContainerViewData *planData =
    (ContainerViewData *) object_getIvar(data->planContainer, ContainerViewDataRef);

    data->numWorkouts = 0;
    array_clear(object, planData->views);
    CFArrayRef views = getArray(planData->stack, sel_getUid("arrangedSubviews"));
    int count = (int) CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        id v = (id) CFArrayGetValueAtIndex(views, i);
        removeView(v);
    }

    if (userData->currentPlan < 0 || userData->planStart > time(NULL)) {
        hideView(data->planContainer, true);
        return;
    }

    CFStringRef workoutNames[7] = {0};
    exerciseManager_setWeeklyWorkoutNames(userData->currentPlan,
                                          appUserData_getWeekInPlan(), workoutNames);

    SEL btnTap = sel_getUid("buttonTapped:");
    CFStringRef days[7];
    fillStringArray(days, CFSTR("dayNames%d"), 7);
    for (int i = 0; i < 7; ++i) {
        if (!workoutNames[i]) continue;
        id btn = statusView_init(workoutNames[i], i, self, btnTap);
        StatusViewData *ptr = (StatusViewData *) object_getIvar(btn, StatusViewDataRef);
        setLabelText(ptr->headerLabel, days[i]);
        containerView_add(data->planContainer, btn);
        CFRelease(workoutNames[i]);
        data->numWorkouts += 1;
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
    data->planContainer = containerView_init(headers[0], 0, true);
    id customContainer = containerView_init(headers[1], 4, true);
    id vStack = createStackView((id[]){data->greetingLabel, data->planContainer, customContainer},
                                3, 1, 20, (Padding){10, 0, 16, 0});

    SEL btnTap = sel_getUid("customButtonTapped:");
    for (int i = 0; i < 5; ++i) {
        id btn = statusView_init(titles[i], i, self, btnTap);
        StatusViewData *ptr = (StatusViewData *) object_getIvar(btn, StatusViewDataRef);
        hideView(ptr->box, true);
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

    homeVC_createWorkoutsList(self);
    appCoordinator->loadedViewControllers |= LoadedVC_Home;
}

void homeVC_viewWillAppear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    struct tm localInfo;
    time_t now = time(NULL);
    localtime_r(&now, &localInfo);
    int timeOfDay = 0;
    int hour = localInfo.tm_hour;

    if (hour >= 12 && hour < 17) {
        timeOfDay = 1;
    } else if (hour < 5 || hour >= 17) {
        timeOfDay = 2;
    }
    setLabelText(data->greetingLabel, data->timeNames[timeOfDay]);
}

void homeVC_workoutButtonTapped(id self, SEL _cmd _U_, id btn) {
    int index = getTag(btn);
    Workout *w = exerciseManager_getWeeklyWorkoutAtIndex(userData->currentPlan,
                                                         appUserData_getWeekInPlan(), index);
    if (w)
        homeVC_navigateToAddWorkout(self, w);
}

void homeVC_customButtonTapped(id self, SEL _cmd _U_, id btn) {
    int index = getTag(btn);
    unsigned char type = WorkoutStrength;
    Workout *w;
    switch (index) {
        case CustomWorkoutSE:
            type = WorkoutSE;
            break;
        case CustomWorkoutHIC:
            type = WorkoutHIC;
            break;
        case CustomWorkoutTestMax:
            w = exerciseManager_getWorkoutFromLibrary(&(WorkoutParams){
                -1, WorkoutStrength, 2, 1, 1, 100
            });
            homeVC_navigateToAddWorkout(self, w);
            return;
        case CustomWorkoutEndurance:
            type = WorkoutEndurance;
        default:
            break;
    }

    CFArrayRef names = exerciseManager_createWorkoutNames(type);
    if (!names) return;
    else if (!CFArrayGetCount(names)) {
        CFRelease(names);
        return;
    }

    presentModalVC(self, setupWorkoutVC_init(self, type, names));
}

void homeVC_navigateToAddWorkout(id self, void *workout) {
    id vc = workoutVC_init(workout);
    id navVC = getNavVC(self);
    ((void(*)(id,SEL,id,bool))objc_msgSend)(navVC,
                                            sel_getUid("pushViewController:animated:"), vc, true);
    releaseObj(vc);
}

static void showConfetti(id self) {
    id view = getView(self);
    CGRect frame;
    getRect(view, &frame, 0);
    id confettiView = createObjectWithFrame(objc_getClass("UIView"), frame);
    id grayColor = createColor(ColorGray);
    id bg = getObjectWithFloat(grayColor, sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackground(confettiView, bg);

    Class cellClass = objc_getClass("CAEmitterCell");
    SEL cgImg = sel_getUid("CGImage"), cgCol = sel_getUid("CGColor"), init = sel_getUid("init");
    SEL sColor = sel_getUid("setColor:"), sImg = sel_getUid("setContents:");
    SEL sRate = sel_getUid("setBirthRate:"), sLife = sel_getUid("setLifetime:");
    SEL sVel = sel_getUid("setVelocity:"), sELong = sel_getUid("setEmissionLongitude:");
    SEL sERange = sel_getUid("setEmissionRange:"), sSpin = sel_getUid("setSpin:");
    SEL sScale = sel_getUid("setScale:"), sScaleR = sel_getUid("setScaleRange:");

    int const velocities[] = {100, 90, 150, 200};
    id _colors[] = {
        createColor(ColorRed), createColor(ColorBlue),
        createColor(ColorGreen), getColorRef(1, 0.84, 0.04, 1)
    };
    CGColorRef shapeColors[4];
    id images[4];
    for (int i = 0; i < 4; ++i) {
        CFStringRef name = CFStringCreateWithFormat(NULL, NULL, CFSTR("confetti%d"), i);
        id img = createImage(name);
        CFRelease(name);
        images[i] = (id)((CGImageRef(*)(id,SEL))objc_msgSend)(img, cgImg);
        shapeColors[i] = ((CGColorRef(*)(id,SEL))objc_msgSend)(_colors[i], cgCol);
    }

    id cells[16];
    for (int i = 0; i < 16; ++i) {
        cells[i] = getObject(allocClass(cellClass), init);
        setFloat(cells[i], sRate, 4);
        setFloat(cells[i], sLife, 14);
        int velocity = velocities[arc4random_uniform(4)];
        setCGFloat(cells[i], sVel, velocity);
        setCGFloat(cells[i], sELong, M_PI);
        setCGFloat(cells[i], sERange, 0.5);
        setCGFloat(cells[i], sSpin, 3.5);
        ((void(*)(id,SEL,CGColorRef))objc_msgSend)(cells[i], sColor, shapeColors[i >> 2]);
        setObject(cells[i], sImg, images[i % 4]);
        setCGFloat(cells[i], sScaleR, 0.25);
        setCGFloat(cells[i], sScale, 0.1);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, &retainedArrCallbacks);
    id _layer = allocClass(objc_getClass("CAEmitterLayer"));
    id particleLayer = getObject(_layer, init);
    id viewLayer = getLayer(confettiView);
    setObject(viewLayer, sel_getUid("addSublayer:"), particleLayer);
    ((void(*)(id,SEL,CGPoint))objc_msgSend)(particleLayer, sel_getUid("setEmitterPosition:"),
                                            (CGPoint){frame.size.width / 2, 0});
    setObject(particleLayer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    ((void(*)(id,SEL,CGSize))objc_msgSend)(particleLayer, sel_getUid("setEmitterSize:"),
                                           (CGSize){frame.size.width - 16, 1});
    setArray(particleLayer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    releaseObj(particleLayer);
    for (int i = 0; i < 16; ++i) {
        releaseObj(cells[i]);
    }
    addSubview(view, confettiView);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 5),
                   dispatch_get_main_queue(), ^(void) {
        removeView(confettiView);
        releaseObj(confettiView);
        id ctrl = createAlertController(localize(CFSTR("homeAlertTitle")),
                                        localize(CFSTR("homeAlertMessage")));
        addAlertAction(ctrl, localize(CFSTR("ok")), 0, NULL);
        presentVC(self, ctrl);
    });
}

void homeVC_handleFinishedWorkout(id self, int totalCompleted) {
    HomeVCData *data = (HomeVCData *) object_getIvar(self, HomeVCDataRef);
    homeVC_updateWorkoutsList(self);
    if (data->numWorkouts == totalCompleted) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 2.5),
                       dispatch_get_main_queue(), ^(void) {
            showConfetti(self);
        });
    }
}