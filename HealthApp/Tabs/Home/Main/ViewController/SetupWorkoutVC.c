#include "SetupWorkoutVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SetupWorkoutVCClass;
Ivar SetupWorkoutVCDataRef;

id setupWorkoutVC_init(id parent, unsigned char type, CFArrayRef names) {
    id self = createVC(SetupWorkoutVCClass);
    SetupWorkoutVCData *data = calloc(1, sizeof(SetupWorkoutVCData));
    data->parent = parent;
    data->names = names;
    data->type = type;
    object_setIvar(self, SetupWorkoutVCDataRef, (id) data);
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutVCData *data = (SetupWorkoutVCData *) object_getIvar(self, SetupWorkoutVCDataRef);
    struct objc_super super = {self, InputVCClass};
    CFRelease(data->names);
    releaseObj(data->workoutTextField);
    free(data);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    SetupWorkoutVCData *data = (SetupWorkoutVCData *) object_getIvar(self, SetupWorkoutVCDataRef);
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutTitle"));
    id workoutLabel = createLabel(pickerTitle, TextFootnote, 4, false);
    CFStringRef defaultTitle = CFArrayGetValueAtIndex(data->names, 0);
    data->workoutTextField = createTextfield(nil, defaultTitle, pickerTitle, 1, 0, 0);
    id workoutContainer = createStackView((id []){workoutLabel, data->workoutTextField},
                                          2, 1, 2, (Padding){30, 8, 20, 8});

    id _wkPicker = allocClass(objc_getClass("UIPickerView"));
    id workoutPicker = getObject(_wkPicker, sel_getUid("init"));
    setDelegate(workoutPicker, self);
    setObject(data->workoutTextField, sel_getUid("setInputView:"), workoutPicker);
    setInputAccessory(data->workoutTextField, parent->toolbar);

    addArrangedSubview(parent->vStack, workoutContainer);

    SEL tapSel = sel_getUid("buttonTapped:");
    id cancelButton = createButton(localize(CFSTR("cancel")), ColorBlue, 0, 0, self, tapSel, -1);
    parent->button = createButton(localize(CFSTR("go")), ColorBlue, 0, 1, self, tapSel, -1);

    CGRect frame;
    getRect(view, &frame, 0);
    setNavButton(self, true, cancelButton, frame.size.width);
    setNavButton(self, false, parent->button, frame.size.width);
    enableButton(parent->button, false);

    short maxes[] = {5, 5, 100};
    CFStringRef rows[] = {CFSTR("setupWorkoutSets"), CFSTR("setupWorkoutReps"), NULL};

    switch (data->type) {
        case WorkoutStrength:
            rows[2] = CFSTR("setupWorkoutMaxWeight");
            break;
        case WorkoutSE:
            maxes[0] = 3;
            maxes[1] = 50;
            break;
        case WorkoutEndurance:
            rows[0] = NULL;
            rows[1] = CFSTR("setupWorkoutDuration");
            maxes[1] = 180;
            break;
        default:
            rows[0] = rows[1] = NULL;
            enableButton(parent->button, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (!rows[i]) continue;
        inputVC_addChild(self, localize(rows[i]), 1, maxes[i]);
    }

    releaseObj(workoutLabel);
    releaseObj(workoutContainer);
    releaseObj(workoutPicker);
}

void setupWorkoutVC_tappedButton(id self, SEL _cmd _U_, id btn) {
    if (!getTag(btn)) {
        id presenter = getObject(self, sel_getUid("presentingViewController"));
        dismissPresentedVC(presenter, nil);
        return;
    }

    SetupWorkoutVCData *data = (SetupWorkoutVCData *) object_getIvar(self, SetupWorkoutVCDataRef);
    WorkoutParams output = {.day = -1, .type = data->type, .index = data->index};
    id *fields = ((InputVCData *) object_getIvar(self, InputVCDataRef))->children;
    switch (output.type) {
        case WorkoutStrength:
            output.weight = ((InputViewData *)
                             object_getIvar(fields[2], InputViewDataRef))->result;
        case WorkoutSE:
            output.sets = ((InputViewData *) object_getIvar(fields[0], InputViewDataRef))->result;
            output.reps = ((InputViewData *) object_getIvar(fields[1], InputViewDataRef))->result;
            break;

        case WorkoutEndurance:
            output.reps = ((InputViewData *) object_getIvar(fields[0], InputViewDataRef))->result;
        default:
            break;
    }
    Workout *w = exerciseManager_getWorkoutFromLibrary(&output);
    id parent = data->parent;
    dismissPresentedVC(parent, ^{
        homeVC_navigateToAddWorkout(parent, w);
    });
}

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id picker) {
    if (osVersion < 13)
        setBackground(picker, createColor(ColorTertiaryBG));
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long section _U_) {
    return CFArrayGetCount(((SetupWorkoutVCData *)
                            object_getIvar(self, SetupWorkoutVCDataRef))->names);
}

id setupWorkoutVC_attrTitleForRow(id self, SEL _cmd _U_,
                                  id picker _U_, long row, long section _U_) {
    SetupWorkoutVCData *data = (SetupWorkoutVCData *) object_getIvar(self, SetupWorkoutVCDataRef);
    id color = createColor(row == data->index ? ColorLabel : ColorSecondaryLabel);
    CFDictionaryRef dict = createTitleTextDict(color, createFont(TextTitle3));
    id attrString = createAttribString(CFArrayGetValueAtIndex(data->names, row), dict);
    voidFunc(attrString, sel_getUid("autorelease"));
    CFRelease(dict);
    return attrString;
}

CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd _U_,
                                       id picker _U_, long row, long section _U_) {
    CFArrayRef names = ((SetupWorkoutVCData *) object_getIvar(self, SetupWorkoutVCDataRef))->names;
    return CFArrayGetValueAtIndex(names, row);
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_, id picker, long row, long section _U_) {
    SetupWorkoutVCData *data = (SetupWorkoutVCData *) object_getIvar(self, SetupWorkoutVCDataRef);
    data->index = (int) row;
    CFStringRef name = CFArrayGetValueAtIndex(data->names, row);
    setLabelText(data->workoutTextField, name);
    if (osVersion < 13)
        setInt(picker, sel_getUid("reloadComponent:"), 0);
}
