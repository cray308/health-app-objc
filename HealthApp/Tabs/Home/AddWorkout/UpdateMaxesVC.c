#include "UpdateMaxesVC.h"
#include "AppDelegate.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

extern uint64_t UIAccessibilityTraitAdjustable;

Class StepperViewClass;
Class UpdateMaxesVCClass;

#pragma mark - Stepper

static id stepperView_init(StepperView **ref, CFLocaleRef l CF_CONSUMED, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(StepperViewClass, Sels.nw);
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    *ref = v;
    v->stxt = tbl->label.stxt;
    v->setText = tbl->label.setText;
    tbl->view.setIsAcc(self, tbl->view.sace, true);
    tbl->view.setTraits(self, tbl->view.satrs, UIAccessibilityTraitAdjustable);
    CFStringRef stepperDescr = localize(CFSTR("stepperLabelDescr"));
    tbl->view.setAcc(self, tbl->view.sacl, stepperDescr);
    CFRelease(stepperDescr);
    CFStringRef repsFmt = localize(CFSTR("stepperLabelInit"));
    CFStringRef repsStr = formatStr(l, repsFmt, 1);
    CFRelease(repsFmt);
    CFStringRef one = formatStr(l, CFSTR("%d"), 1);
    CFStringFindWithOptionsAndLocale(
      repsStr, one, (CFRange){0, CFStringGetLength(repsStr)}, 0, l, &v->range);
    CFRelease(one);
    CFRelease(l);
    v->repsStr = CFStringCreateMutableCopy(NULL, 64, repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), repsStr);
    setHeight(&tbl->cc, self, 44, true, false);
    v->label = createLabel(tbl, clr, repsStr, UIFontTextStyleBody, ColorLabel);
    v->stepper = Sels.new(objc_getClass("UIStepper"), Sels.nw);
    msg1(void, double, v->stepper, sel_getUid("setValue:"), 1);
    msg1(void, double, v->stepper, sel_getUid("setMinimumValue:"), 1);
    msg1(void, double, v->stepper, sel_getUid("setMaximumValue:"), 10);
    tbl->button.addTarget(v->stepper, tbl->button.atgt, self, sel_getUid("stepperChanged"), 4096);
    id stack = createHStack(tbl, (id []){v->label, v->stepper});
    msg1(void, bool, stack, tbl->view.trans, false);
    tbl->view.addSub(self, tbl->view.asv, stack);
    pin(&tbl->cc, stack, self);
    Sels.viewRel(stack, Sels.rel);
    return self;
}

void stepperView_deinit(id self, SEL _cmd) {
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    CFRelease(v->repsStr);
    Sels.viewRel(v->stepper, Sels.rel);
    Sels.viewRel(v->label, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

static void handleNewStepperValue(id self, StepperView *v, int value) {
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef newVal = formatStr(l, CFSTR("%d"), value);
    CFRelease(l);
    CFStringReplace(v->repsStr, v->range, newVal);
    v->range.length = CFStringGetLength(newVal);
    CFRelease(newVal);
    v->setText(v->label, v->stxt, v->repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), v->repsStr);
}

void stepperView_updatedStepper(id self, SEL _cmd _U_) {
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    handleNewStepperValue(self, v, (int)msg0(double, v->stepper, sel_getUid("value")));
}

static void stepperChangeGeneric(id self, int change) {
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    int value = (int)msg0(double, v->stepper, sel_getUid("value")) + change;
    if (value > 0 && value < 11) {
        msg1(void, double, v->stepper, sel_getUid("setValue:"), value);
        handleNewStepperValue(self, v, value);
    }
}

void stepperView_increment(id self, SEL _cmd _U_) { stepperChangeGeneric(self, 1); }

void stepperView_decrement(id self, SEL _cmd _U_) { stepperChangeGeneric(self, -1); }

#pragma mark - VC

id updateMaxesVC_init(void *parent, int index, int bodyweight, VCacheRef tbl, CCacheRef clr) {
    id self = msg2(id, VCacheRef, CCacheRef, Sels.alloc(UpdateMaxesVCClass, Sels.alo),
                   sel_getUid("initWithVCache:cCache:"), tbl, clr);
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    d->parent = parent;
    d->index = index;
    d->bodyweight = bodyweight;
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    Sels.viewRel((id)((char *)d->stack - ViewSize), Sels.rel);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBG));
    setVCTitle(navItem, localize(CFSTR("updateMaxesTitle")));

    tbl->stack.setSpace(sup->vStack, tbl->stack.ssp, 20);
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef liftKey = formatStr(NULL, CFSTR("exNames%02d"), d->index);
    CFStringRef liftVal = localize(liftKey);
    CFRelease(liftKey);
    CFMutableStringRef adjLift = CFStringCreateMutableCopy(NULL, 128, liftVal);
    CFRelease(liftVal);
    CFStringLowercase(adjLift, l);
    CFStringRef fieldKey = localize(CFSTR("maxWeight"));
    int kbType = getKeyboardForLocale(l);
    inputVC_addChild(self, formatStr(NULL, fieldKey, adjLift), kbType, 1, FieldMaxDefault);
    CFRelease(adjLift);
    CFRelease(fieldKey);

    id stepperView = stepperView_init(&d->stack, l, tbl, sup->clr);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, stepperView);

    sup->button = createButton(tbl, sup->clr, localize(CFSTR("finish")),
                               ColorBlue, UIFontTextStyleBody, self, sel_getUid("tappedFinish"));
    setNavButtons(navItem, (id []){nil, sup->button});
    tbl->button.setEnabled(sup->button, tbl->button.en, false);
    if (objc_getClass("UINavigationBarAppearance"))
        msg1(void, bool, self, sel_getUid("setModalInPresentation:"), true);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    CFLocaleRef locale = CFLocaleCopyCurrent();
    int extra = d->index == LiftPullup ? d->bodyweight : 0;
    float initWeight = ((sup->children[0].data->result * getSavedMassFactor(locale)) + extra) * 36;
    float reps = 37.f - (float)msg0(double, d->stack->stepper, sel_getUid("value"));
    int weight = (int)lrintf(initWeight / reps) - extra;
    CFRelease(locale);
    void *parent = d->parent;
    int index = d->index;
    dismissPresentedVC(^{ workoutVC_finishedBottomSheet(parent, index, weight); });
}
