#include "HomeVC.h"
#include "StatusView.h"
#include "Views.h"
#include "WorkoutVC.h"

extern id kCAEmitterLayerLine;

Class HomeVCClass;

static CFStringRef dayNames[7];

id homeVC_init(time_t startDate) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFDateFormatterRef fmt = CFDateFormatterCreate(NULL, locale, 0, 0);
    CFRelease(locale);
    CFDateFormatterSetFormat(fmt, CFSTR("EEEE"));
    for (int i = 0; i < 7; ++i, startDate += DaySeconds) {
        dayNames[i] = CFDateFormatterCreateStringWithAbsoluteTime(NULL, fmt, ToAppleTime(startDate));
    }
    CFRelease(fmt);
    return new(HomeVCClass);
}

#pragma mark - Public Functions

void homeVC_updateWorkoutsList(HomeVC *d, unsigned char completed) {
    id gray = getColor(ColorGray), green = getColor(ColorGreen);
    CFArrayRef views = getArrangedSubviews(d->planContainer.data->stack);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        StatusView *v = (StatusView *)getIVV(CFArrayGetValueAtIndex(views, i));
        bool enable = !(completed & (1 << getTag(v->button)));
        setEnabled(v->button, enable);
        setBackgroundColor(v->box, enable ? gray : green);
    }
}

void homeVC_createWorkoutsList(id self, const UserInfo *info) {
    HomeVC *d = (HomeVC *)getIVVC(self);

    CFArrayRef views = getArrangedSubviews(d->planContainer.data->stack);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        removeFromSuperview((id)CFArrayGetValueAtIndex(views, i));
    }

    if (info->currentPlan > MaxValidCharVal || info->planStart > time(NULL)) {
        setHidden(d->planContainer.view, true);
        setHidden(d->firstDiv, true);
        return;
    }

    CFStringRef workoutNames[7] = {0};
    setWeeklyWorkoutNames(info->currentPlan, workoutNames);

    SEL btnTap = sel_getUid("buttonTapped:");
    StatusView *sv;
    for (int i = 0; i < 7; ++i) {
        if (!workoutNames[i]) continue;
        id btn = statusView_init(&sv, dayNames[i], workoutNames[i], i, self, btnTap);
        statusView_updateAccessibility(sv);
        addArrangedSubview(d->planContainer.data->stack, btn);
        releaseV(btn);
    }

    setHidden(d->planContainer.view, false);
    setHidden(d->firstDiv, false);
    homeVC_updateWorkoutsList(d, info->completedWorkouts);
}

#pragma mark - Selectors/Methods

void homeVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    HomeVC *d = (HomeVC *)getIVVC(self);
    setupNavItem(self, CFSTR("tabs0"), NULL);

    CFStringRef titles[5] = {[0] = localize(CFSTR("homeTestMax"))}, headers[2];
    fillStringArray(&titles[1], CFSTR("workoutTypes%d"), 4);
    fillStringArray(headers, CFSTR("homeHeader%d"), 2);

    d->planContainer.view = containerView_init(&d->planContainer.data, headers[0]);
    setHidden(d->planContainer.data->divider, true);
    ContainerView *cc;
    id customContainer = containerView_init(&cc, headers[1]);
    setSpacing(cc->stack, ViewSpacing);
    d->firstDiv = msg0(id, cc->divider, sel_getUid("retain"));

    SEL btnTap = sel_getUid("customButtonTapped:");
    StatusView *sv;
    for (int i = 0; i < 5; ++i) {
        id btn = statusView_init(&sv, NULL, titles[i], i, self, btnTap);
        setHidden(sv->box, true);
        statusView_updateAccessibility(sv);
        addArrangedSubview(cc->stack, btn);
        releaseV(btn);
    }

    id vStack = createVStack((id []){d->planContainer.view, customContainer}, 2);
    setSpacing(vStack, GroupSpacing);
    setLayoutMargins(vStack, VCMargins);
    setupHierarchy(self, vStack, createScrollView(), ColorPrimaryBGGrouped);
    releaseV(customContainer);

    homeVC_createWorkoutsList(self, getUserInfo());
}

void homeVC_navigateToAddWorkout(id self, Workout *workout) {
    id vc = workoutVC_init(workout);
    id nav = msg0(id, self, sel_getUid("navigationController"));
    msg2(void, id, bool, nav, sel_getUid("pushViewController:animated:"), vc, true);
    releaseVC(vc);
}

void homeVC_workoutButtonTapped(id self, SEL _cmd _U_, id btn) {
    UserInfo const *info = getUserInfo();
    Workout *w = getWeeklyWorkout((int)getTag(btn), info->currentPlan, info->liftMaxes);
    homeVC_navigateToAddWorkout(self, w);
}

