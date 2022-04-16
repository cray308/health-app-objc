#include "UpdateMaxesVC.h"
#include "AppDelegate.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

extern uint64_t UIAccessibilityTraitAdjustable;
void workoutVC_finishedBottomSheet(void *self, int index, short weight);

Class UpdateMaxesVCClass;
Class StepperViewClass;

id updateMaxesVC_init(void *parent, int index, short bodyweight, VCacheRef tbl, CCacheRef clr) {
    id self = msg2(id, VCacheRef, CCacheRef, Sels.alloc(UpdateMaxesVCClass, Sels.alo),
                   sel_getUid("initWithVCache:cCache:"), tbl, clr);
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    data->parent = parent;
    data->index = index;
    data->bodyweight = bodyweight;
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    Sels.viewRel((id)((char *)data->stack - ViewSize), Sels.rel);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

static id stepperViewInit(CFBundleRef b, StepperView **ref, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(StepperViewClass, Sels.nw);
    StepperView *data = (StepperView *)((char *)self + ViewSize);
    *ref = data;
    data->stxt = tbl->label.stxt;
    data->setText = tbl->label.setText;
    tbl->view.setIsAcc(self, tbl->view.sace, true);
    tbl->view.setTraits(self, tbl->view.satrs, UIAccessibilityTraitAdjustable);
    CFStringRef stepperDescr = localize(b, CFSTR("stepperLabelDescr"));
    tbl->view.setAcc(self, tbl->view.sacl, stepperDescr);
    CFRelease(stepperDescr);
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef repsFmt = localize(b, CFSTR("stepperLabelInit"));
    CFStringRef repsStr = formatStr(l, repsFmt, 1);
    CFRelease(repsFmt);
    CFStringRef one = formatStr(l, CFSTR("%d"), 1);
    CFStringFindWithOptionsAndLocale(
      repsStr, one, (CFRange){0, CFStringGetLength(repsStr)}, 0, l, &data->range);
    CFRelease(one);
    CFRelease(l);
    data->repsStr = CFStringCreateMutableCopy(NULL, 64, repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), repsStr);
    setHeight(&tbl->cc, self, 44, true, false);
    data->label = createLabel(tbl, clr, repsStr, UIFontTextStyleBody, ColorLabel);
    data->stepper = Sels.new(objc_getClass("UIStepper"), Sels.nw);
    msg1(void, double, data->stepper, sel_getUid("setValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMinimumValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMaximumValue:"), 10);
    tbl->button.addTarget(data->stepper, tbl->button.atgt, self, sel_getUid("stepperChanged"), 4096);
    id stack = createHStack(tbl, (id []){data->label, data->stepper});
    msg1(void, bool, stack, tbl->view.trans, false);
    tbl->view.addSub(self, tbl->view.asv, stack);
    pin(&tbl->cc, stack, self);
    Sels.viewRel(stack, Sels.rel);
    return self;
}

void stepperView_deinit(id self, SEL _cmd) {
    StepperView *data = (StepperView *)((char *)self + ViewSize);
    CFRelease(data->repsStr);
    Sels.viewRel(data->stepper, Sels.rel);
    Sels.viewRel(data->label, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    CFBundleRef b = CFBundleGetMainBundle();
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBG));
    setVCTitle(navItem, localize(b, CFSTR("updateMaxesTitle")));

    tbl->stack.setSpace(sup->vStack, tbl->stack.ssp, 20);
    CFStringRef liftKey = formatStr(NULL, CFSTR("exNames%02d"), data->index);
    CFStringRef liftVal = localize(b, liftKey);
    CFRelease(liftKey);
    CFStringRef fieldKey = localize(b, CFSTR("maxWeight"));
    inputVC_addChild(self, formatStr(NULL, fieldKey, liftVal), 1, 999);
    CFRelease(liftVal);
    CFRelease(fieldKey);

    id stepperView = stepperViewInit(b, &data->stack, tbl, sup->clr);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, stepperView);

    sup->button = createButton(tbl, sup->clr, localize(b, CFSTR("finish")),
                               ColorBlue, UIFontTextStyleBody, self, sel_getUid("tappedFinish"));
    setNavButtons(navItem, (id []){nil, sup->button});
    tbl->button.setEnabled(sup->button, tbl->button.en, false);
    if (objc_getClass("UINavigationBarAppearance"))
        msg1(void, bool, self, sel_getUid("setModalInPresentation:"), true);
}

static void handleNewStepperValue(id self, StepperView *data, int value) {
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef newVal = formatStr(l, CFSTR("%d"), value);
    CFRelease(l);
    CFStringReplace(data->repsStr, data->range, newVal);
    data->range.length = CFStringGetLength(newVal);
    CFRelease(newVal);
    data->setText(data->label, data->stxt, data->repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), data->repsStr);
}

void stepperView_updatedStepper(id self, SEL _cmd _U_) {
    StepperView *data = (StepperView *)((char *)self + ViewSize);
    handleNewStepperValue(self, data, (int)msg0(double, data->stepper, sel_getUid("value")));
}

static void stepperChangeGeneric(id self, int change) {
    StepperView *data = (StepperView *)((char *)self + ViewSize);
    int value = (int)msg0(double, data->stepper, sel_getUid("value")) + change;
    if (value > 0 && value < 11) {
        msg1(void, double, data->stepper, sel_getUid("setValue:"), value);
        handleNewStepperValue(self, data, value);
    }
}

void stepperView_increment(id self, SEL _cmd _U_) { stepperChangeGeneric(self, 1); }

void stepperView_decrement(id self, SEL _cmd _U_) { stepperChangeGeneric(self, -1); }

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    short extra = data->index == LiftPullup ? data->bodyweight : 0;
    int initWeight = (sup->children[0].data->result + extra) * 36;
    float reps = 37.f - (float)msg0(double, data->stack->stepper, sel_getUid("value"));
    short weight = (short)((initWeight / reps) + 0.5f) - extra;
    void *parent = data->parent;
    int index = data->index;
    dismissPresentedVC(^{ workoutVC_finishedBottomSheet(parent, index, weight); });
}
