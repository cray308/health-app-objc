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

static id stepperViewInit(CFBundleRef bundle, StepperView **ref, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(StepperViewClass, Sels.nw);
    StepperView *data = (StepperView *)((char *)self + ViewSize);
    *ref = data;
    data->stxt = tbl->label.stxt;
    data->setText = tbl->label.setText;
    tbl->view.setIsAcc(self, tbl->view.sace, true);
    tbl->view.setTraits(self, tbl->view.satrs, UIAccessibilityTraitAdjustable);
    CFStringRef stepperDescr = localize(bundle, CFSTR("stepperLabelDescr"));
    tbl->view.setAcc(self, tbl->view.sacl, stepperDescr);
    CFRelease(stepperDescr);
    CFStringRef repsStr = localize(bundle, CFSTR("stepperLabelInit"));
    data->range = CFStringFind(repsStr, CFSTR("1"), 0);
    data->repsStr = CFStringCreateMutableCopy(NULL, CFStringGetLength(repsStr) + 2, repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), repsStr);
    tbl->cc.activateC(tbl->cc.init(tbl->cc.cls, tbl->cc.cr, self, 8, 1, nil, 0, 1, 44), tbl->cc.ac, true);
    data->label = createLabel(tbl, clr, repsStr, UIFontTextStyleBody, 1);
    data->stepper = Sels.new(objc_getClass("UIStepper"), Sels.nw);
    msg1(void, double, data->stepper, sel_getUid("setValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMinimumValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMaximumValue:"), 10);
    tbl->button.addTarget(data->stepper, tbl->button.atgt, self, sel_getUid("stepperChanged"), 4096);
    id stack = createStackView(tbl, (id []){data->label, data->stepper}, 2, 0, 8, (Padding){20, 8, 0, 8});
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

    CFBundleRef bundle = CFBundleGetMainBundle();
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg, sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBG));

    tbl->stack.setMargins(sup->vStack, tbl->stack.smr, (HAInsets){.top = 20});
    CFStringRef liftKey = formatStr(CFSTR("liftTypes%d"), data->index);
    CFStringRef liftVal = localize(bundle, liftKey);
    CFRelease(liftKey);
    CFStringRef fieldKey = localize(bundle, CFSTR("maxWeight"));
    inputVC_addChild(self, formatStr(fieldKey, liftVal), 1, 999);
    CFRelease(liftVal);
    CFRelease(fieldKey);

    id stepperView = stepperViewInit(bundle, &data->stack, tbl, sup->clr);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, stepperView);

    sup->button = createButton(tbl, sup->clr, localize(bundle, CFSTR("finish")),
                               ColorBlue, UIFontTextStyleBody, 0, self, sel_getUid("tappedFinish"));
    setNavButtons(self, (id []){nil, sup->button});
    tbl->button.setEnabled(sup->button, tbl->button.en, false);
    if (objc_getClass("UINavigationBarAppearance"))
        msg1(void, bool, self, sel_getUid("setModalInPresentation:"), true);
}

static void handleNewStepperValue(id self, StepperView *data, int value) {
    CFStringRef newVal = formatStr(CFSTR("%d"), value);
    CFStringReplace(data->repsStr, data->range, newVal);
    CFRelease(newVal);
    data->setText(data->label, data->stxt, data->repsStr);
    if (value == 10) {
        data->range.length = 2;
    } else if (data->range.length == 2) {
        data->range.length = 1;
    }
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
    int initWeight = (sup->children[0]->result + extra) * 36;
    float reps = 37.f - (float)msg0(double, data->stack->stepper, sel_getUid("value"));
    short weight = (short)((initWeight / reps) + 0.5f) - extra;
    void *parent = data->parent;
    int index = data->index;
    dismissPresentedVC(^{ workoutVC_finishedBottomSheet(parent, index, weight); });
}
