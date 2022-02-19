#include "UpdateMaxesVC.h"
#include <CoreFoundation/CFString.h>
#include "InputVC.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

#define setDouble(_obj, _cmd, _arg) (((void(*)(id,SEL,double))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getStepperValue(_v) (((double(*)(id,SEL))objc_msgSend)((_v), sel_getUid("value")))

Class UpdateMaxesVCClass;

id updateMaxesVC_init(id parent, int index, short bodyweight) {
    id self = createNew(UpdateMaxesVCClass);
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    data->parent = parent;
    data->index = index;
    data->bodyweight = bodyweight;
    data->stepperFormat = localize(CFSTR("stepperLabelFormat"));
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    struct objc_super super = {self, InputVCClass};
    releaseObj(data->stepper);
    releaseObj(data->stepperLabel);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    UpdateMaxesVC *data = (UpdateMaxesVC *) ((char *)self + InputVCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    setMargins(parent->vStack, ((HAInsets){.top = 20}));
    CFStringRef fieldKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), data->index);
    inputVC_addChild(self, localize(fieldKey), 1, 999);

    CFStringRef stepperText = CFStringCreateWithFormat(NULL, NULL, data->stepperFormat, 1);
    data->stepperLabel = createLabel(stepperText, TextBody, true);
    data->stepper = createNew(objc_getClass("UIStepper"));
    setDouble(data->stepper, sel_getUid("setValue:"), 1);
    setDouble(data->stepper, sel_getUid("setMinimumValue:"), 1);
    setDouble(data->stepper, sel_getUid("setMaximumValue:"), 10);
    addTarget(data->stepper, self, sel_getUid("stepperChanged"), 4096);
    id stepperStack = createStackView((id []){data->stepperLabel, data->stepper}, 2, 0, 8,
                                      (Padding){20, 8, 0, 8});
    addArrangedSubview(parent->vStack, stepperStack);

    CGRect frame;
    getRect(view, &frame, 0);
    parent->button = createButton(localize(CFSTR("finish")), ColorBlue, 0, 0, self,
                                  sel_getUid("tappedFinish"));
    setNavButton(self, false, parent->button, (int) frame.size.width);
    enableButton(parent->button, false);

    releaseObj(stepperStack);
    CFRelease(stepperText);
    CFRelease(fieldKey);
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
    id field = ((InputVC *) ((char *)self + VCSize))->children[0];
    short extra = data->index == LiftPullup ? data->bodyweight : 0;
    int initWeight = (((InputView *) ((char *)field + ViewSize))->result + extra) * 36;
    float reps = 37.f - ((float) getStepperValue(data->stepper));
    short weight = (short) ((initWeight / reps) + 0.5f) - extra;
    id parent = data->parent;
    int index = data->index;
    dismissPresentedVC(parent, ^{
        workoutVC_finishedBottomSheet(parent, index, weight);
    });
}
