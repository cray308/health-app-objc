#include "HomeVC.h"
#include <CoreGraphics/CoreGraphics.h>
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "Views.h"
#include "WorkoutVC.h"

extern id kCAEmitterLayerLine;

Class HomeVCClass;

#pragma mark - Public Functions

void homeVC_updateWorkoutsList(HomeVC *d, uint8_t completedWorkouts) {
    id gray = getColor(ColorGray), green = getColor(ColorGreen);
    CFArrayRef views = getArrangedSubviews(d->planContainer.data->stack);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        StatusView *v = getIVV(StatusView, CFArrayGetValueAtIndex(views, i));
        bool enable = !(completedWorkouts & (1 << getTag(v->button)));
        setEnabled(v->button, enable);
        setBackgroundColor(v->box, enable ? gray : green);
    }
}

void homeVC_createWorkoutsList(id self, UserData const *data) {
    HomeVC *d = getIVVC(HomeVC, self);

    CFArrayRef views = getArrangedSubviews(d->planContainer.data->stack);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        removeFromSuperview((id)CFArrayGetValueAtIndex(views, i));
    }

    bool shouldHide = data->plan > MaxValidChar || data->planStart > time(NULL);
    setHidden(d->planContainer.view, shouldHide);
    setHidden(d->customContainer.data->divider, shouldHide);
    if (shouldHide) return;

    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFDateFormatterRef formatter = CFDateFormatterCreate(NULL, locale, 0, 0);
    CFDateFormatterSetFormat(formatter, CFSTR("EEEE"));
    CFRelease(locale);
    CFStringRef workoutNames[6] = {0};
    getWeeklyWorkoutNames(workoutNames, data->plan);
    SEL tapSel = getTapSel();
    StatusView *sv;
    time_t date = data->weekStart + (DaySeconds >> 1);

    for (int i = 0; i < 6; ++i, date += DaySeconds) {
        if (!workoutNames[i]) continue;
        id btn = statusView_init(&sv, formatDate(formatter, date), workoutNames[i], i, self, tapSel);
        addArrangedSubview(d->planContainer.data->stack, btn);
        releaseView(btn);
    }
    CFRelease(formatter);

    homeVC_updateWorkoutsList(d, data->completedWorkouts);
}

void homeVC_updateColors(id self) {
    HomeVC *d = getIVVC(HomeVC, self);
    id labelColor = getColor(ColorLabel), gray = getColor(ColorGray), green = getColor(ColorGreen);
    setBackgroundColor(getView(self), getColor(ColorPrimaryBGGrouped));
    containerView_updateColors(d->planContainer.data);
    containerView_updateColors(d->customContainer.data);
    CFArrayRef views = getArrangedSubviews(d->planContainer.data->stack);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        StatusView *v = getIVV(StatusView, CFArrayGetValueAtIndex(views, i));
        setTextColor(v->header, labelColor);
        updateButtonColors(v->button, ColorLabel);
        setBackgroundColor(v->box, isEnabled(v->button) ? gray : green);
    }
    views = getArrangedSubviews(d->customContainer.data->stack);
    for (int i = 0; i < 5; ++i) {
        StatusView *v = getIVV(StatusView, CFArrayGetValueAtIndex(views, i));
        updateButtonColors(v->button, ColorLabel);
    }
}

#pragma mark - Selectors/Methods

void homeVC_viewDidLoad(id self, SEL _cmd) {
    msgSupV(supSig(), self, VC, _cmd);

    HomeVC *d = getIVVC(HomeVC, self);
    setupNavItem(self, CFSTR("tabs0"), NULL);

    CFStringRef titles[5] = {[0] = localize(CFSTR("homeTestMax"))}, headers[2];
    fillStringArray(&titles[1], CFSTR("workoutTypes%d"), 4);
    fillStringArray(headers, CFSTR("homeHeader%d"), 2);

    d->planContainer.view = containerView_init(&d->planContainer.data, headers[0]);
    setHidden(d->planContainer.data->divider, true);
    CVPair customContainer;
    customContainer.view = containerView_init(&customContainer.data, headers[1]);
    setSpacing(customContainer.data->stack, ViewSpacing);
    memcpy(&d->customContainer, &customContainer, sizeof(CVPair));

    SEL tapSel = getCustomButtonSel();
    StatusView *sv;
    for (int i = 0; i < 5; ++i) {
        id btn = statusView_init(&sv, NULL, titles[i], i, self, tapSel);
        setHidden(sv->box, true);
        addArrangedSubview(customContainer.data->stack, btn);
        releaseView(btn);
    }

    id vStack = createVStack((id []){d->planContainer.view, customContainer.view}, 2);
    setSpacing(vStack, GroupSpacing);
    setupHierarchy(self, vStack, createScrollView(), ColorPrimaryBGGrouped);

    homeVC_createWorkoutsList(self, getUserData());
}

void homeVC_navigateToWorkout(id self, Workout *workout) {
    id workoutVC = workoutVC_init(workout);
    id navVC = getNavVC(self);
    msgV(objSig(void, id, bool), navVC, sel_getUid("pushViewController:animated:"), workoutVC, true);
    releaseVC(workoutVC);
}

void homeVC_planButtonTapped(id self, SEL _cmd _U_, id button) {
    UserData const *data = getUserData();
    homeVC_navigateToWorkout(self, getWeeklyWorkout((int)getTag(button), data->plan, data->lifts));
}