void homeVC_customButtonTapped(id self, SEL _cmd _U_, id btn) {
    unsigned char index = (unsigned char)getTag(btn);
    if (!index) {
        WorkoutParams params = {StrengthIndexTestMax, 1, 1, 100, WorkoutStrength, UCHAR_MAX};
        Workout *w = getWorkoutFromLibrary(&params, getUserInfo()->liftMaxes);
        homeVC_navigateToAddWorkout(self, w);
        return;
    }

    presentModalVC(self, setupWorkoutVC_init(self, --index));
}

#pragma mark - Navigate Back From Workout

static void showConfetti(id self) {
    Class Cell = objc_getClass("CAEmitterCell");
    SEL cgImg = sel_getUid("CGImage"), cgClr = sel_getUid("CGColor");
    SEL cellSels[] = {
        sel_getUid("setBirthRate:"), sel_getUid("setLifetime:"), sel_getUid("setVelocity:"),
        sel_getUid("setEmissionLongitude:"), sel_getUid("setEmissionRange:"), sel_getUid("setSpin:"),
        sel_getUid("setColor:"), sel_getUid("setContents:"),
        sel_getUid("setScaleRange:"), sel_getUid("setScale:")
    };
    int const velocities[] = {100, 90, 150, 200};
    CGColorRef shapeColors[] = {
        msg0(CGColorRef, getColor(ColorRed), cgClr), msg0(CGColorRef, getColor(ColorBlue), cgClr),
        msg0(CGColorRef, getColor(ColorGreen), cgClr), msg0(CGColorRef, getColor(ColorOrange), cgClr)
    };
    CGImageRef images[] = {
        msg0(CGImageRef, getImg(CFSTR("cv0")), cgImg), msg0(CGImageRef, getImg(CFSTR("cv1")), cgImg),
        msg0(CGImageRef, getImg(CFSTR("cv2")), cgImg), msg0(CGImageRef, getImg(CFSTR("cv3")), cgImg),
    };
    id cells[16];

    for (int i = 0; i < 16; ++i) {
        cells[i] = new(Cell);
        msg1(void, float, cells[i], cellSels[0], 4);
        msg1(void, float, cells[i], cellSels[1], 14);
        msg1(void, CGFloat, cells[i], cellSels[2], velocities[arc4random_uniform(4)]);
        msg1(void, CGFloat, cells[i], cellSels[3], M_PI);
        msg1(void, CGFloat, cells[i], cellSels[4], 0.5);
        msg1(void, CGFloat, cells[i], cellSels[5], 3.5);
        msg1(void, CGColorRef, cells[i], cellSels[6], shapeColors[i >> 2]);
        msg1(void, id, cells[i], cellSels[7], (id)images[i % 4]);
        msg1(void, CGFloat, cells[i], cellSels[8], 0.25);
        msg1(void, CGFloat, cells[i], cellSels[9], 0.1);
    }

    id view = msg0(id, self, sel_getUid("view"));
    CGRect frame;
    getRect(frame, view, sel_getUid("frame"));
    id confetti = new(View);
    setTrans(confetti);
    id bg = msg1(id, CGFloat, getColor(ColorGray), sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackgroundColor(confetti, bg);
    id layer = new(objc_getClass("CAEmitterLayer"));
    msg1(void, id, getLayer(confetti), sel_getUid("addSublayer:"), layer);
    CGPoint pos = {frame.size.width * 0.5, 0};
    msg1(void, CGPoint, layer, sel_getUid("setEmitterPosition:"), pos);
    msg1(void, id, layer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    msg1(void, CGSize, layer, sel_getUid("setEmitterSize:"), ((CGSize){frame.size.width - 16, 1}));
    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, &retainedArrCallbacks);
    msg1(void, CFArrayRef, layer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    msg0(void, layer, Sels.rel);
    for (int i = 0; i < 16; ++i) {
        releaseO(cells[i]);
    }
    addSubview(view, confetti);
    pin(confetti, view);

    dispatch_after(dispatch_time(0, 5000000000), dispatch_get_main_queue(), ^{
        removeFromSuperview(confetti);
        releaseV(confetti);
        id ctrl = createAlertController(CFSTR("homeAlertTitle"), CFSTR("homeAlertMessage"));
        addAlertAction(ctrl, CFSTR("ok"), UIAlertActionStyleDefault, NULL);
        showAlert(ctrl);
    });
}

void homeVC_handleFinishedWorkout(id self, unsigned char completed) {
    HomeVC *d = (HomeVC *)getIVVC(self);
    int total = 0;
    for (int i = 0; i < 7; ++i) {
        if ((1 << i) & completed) ++total;
    }
    homeVC_updateWorkoutsList(d, completed);
    if (CFArrayGetCount(getArrangedSubviews(d->planContainer.data->stack)) == total) {
        dispatch_after(dispatch_time(0, 2500000000), dispatch_get_main_queue(), ^{
            showConfetti(self);
        });
    }
}
