#include "HomeVC.h"
#include <CoreGraphics/CGColor.h>
#include <dispatch/queue.h>
#include <math.h>
#include "AppUserData.h"
#include "ExerciseManager.h"
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

extern id kCAEmitterLayerLine;

Class HomeVCClass;

static CFStringRef stateNames[2];

id homeVC_init(CFBundleRef bundle) {
    id self = createNew(HomeVCClass);
    fillStringArray(bundle, stateNames, CFSTR("homeState%d"), 2);
    return self;
}

void homeVC_updateWorkoutsList(HomeVC *self, unsigned char completed) {
    id gray = createColor(ColorGray), green = createColor(ColorGreen);
    CFArrayRef views = getArrangedSubviews(self->planContainer->stack);
    for (int i = 0; i < self->numWorkouts; ++i) {
        id v = (id) CFArrayGetValueAtIndex(views, i);
        StatusView *ptr = (StatusView *) ((char *)v + ViewSize);
        bool enable = !(completed & (1 << getTag(v)));
        enableButton(ptr->button, enable);
        setBackground(ptr->box, enable ? gray : green);
        statusView_updateAccessibility(ptr, stateNames[enable]);
    }
}

void homeVC_createWorkoutsList(id self, unsigned char plan) {
    HomeVC *data = (HomeVC *) ((char *)self + VCSize);
    id planContainer = (id) ((char *)data->planContainer - ViewSize);

    data->numWorkouts = 0;
    CFArrayRef views = getArrangedSubviews(data->planContainer->stack);
    int count = (int) CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        removeView((id) CFArrayGetValueAtIndex(views, i));
    }

    if ((plan & 128) || userData->planStart > time(NULL)) {
        hideView(planContainer, true);
        hideView(data->customContainer->divider, true);
        return;
    }

    CFStringRef workoutNames[7] = {0};
    exerciseManager_setWeeklyWorkoutNames(plan, workoutNames);

    SEL btnTap = sel_getUid("buttonTapped:");
    CFStringRef days[7];
    fillStringArray(CFBundleGetMainBundle(), days, CFSTR("dayNames%d"), 7);
    StatusView *sv;
    for (int i = 0; i < 7; ++i) {
        if (!workoutNames[i]) continue;
        id btn = statusView_init(workoutNames[i], &sv, i, self, btnTap);
        setLabelText(sv->headerLabel, days[i]);
        addArrangedSubview(data->planContainer->stack, btn);
        CFRelease(days[i]);
        releaseObj(btn);
        data->numWorkouts += 1;
    }

    hideView(planContainer, false);
    hideView(data->customContainer->divider, false);
    homeVC_updateWorkoutsList(data, userData->completedWorkouts);
}

void homeVC_updateColors(id self) {
    HomeVC *data = (HomeVC *) ((char *)self + VCSize);
    setBackground(getView(self), createColor(ColorPrimaryBGGrouped));
    SEL enabled = sel_getUid("isEnabled");
    id bg = createColor(ColorSecondaryBGGrouped), divColor = createColor(ColorSeparator);
    id label = createColor(ColorLabel), disabled = createColor(ColorSecondaryLabel);
    id gray = createColor(ColorGray), green = createColor(ColorGreen);
    containerView_updateColors(data->planContainer, label, divColor);
    containerView_updateColors(data->customContainer, label, divColor);
    CFArrayRef views = getArrangedSubviews(data->planContainer->stack);
    for (int i = 0; i < data->numWorkouts; ++i) {
        id v = (id) CFArrayGetValueAtIndex(views, i);
        StatusView *ptr = (StatusView *) ((char *)v + ViewSize);
        setTextColor(ptr->headerLabel, label);
        setButtonColor(ptr->button, label, 0);
        setButtonColor(ptr->button, disabled, 2);
        setBackground(ptr->button, bg);
        setBackground(ptr->box, msg0(bool, ptr->button, enabled) ? gray : green);
    }
    views = getArrangedSubviews(data->customContainer->stack);
    for (int i = 0; i < 5; ++i) {
        id v = (id) CFArrayGetValueAtIndex(views, i);
        StatusView *ptr = (StatusView *) ((char *)v + ViewSize);
        setButtonColor(ptr->button, label, 0);
        setBackground(ptr->button, bg);
    }
}

void homeVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    HomeVC *data = (HomeVC *) ((char *)self + VCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, CFBundleCopyLocalizedString(bundle, CFSTR("tabs0"), NULL, NULL));

    CFStringRef titles[5], headers[2];
    fillStringArray(bundle, titles, CFSTR("homeWorkoutType%d"), 5);
    fillStringArray(bundle, headers, CFSTR("homeHeader%d"), 2);

    id planContainer = containerView_init(headers[0], &data->planContainer, 0, true);
    hideView(data->planContainer->divider, true);
    id customContainer = containerView_init(headers[1], &data->customContainer, 4, true);
    id vStack = createStackView((id[]){planContainer, customContainer}, 2, 1, 20,
                                (Padding){10, 0, 16, 0});

    SEL btnTap = sel_getUid("customButtonTapped:");
    StatusView *sv;
    for (int i = 0; i < 5; ++i) {
        id btn = statusView_init(titles[i], &sv, i, self, btnTap);
        hideView(sv->box, true);
        statusView_updateAccessibility(sv, NULL);
        addArrangedSubview(data->customContainer->stack, btn);
        releaseObj(btn);
    }

    id scrollView = createScrollView();
    addVStackToScrollView(view, vStack, scrollView);
    releaseObj(vStack);
    releaseObj(scrollView);

    homeVC_createWorkoutsList(self, userData->currentPlan);
}

