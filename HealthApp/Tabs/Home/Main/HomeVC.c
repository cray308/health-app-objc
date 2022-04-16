#include "HomeVC.h"
#include <CoreFoundation/CFDateFormatter.h>
#include <CoreGraphics/CGColor.h>
#include <dispatch/queue.h>
#include <math.h>
#include <time.h>
#include "AppDelegate.h"
#include "StatusView.h"
#include "Views.h"

extern id kCAEmitterLayerLine;
void setWeeklyWorkoutNames(unsigned char plan, CFStringRef *names);
Workout *getWeeklyWorkout(unsigned char plan, int index);
id setupWorkoutVC_init(id parent, unsigned char type, VCacheRef tbl, CCacheRef clr);
id workoutVC_init(Workout *workout, VCacheRef tbl, CCacheRef clr);

Class HomeVCClass;
static CFStringRef dayNames[7];

id homeVC_init(VCacheRef tbl, CCacheRef clr, time_t startDate) {
    id self = Sels.new(HomeVCClass, Sels.nw);
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    data->tbl = tbl;
    data->clr = clr;
    const long diff = (long)kCFAbsoluteTimeIntervalSince1970;
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFDateFormatterRef f = CFDateFormatterCreate(NULL, l, 0, 0);
    CFRelease(l);
    CFDateFormatterSetFormat(f, CFSTR("EEEE"));
    for (int i = 0; i < 7; ++i, startDate += 86400) {
        dayNames[i] = CFDateFormatterCreateStringWithAbsoluteTime(NULL, f, startDate - diff);
    }
    CFRelease(f);
    return self;
}

void homeVC_updateWorkoutsList(HomeVC *self, unsigned char completed) {
    id gray = self->clr->getColor(self->clr->cls, self->clr->sc, ColorGray);
    id green = self->clr->getColor(self->clr->cls, self->clr->sc, ColorGreen);
    VCacheRef tbl = self->tbl;
    CFArrayRef views = tbl->stack.getSub(self->planContainer.data->stack, tbl->stack.gsv);
    for (int i = 0; i < self->numWorkouts; ++i) {
        id v = (id)CFArrayGetValueAtIndex(views, i);
        StatusView *ptr = (StatusView *)((char *)v + ViewSize);
        bool enable = !(completed & (1 << tbl->view.getTag(ptr->button, tbl->view.gtg)));
        tbl->button.setEnabled(ptr->button, tbl->button.en, enable);
        tbl->view.setBG(ptr->box, tbl->view.sbg, enable ? gray : green);
    }
}

void homeVC_createWorkoutsList(id self, const UserInfo *info) {
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    VCacheRef tbl = data->tbl;

    data->numWorkouts = 0;
    CFArrayRef views = tbl->stack.getSub(data->planContainer.data->stack, tbl->stack.gsv);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        tbl->view.rmSub((id)CFArrayGetValueAtIndex(views, i), tbl->view.rsv);
    }

    if ((info->currentPlan & 128) || info->planStart > time(NULL)) {
        tbl->view.hide(data->planContainer.view, tbl->view.shd, true);
        tbl->view.hide(data->customContainer->divider, tbl->view.shd, true);
        return;
    }

    CFStringRef workoutNames[7] = {0};
    setWeeklyWorkoutNames(info->currentPlan, workoutNames);

    SEL btnTap = sel_getUid("buttonTapped:");
    StatusView *sv;
    for (int i = 0; i < 7; ++i) {
        if (!workoutNames[i]) continue;
        id btn = statusView_init(tbl, data->clr, workoutNames[i], &sv, i, self, btnTap);
        tbl->label.setText(sv->headerLabel, tbl->label.stxt, dayNames[i]);
        statusView_updateAccessibility(sv, tbl);
        tbl->stack.addSub(data->planContainer.data->stack, tbl->stack.asv, btn);
        Sels.viewRel(btn, Sels.rel);
        data->numWorkouts += 1;
    }

    tbl->view.hide(data->planContainer.view, tbl->view.shd, false);
    tbl->view.hide(data->customContainer->divider, tbl->view.shd, false);
    homeVC_updateWorkoutsList(data, info->completedWorkouts);
}

