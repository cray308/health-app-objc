#include "UpdateMaxesVC.h"
#include "AppDelegate.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

void workoutVC_finishedBottomSheet(void *self, int index, short weight);

Class UpdateMaxesVCClass;

id updateMaxesVC_init(void *parent, int index, short bodyweight, VCacheRef tbl, CCacheRef clr) {
    id self = msg2(id, VCacheRef, CCacheRef, Sels.alloc(UpdateMaxesVCClass, Sels.alo),
                   sel_getUid("initWithVCache:cCache:"), tbl, clr);
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    data->parent = parent;
    data->index = index;
    data->bodyweight = bodyweight;
    CFStringRef repsStr = localize(CFBundleGetMainBundle(), CFSTR("stepperLabelInit"));
    data->range = CFStringFind(repsStr, CFSTR("1"), 0);
    data->repsStr = CFStringCreateMutableCopy(NULL, CFStringGetLength(repsStr) + 2, repsStr);
    CFRelease(repsStr);
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    CFRelease(data->repsStr);
    Sels.viewRel(data->stepper, Sels.rel);
    Sels.viewRel(data->stepperLabel, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
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
    CFStringRef fieldKey = formatStr(CFSTR("maxWeight%d"), data->index);
    inputVC_addChild(self, localize(bundle, fieldKey), 1, 999);
    CFRelease(fieldKey);

    CFRetain(data->repsStr);
    data->stepperLabel = createLabel(tbl, sup->clr, data->repsStr, UIFontTextStyleBody, true);
    data->stepper = Sels.new(objc_getClass("UIStepper"), Sels.nw);
    msg1(void, double, data->stepper, sel_getUid("setValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMinimumValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMaximumValue:"), 10);
    tbl->button.addTarget(data->stepper, tbl->button.atgt, self, sel_getUid("stepperChanged"), 4096);
    id stepperStack = createStackView(tbl, (id []){data->stepperLabel, data->stepper}, 2, 0, 0, 8,
                                      (Padding){20, 8, 0, 8});
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, stepperStack);
    Sels.viewRel(stepperStack, Sels.rel);

    sup->button = createButton(tbl, sup->clr, localize(bundle, CFSTR("finish")),
                               ColorBlue, UIFontTextStyleBody, 0, self, sel_getUid("tappedFinish"));
    setNavButtons(self, (id []){nil, sup->button});
    tbl->button.setEnabled(sup->button, tbl->button.en, false);
}

void updateMaxesVC_updatedStepper(id self, SEL _cmd _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    int value = (int)msg0(double, data->stepper, sel_getUid("value"));
    CFStringRef newVal = formatStr(CFSTR("%d"), value);
    CFStringReplace(data->repsStr, data->range, newVal);
    CFRelease(newVal);
    sup->tbl->label.setText(data->stepperLabel, sup->tbl->label.stxt, data->repsStr);
    if (value == 10) {
        data->range.length = 2;
    } else if (data->range.length == 2) {
        data->range.length = 1;
    }
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    UpdateMaxesVC *data = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    short extra = data->index == LiftPullup ? data->bodyweight : 0;
    int initWeight = (sup->children[0]->result + extra) * 36;
    float reps = 37.f - (float)msg0(double, data->stepper, sel_getUid("value"));
    short weight = (short)((initWeight / reps) + 0.5f) - extra;
    void *parent = data->parent;
    int index = data->index;
    dismissPresentedVC(^{ workoutVC_finishedBottomSheet(parent, index, weight); });
}
