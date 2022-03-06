#include "UpdateMaxesVC.h"
#include "AppDelegate.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

#define getStepperValue(_v) msg0(double, _v, sel_getUid("value"))

void workoutVC_finishedBottomSheet(id self, int index, short weight);

Class UpdateMaxesVCClass;

id updateMaxesVC_init(id parent, int index, short bodyweight) {
    id self = createNew(UpdateMaxesVCClass);
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    data->parent = parent;
    data->index = index;
    data->bodyweight = bodyweight;
    data->stepperFormat = CFBundleCopyLocalizedString(CFBundleGetMainBundle(),
                                                      CFSTR("stepperLabelFormat"), NULL, NULL);
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    struct objc_super super = {self, InputVCClass};
    CFRelease(data->stepperFormat);
    releaseObj(data->stepper);
    releaseObj(data->stepperLabel);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    setMargins(parent->vStack, ((HAInsets){.top = 20}));
    CFStringRef fieldKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), data->index);
    inputVC_addChild(self, CFBundleCopyLocalizedString(bundle, fieldKey, NULL, NULL), 1, 999);
    CFRelease(fieldKey);

    data->stepperLabel = createLabel(CFStringCreateWithFormat(NULL, NULL, data->stepperFormat, 1),
                                     UIFontTextStyleBody, true);
    data->stepper = createNew(objc_getClass("UIStepper"));
    msg1(void, double, data->stepper, sel_getUid("setValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMinimumValue:"), 1);
    msg1(void, double, data->stepper, sel_getUid("setMaximumValue:"), 10);
    addTarget(data->stepper, self, sel_getUid("stepperChanged"), 4096);
    id stepperStack = createStackView((id []){data->stepperLabel, data->stepper}, 2, 0, 8,
                                      (Padding){20, 8, 0, 8});
    addArrangedSubview(parent->vStack, stepperStack);
    releaseObj(stepperStack);

    parent->button = createButton(CFBundleCopyLocalizedString(bundle, CFSTR("finish"), NULL, NULL),
                                  ColorBlue, 0, self, sel_getUid("tappedFinish"));
    setNavButtons(self, (id []){nil, parent->button});
    enableButton(parent->button, false);
}

void updateMaxesVC_updatedStepper(id self, SEL _cmd _U_) {
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    int value = (int) getStepperValue(data->stepper);
    CFStringRef stepperText = CFStringCreateWithFormat(NULL, NULL, data->stepperFormat, value);
    setLabelText(data->stepperLabel, stepperText);
    CFRelease(stepperText);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_) {
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    short extra = data->index == LiftPullup ? data->bodyweight : 0;
    int initWeight = (((InputVC *) ((char *)self + VCSize))->children[0]->result + extra) * 36;
    float reps = 37.f - ((float) getStepperValue(data->stepper));
    short weight = (short) ((initWeight / reps) + 0.5f) - extra;
    id parent = data->parent;
    int index = data->index;
    dismissPresentedVC(^{ workoutVC_finishedBottomSheet(parent, index, weight); });
}