void homeVC_updateColors(id self) {
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    CCacheRef clr = data->clr;
    VCacheRef tbl = data->tbl;
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    clr->getColor(clr->cls, clr->sc, ColorPrimaryBGGrouped));
    SEL enabled = sel_getUid("isEnabled");
    id bg = clr->getColor(clr->cls, clr->sc, ColorSecondaryBGGrouped);
    id label = clr->getColor(clr->cls, clr->sc, ColorLabel);
    id disabled = clr->getColor(clr->cls, clr->sc, ColorSecondaryLabel);
    id gray = clr->getColor(clr->cls, clr->sc, ColorGray);
    id green = clr->getColor(clr->cls, clr->sc, ColorGreen);
    containerView_updateColors(data->planContainer.data, tbl, clr);
    containerView_updateColors(data->customContainer, tbl, clr);
    CFArrayRef views = tbl->stack.getSub(data->planContainer.data->stack, tbl->stack.gsv);
    for (int i = 0; i < data->numWorkouts; ++i) {
        StatusView *ptr = (StatusView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize);
        tbl->label.setColor(ptr->headerLabel, tbl->label.stc, label);
        tbl->button.setColor(ptr->button, tbl->button.sbc, label, 0);
        tbl->button.setColor(ptr->button, tbl->button.sbc, disabled, 2);
        tbl->view.setBG(ptr->button, tbl->view.sbg, bg);
        tbl->view.setBG(ptr->box, tbl->view.sbg, msg0(bool, ptr->button, enabled) ? gray : green);
    }
    views = tbl->stack.getSub(data->customContainer->stack, tbl->stack.gsv);
    for (int i = 0; i < 5; ++i) {
        StatusView *ptr = (StatusView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize);
        tbl->button.setColor(ptr->button, tbl->button.sbc, label, 0);
        tbl->view.setBG(ptr->button, tbl->view.sbg, bg);
    }
}

void homeVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    CFBundleRef b = CFBundleGetMainBundle();
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    VCacheRef tbl = data->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg,
                    data->clr->getColor(data->clr->cls, data->clr->sc, ColorPrimaryBGGrouped));
    setVCTitle(msg0(id, self, sel_getUid("navigationItem")), localize(b, CFSTR("tabs0")));

    CFStringRef titles[5] = {[0] = localize(b, CFSTR("homeTestMax"))}, headers[2];
    fillStringArray(b, &titles[1], CFSTR("workoutTypes%d"), 4);
    fillStringArray(b, headers, CFSTR("homeHeader%d"), 2);

    data->planContainer.view = containerView_init(tbl, data->clr,
                                                  headers[0], &data->planContainer.data);
    tbl->view.hide(data->planContainer.data->divider, tbl->view.shd, true);
    id customContainer = containerView_init(tbl, data->clr, headers[1], &data->customContainer);
    tbl->stack.setSpace(data->customContainer->stack, tbl->stack.ssp, 4);
    id vStack = createVStack((id []){data->planContainer.view, customContainer}, 2);
    tbl->stack.setSpace(vStack, tbl->stack.ssp, 20);
    tbl->stack.setMargins(vStack, tbl->stack.smr, (HAInsets){16, 8, 16, 8});

    SEL btnTap = sel_getUid("customButtonTapped:");
    StatusView *sv;
    for (int i = 0; i < 5; ++i) {
        id btn = statusView_init(tbl, data->clr, titles[i], &sv, i, self, btnTap);
        tbl->view.hide(sv->box, tbl->view.shd, true);
        statusView_updateAccessibility(sv, tbl);
        tbl->stack.addSub(data->customContainer->stack, tbl->stack.asv, btn);
        Sels.viewRel(btn, Sels.rel);
    }

    id scrollView = createScrollView();
    addVStackToScrollView(tbl, view, vStack, scrollView);
    Sels.viewRel(vStack, Sels.rel);
    Sels.viewRel(scrollView, Sels.rel);

    homeVC_createWorkoutsList(self, getUserInfo());
}

void homeVC_navigateToAddWorkout(id self, void *workout) {
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    id vc = workoutVC_init(workout, data->tbl, data->clr);
    msg2(void, id, bool, msg0(id, self, sel_getUid("navigationController")),
         sel_getUid("pushViewController:animated:"), vc, true);
    Sels.vcRel(vc, Sels.rel);
}

void homeVC_workoutButtonTapped(id self, SEL _cmd _U_, id btn) {
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    int tag = (int)data->tbl->view.getTag(btn, data->tbl->view.gtg);
    Workout *w = getWeeklyWorkout(getUserInfo()->currentPlan, tag);
    homeVC_navigateToAddWorkout(self, w);
}