void homeVC_workoutButtonTapped(id self, SEL _cmd _U_, id btn) {
    Workout *w = exerciseManager_getWeeklyWorkout(userData->currentPlan, (int) getTag(btn));
    homeVC_navigateToAddWorkout(self, w);
}

void homeVC_customButtonTapped(id self, SEL _cmd _U_, id btn) {
    unsigned char index = (unsigned char) getTag(btn);
    if (!index) {
        Workout *w = exerciseManager_getWorkoutFromLibrary(&(WorkoutParams){
            2, 1, 1, 100, WorkoutStrength, 0xff
        });
        homeVC_navigateToAddWorkout(self, w);
        return;
    }

    presentModalVC(setupWorkoutVC_init(self, --index));
}

void homeVC_navigateToAddWorkout(id self, void *workout) {
    id vc = workoutVC_init(workout);
    msg2(void, id, bool, getNavVC(self), sel_getUid("pushViewController:animated:"), vc, true);
    releaseObj(vc);
}

static void showConfetti(id self) {
    id view = getView(self);
    CGRect frame;
    getRect(view, &frame, RectFrame);
    id confettiView = createObjectWithFrame(ViewClass, frame);
    id bg = msg1(id, CGFloat, createColor(ColorGray),
                           sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackground(confettiView, bg);

    Class cellClass = objc_getClass("CAEmitterCell");
    SEL cgSels[] = {sel_getUid("CGImage"), sel_getUid("CGColor")};
    SEL cellSels[10] = {
        sel_getUid("setBirthRate:"), sel_getUid("setLifetime:"), sel_getUid("setVelocity:"),
        sel_getUid("setEmissionLongitude:"), sel_getUid("setEmissionRange:"), sel_getUid("setSpin:"),
        sel_getUid("setColor:"), sel_getUid("setContents:"),
        sel_getUid("setScaleRange:"), sel_getUid("setScale:")
    };

    int const velocities[] = {100, 90, 150, 200};
    const char *const colorNames[4] = {
        "systemRedColor", "systemBlueColor", "systemGreenColor", "systemYellowColor"
    };
    CFStringRef imgNames[] = {
        CFSTR("confetti0"), CFSTR("confetti1"), CFSTR("confetti2"), CFSTR("confetti3")
    };
    CGColorRef shapeColors[4];
    id images[4];
    for (int i = 0; i < 4; ++i) {
        images[i] = (id) msg0(CGImageRef, createImage(imgNames[i]), cgSels[0]);
        id color = clsF0(id, ColorClass, sel_getUid(colorNames[i]));
        shapeColors[i] = msg0(CGColorRef, color, cgSels[1]);
    }

    id cells[16];
    for (int i = 0; i < 16; ++i) {
        cells[i] = createNew(cellClass);
        msg1(void, float, cells[i], cellSels[0], 4);
        msg1(void, float, cells[i], cellSels[1], 14);
        int velocity = velocities[arc4random_uniform(4)];
        msg1(void, CGFloat, cells[i], cellSels[2], velocity);
        msg1(void, CGFloat, cells[i], cellSels[3], M_PI);
        msg1(void, CGFloat, cells[i], cellSels[4], 0.5);
        msg1(void, CGFloat, cells[i], cellSels[5], 3.5);
        msg1(void, CGColorRef, cells[i], cellSels[6], shapeColors[i >> 2]);
        msg1(void, id, cells[i], cellSels[7], images[i % 4]);
        msg1(void, CGFloat, cells[i], cellSels[8], 0.25);
        msg1(void, CGFloat, cells[i], cellSels[9], 0.1);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, &retainedArrCallbacks);
    id layer = createNew(objc_getClass("CAEmitterLayer"));
    msg1(void, id, getLayer(confettiView), sel_getUid("addSublayer:"), layer);
    msg1(void, CGPoint, layer,
         sel_getUid("setEmitterPosition:"), ((CGPoint){frame.size.width * 0.5, 0}));
    msg1(void, id, layer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    msg1(void, CGSize, layer, sel_getUid("setEmitterSize:"), ((CGSize){frame.size.width - 16, 1}));
    msg1(void, CFArrayRef, layer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    releaseObj(layer);
    for (int i = 0; i < 16; ++i) {
        releaseObj(cells[i]);
    }
    addSubview(view, confettiView);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5000000000),
                   dispatch_get_main_queue(), ^(void) {
        removeView(confettiView);
        releaseObj(confettiView);
        CFBundleRef bundle = CFBundleGetMainBundle();
        id ctrl = createAlertController(CFBundleCopyLocalizedString(bundle, CFSTR("homeAlertTitle"),
                                                                    NULL, NULL),
                                        CFBundleCopyLocalizedString(bundle, CFSTR("homeAlertMessage"),
                                                                    NULL, NULL));
        addAlertAction(ctrl, CFBundleCopyLocalizedString(bundle, CFSTR("ok"), NULL, NULL), 0, NULL);
        presentVC(ctrl);
    });
}

void homeVC_handleFinishedWorkout(id self, unsigned char completed) {
    HomeVC *data = (HomeVC *) ((char *)self + VCSize);
    int total = 0;
    for (int i = 0; i < 7; ++i) {
        if ((1 << i) & completed)
            ++total;
    }
    homeVC_updateWorkoutsList(data, completed);
    if (data->numWorkouts == total) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2500000000),
                       dispatch_get_main_queue(), ^(void) {
            showConfetti(self);
        });
    }
}
