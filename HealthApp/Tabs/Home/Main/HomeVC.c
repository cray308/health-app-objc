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
    setHidden(d->divider, shouldHide);
    if (shouldHide) return;

    CFLocaleRef locale = copyLocale();
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
    setID(d->planContainer.view, CFSTR("planContainer"))
    CVPair customContainer;
    customContainer.view = containerView_init(&customContainer.data, headers[1]);
    setSpacing(customContainer.data->stack, ViewSpacing);
    setID(customContainer.view, CFSTR("customContainer"))
    d->divider = retainView(customContainer.data->divider);

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
    releaseView(customContainer.view);

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

    Class Cell = objc_getClass("CAEmitterCell");
    SEL esbr = sel_getUid("setBirthRate:"), esl = sel_getUid("setLifetime:");
    SEL esv = sel_getUid("setVelocity:"), esel = sel_getUid("setEmissionLongitude:");
    SEL eser = sel_getUid("setEmissionRange:"), ess = sel_getUid("setSpin:");
    SEL escl = sel_getUid("setColor:"), escn = sel_getUid("setContents:");
    SEL esscr = sel_getUid("setScaleRange:"), essc = sel_getUid("setScale:");
    void (*setBirthRate)(id, SEL, float) =
      (void(*)(id, SEL, float))class_getMethodImplementation(Cell, esbr);
    void (*setLifetime)(id, SEL, float) =
      (void(*)(id, SEL, float))class_getMethodImplementation(Cell, esl);
    void (*setVelocity)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Cell, esv);
    void (*setEmissionLongitude)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Cell, esel);
    void (*setEmissionRange)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Cell, eser);
    void (*setSpin)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Cell, ess);
    void (*setColor)(id, SEL, CGColorRef) =
      (void(*)(id, SEL, CGColorRef))class_getMethodImplementation(Cell, escl);
    void (*setContents)(id, SEL, id) =
      (void(*)(id, SEL, id))class_getMethodImplementation(Cell, escn);
    void (*setScaleRange)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Cell, esscr);
    void (*setScale)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Cell, essc);

    int velocities[] = {100, 90, 150, 200};
    CFMutableArrayRef cells = CFArrayCreateMutable(NULL, 16, &kCFTypeArrayCallBacks);

    for (int i = 0; i < 16; ++i) {
        id cell = new(Cell);
        setBirthRate(cell, esbr, 4);
        setLifetime(cell, esl, 14);
        setVelocity(cell, esv, velocities[arc4random_uniform(4)]);
        setEmissionLongitude(cell, esel, M_PI);
        setEmissionRange(cell, eser, 0.5);
        setSpin(cell, ess, 3.5);
        setColor(cell, escl, colors[i >> 2]);
        setContents(cell, escn, (id)images[i % 4]);
        setScaleRange(cell, esscr, 0.25);
        setScale(cell, essc, 0.1);
        CFArrayAppendValue(cells, cell);
        releaseObject(cell);
    }

    id view = getView(self);
    CGRect bounds;
    getBounds(bounds, view);

    id confetti = new(View);
    useConstraints(confetti);
    setID(confetti, CFSTR("confetti"))
    setBackgroundColor(confetti, msgV(objSig(id, CGFloat), getColor(ColorGray),
                                      sel_getUid("colorWithAlphaComponent:"), 0.8));

    id layer = new(objc_getClass("CAEmitterLayer"));
    msgV(objSig(void, id), getLayer(confetti), sel_getUid("addSublayer:"), layer);
    CGPoint position = {bounds.size.width * 0.5, 0};
    msgV(objSig(void, CGPoint), layer, sel_getUid("setEmitterPosition:"), position);
    msgV(objSig(void, id), layer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    CGSize size = {bounds.size.width - 16, 1};
    msgV(objSig(void, CGSize), layer, sel_getUid("setEmitterSize:"), size);
    msgV(objSig(void, CFArrayRef), layer, sel_getUid("setEmitterCells:"), cells);

    CFRelease(cells);
    msgV(objSig(void), layer, ReleaseSel);
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