void homeVC_customButtonTapped(id self, SEL _cmd _U_, id btn) {
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    unsigned char index = (unsigned char)data->tbl->view.getTag(btn, data->tbl->view.gtg);
    if (!index) {
        Workout *w = getWorkoutFromLibrary(&(WorkoutParams){2, 1, 1, 100, WorkoutStrength, 0xff});
        homeVC_navigateToAddWorkout(self, w);
        return;
    }

    presentModalVC(setupWorkoutVC_init(self, --index, data->tbl, data->clr));
}

static void showConfetti(id view, HomeVC *data) {
    VCacheRef tbl = data->tbl;
    CGRect frame;
#if defined(__arm64__)
    frame = msg0(CGRect, view, sel_getUid("frame"));
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&frame, view, sel_getUid("frame"));
#endif
    id confetti = msg1(id, CGRect, Sels.alloc(View, Sels.alo), sel_getUid("initWithFrame:"), frame);
    id bg = msg1(id, CGFloat, data->clr->getColor(data->clr->cls, data->clr->sc, ColorGray),
                 sel_getUid("colorWithAlphaComponent:"), 0.8);
    tbl->view.setBG(confetti, tbl->view.sbg, bg);

    Class cellClass = objc_getClass("CAEmitterCell");
    SEL cgSels[] = {sel_getUid("imageNamed:"), sel_getUid("CGImage"), sel_getUid("CGColor")};
    SEL cellSels[10] = {
        sel_getUid("setBirthRate:"), sel_getUid("setLifetime:"), sel_getUid("setVelocity:"),
        sel_getUid("setEmissionLongitude:"), sel_getUid("setEmissionRange:"), sel_getUid("setSpin:"),
        sel_getUid("setColor:"), sel_getUid("setContents:"),
        sel_getUid("setScaleRange:"), sel_getUid("setScale:")
    };

    int const velocities[] = {100, 90, 150, 200};
    const char *const colorNames[] = {
        "systemRedColor", "systemBlueColor", "systemGreenColor", "systemYellowColor"
    };
    Class Image = objc_getClass("UIImage");
    CFStringRef imgNames[] = {CFSTR("cv0"), CFSTR("cv1"), CFSTR("cv2"), CFSTR("cv3")};
    CGColorRef shapeColors[4];
    id images[4];
    for (int i = 0; i < 4; ++i) {
        id img = clsF1(id, CFStringRef, Image, cgSels[0], imgNames[i]);
        images[i] = (id)msg0(CGImageRef, img, cgSels[1]);
        id color = clsF0(id, data->clr->cls, sel_getUid(colorNames[i]));
        shapeColors[i] = msg0(CGColorRef, color, cgSels[2]);
    }

    id cells[16];
    for (int i = 0; i < 16; ++i) {
        cells[i] = Sels.new(cellClass, Sels.nw);
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
    id layer = Sels.new(objc_getClass("CAEmitterLayer"), Sels.nw);
    msg1(void, id, tbl->view.layer(confetti, tbl->view.glyr), sel_getUid("addSublayer:"), layer);
    msg1(void, CGPoint, layer,
         sel_getUid("setEmitterPosition:"), ((CGPoint){frame.size.width * 0.5, 0}));
    msg1(void, id, layer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    msg1(void, CGSize, layer, sel_getUid("setEmitterSize:"), ((CGSize){frame.size.width - 16, 1}));
    msg1(void, CFArrayRef, layer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    msg0(void, layer, Sels.rel);
    for (int i = 0; i < 16; ++i) {
        Sels.objRel(cells[i], Sels.rel);
    }
    tbl->view.addSub(view, tbl->view.asv, confetti);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5000000000), dispatch_get_main_queue(), ^{
        tbl->view.rmSub(confetti, tbl->view.rsv);
        Sels.viewRel(confetti, Sels.rel);
        CFBundleRef b = CFBundleGetMainBundle();
        id ctrl = createAlertController(localize(b, CFSTR("homeAlertTitle")),
                                        localize(b, CFSTR("homeAlertMessage")));
        addAlertAction(ctrl, localize(b, CFSTR("ok")), 0, NULL);
        presentVC(ctrl);
    });
}

void homeVC_handleFinishedWorkout(id self, unsigned char completed) {
    HomeVC *data = (HomeVC *)((char *)self + VCSize);
    int total = 0;
    for (int i = 0; i < 7; ++i) {
        if ((1 << i) & completed)
            ++total;
    }
    homeVC_updateWorkoutsList(data, completed);
    if (data->numWorkouts == total) {
        id view = msg0(id, self, sel_getUid("view"));
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2500000000), dispatch_get_main_queue(), ^{
            showConfetti(view, data);
        });
    }
}
