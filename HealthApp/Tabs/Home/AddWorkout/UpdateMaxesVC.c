#include "UpdateMaxesVC.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

Class UpdateMaxesVCClass;
Ivar UpdateMaxesVCDataRef;

id updateMaxesVC_init(id parent) {
    id self = createVC(UpdateMaxesVCClass);
    object_setIvar(self, UpdateMaxesVCDataRef, parent);
    return self;
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    CFStringRef titles[4];
    fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    setMargins(parent->vStack, ((HAInsets){.top = 20}));

    for (int i = 0; i < 4; ++i) {
        inputVC_addChild(self, titles[i], 1, 999);
    }

    CGRect frame;
    getRect(view, &frame, 0);
    parent->button = createButton(localize(CFSTR("finish")), ColorBlue, 0, 0, self,
                                  sel_getUid("tappedFinish"), -1);
    setNavButton(self, false, parent->button, frame.size.width);
    enableButton(parent->button, false);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_) {
    id parent = object_getIvar(self, UpdateMaxesVCDataRef);
    short *lifts = malloc(sizeof(short) << 2);
    id *fields = ((InputVCData *) object_getIvar(self, InputVCDataRef))->children;
    for (int i = 0; i < 4; ++i) {
        lifts[i] = ((InputViewData *) object_getIvar(fields[i], InputViewDataRef))->result;
    }

    dismissPresentedVC(parent, ^{
        workoutVC_handleFinishedWorkout(parent, lifts);
    });
}