void homeVC_customButtonTapped(id self, SEL _cmd _U_, id button) {
    uint8_t index = (uint8_t)getTag(button);
    if (!index) {
        WorkoutParams params = {StrengthIndexTestMax, 1, 1, 100, WorkoutStrength, UCHAR_MAX};
        homeVC_navigateToWorkout(self, getWorkoutFromLibrary(&params, getUserData()->lifts));
        return;
    }

    presentModalVC(self, setupWorkoutVC_init(self, --index));
}

#pragma mark - Navigate Back From Workout

static void showConfetti(id self) {
    SEL cgColor = sel_getUid("CGColor");
    SEL iimn = sel_getUid("imageNamed:"), icgi = sel_getUid("CGImage");
    id (*imageNamed)(Class, SEL, CFStringRef) =
      (id(*)(Class, SEL, CFStringRef))getClassMethodImp(Image, iimn);
    CGImageRef (*getCGImage)(id, SEL) =
      (CGImageRef(*)(id, SEL))class_getMethodImplementation(Image, icgi);

    CGColorRef colors[] = {
        msgV(objSig(CGColorRef), getColor(ColorRed), cgColor),
        msgV(objSig(CGColorRef), getColor(ColorBlue), cgColor),
        msgV(objSig(CGColorRef), getColor(ColorGreen), cgColor),
        msgV(objSig(CGColorRef), getColor(ColorOrange), cgColor)
    };
    CGImageRef images[] = {
        getCGImage(imageNamed(Image, iimn, CFSTR("cv0")), icgi),
        getCGImage(imageNamed(Image, iimn, CFSTR("cv1")), icgi),
        getCGImage(imageNamed(Image, iimn, CFSTR("cv2")), icgi),
        getCGImage(imageNamed(Image, iimn, CFSTR("cv3")), icgi)
    };

    Class cellClass = objc_getClass("CAEmitterCell");
    SEL cellSels[10] = {
        sel_getUid("setBirthRate:"), sel_getUid("setLifetime:"), sel_getUid("setVelocity:"),
        sel_getUid("setEmissionLongitude:"), sel_getUid("setEmissionRange:"), sel_getUid("setSpin:"),
        sel_getUid("setColor:"), sel_getUid("setContents:"),
        sel_getUid("setScaleRange:"), sel_getUid("setScale:")
    };

    int const velocities[] = {100, 90, 150, 200};
    id cells[16];

    for (int i = 0; i < 16; ++i) {
        cells[i] = new(cellClass);
        msgV(objSig(void, float), cells[i], cellSels[0], 4);
        msgV(objSig(void, float), cells[i], cellSels[1], 14);
        int velocity = velocities[arc4random_uniform(4)];
        msgV(objSig(void, CGFloat), cells[i], cellSels[2], velocity);
        msgV(objSig(void, CGFloat), cells[i], cellSels[3], M_PI);
        msgV(objSig(void, CGFloat), cells[i], cellSels[4], 0.5);
        msgV(objSig(void, CGFloat), cells[i], cellSels[5], 3.5);
        msgV(objSig(void, CGColorRef), cells[i], cellSels[6], colors[i >> 2]);
        msgV(objSig(void, id), cells[i], cellSels[7], (id)images[i % 4]);
        msgV(objSig(void, CGFloat), cells[i], cellSels[8], 0.25);
        msgV(objSig(void, CGFloat), cells[i], cellSels[9], 0.1);
    }

    id view = getView(self);
    CGRect bounds;
    getBounds(bounds, view);

    id confetti = new(View);
    useConstraints(confetti);
    setBackgroundColor(confetti, msgV(objSig(id, CGFloat), getColor(ColorGray),
                                      sel_getUid("colorWithAlphaComponent:"), 0.8));

    id layer = new(objc_getClass("CAEmitterLayer"));
    msgV(objSig(void, id), getLayer(confetti), sel_getUid("addSublayer:"), layer);
    CGPoint position = {bounds.size.width * 0.5, 0};
    msgV(objSig(void, CGPoint), layer, sel_getUid("setEmitterPosition:"), position);
    msgV(objSig(void, id), layer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    CGSize size = {bounds.size.width - 16, 1};
    msgV(objSig(void, CGSize), layer, sel_getUid("setEmitterSize:"), size);
    CFArrayRef cellArr = CFArrayCreate(NULL, (const void **)cells, 16, &RetainedArrCallbacks);
    msgV(objSig(void, CFArrayRef), layer, sel_getUid("setEmitterCells:"), cellArr);

    CFRelease(cellArr);
    msgV(objSig(void), layer, ReleaseSel);
    for (int i = 0; i < 16; ++i) {
        releaseObject(cells[i]);
    }
    addSubview(view, confetti);
    pinToSafeArea(confetti, view);

    dispatch_after(dispatch_time(0, 5000000000), dispatch_get_main_queue(), ^{
        removeFromSuperview(confetti);
        releaseView(confetti);
        id alert = createAlert(CFSTR("homeAlert"), CFSTR("homeAlertMessage"));
        addAlertAction(alert, CFSTR("ok"), ActionStyleDefault, NULL);
        disableWindowTint();
        presentVC(self, alert);
    });
}

void homeVC_handleFinishedWorkout(id self, uint8_t completedWorkouts) {
    HomeVC *d = getIVVC(HomeVC, self);
    int totalCompleted = 0;
    for (int i = 0; i < 6; ++i) {
        if ((1 << i) & completedWorkouts) ++totalCompleted;
    }
    homeVC_updateWorkoutsList(d, completedWorkouts);
    if (CFArrayGetCount(getArrangedSubviews(d->planContainer.data->stack)) == totalCompleted) {
        dispatch_after(dispatch_time(0, 2500000000), dispatch_get_main_queue(), ^{
            showConfetti(self);
        });
    }
}
