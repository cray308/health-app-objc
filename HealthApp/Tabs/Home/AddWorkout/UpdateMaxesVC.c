#include "UpdateMaxesVC.h"
#include <CoreFoundation/CFString.h>
#include "InputVC.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

#define setDouble(_obj, _cmd, _arg) (((void(*)(id,SEL,double))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getStepperValue(_v) (((double(*)(id,SEL))objc_msgSend)((_v), sel_getUid("value")))

Class UpdateMaxesVCClass;
Ivar UpdateMaxesVCDataRef;

id updateMaxesVC_init(id parent, unsigned index) {
    id self = createVC(UpdateMaxesVCClass);
    UpdateMaxesVCData *data = malloc(sizeof(UpdateMaxesVCData));
    data->parent = parent;
    data->index = index;
    data->stepperFormat = localize(CFSTR("stepperLabelFormat"));
    object_setIvar(self, UpdateMaxesVCDataRef, (id) data);
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVCData *data = (UpdateMaxesVCData *) object_getIvar(self, UpdateMaxesVCDataRef);
    struct objc_super super = {self, InputVCClass};
    releaseObj(data->stepper);
    releaseObj(data->stepperLabel);
    free(data);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    UpdateMaxesVCData *data = (UpdateMaxesVCData *) object_getIvar(self, UpdateMaxesVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    setMargins(parent->vStack, ((HAInsets){.top = 20}));
    CFStringRef fieldKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), data->index);
    inputVC_addChild(self, localize(fieldKey), 1, 999);

    CFStringRef stepperText = CFStringCreateWithFormat(NULL, NULL, data->stepperFormat, 1);
    data->stepperLabel = createLabel(stepperText, TextBody, 4, true);
    data->stepper = createObjectWithFrame(objc_getClass("UIStepper"), CGRectZero);
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
                                  sel_getUid("tappedFinish"), -1);
    setNavButton(self, false, parent->button, frame.size.width);
    enableButton(parent->button, false);

    releaseObj(stepperStack);
    CFRelease(stepperText);
    CFRelease(fieldKey);
}

void updateMaxesVC_updatedStepper(id self, SEL _cmd _U_) {
    UpdateMaxesVCData *data = (UpdateMaxesVCData *) object_getIvar(self, UpdateMaxesVCDataRef);
    int value = (int) getStepperValue(data->stepper);
    CFStringRef stepperText = CFStringCreateWithFormat(NULL, NULL, data->stepperFormat, value);
    setLabelText(data->stepperLabel, stepperText);
    CFRelease(stepperText);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_) {
    UpdateMaxesVCData *data = (UpdateMaxesVCData *) object_getIvar(self, UpdateMaxesVCDataRef);
    id field = ((InputVCData *) object_getIvar(self, InputVCDataRef))->children[0];
    short extra = data->index == LiftPullup ? 145 : 0;
    int initWeight = ((InputViewData *) object_getIvar(field, InputViewDataRef))->result + extra;
    initWeight *= 36;
    float reps = 37.f - ((float) getStepperValue(data->stepper));
    short weight = (short) ((initWeight / reps) + 0.5f) - extra;
    id parent = data->parent;
    unsigned index = data->index;
    dismissPresentedVC(parent, ^{
        workoutVC_finishedBottomSheet(parent, index, weight);
    });
}
