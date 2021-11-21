#include "SetupWorkoutModal.h"
#include "HomeTabCoordinator.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SetupWorkoutModalClass;
Ivar SetupWorkoutModalDataRef;

id setupWorkoutVC_init(void *delegate, unsigned char type, CFArrayRef names) {
    id self = createVC(SetupWorkoutModalClass);
    SetupWorkoutModalData *data = calloc(1, sizeof(SetupWorkoutModalData));
    data->delegate = delegate;
    data->names = names;
    workoutParams_init(&data->output, -1);
    data->output.type = type;
    object_setIvar(self, SetupWorkoutModalDataRef, (id) data);
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutModalData *data =
    (SetupWorkoutModalData *) object_getIvar(self, SetupWorkoutModalDataRef);
    struct objc_super super = {self, InputVCClass};
    CFRelease(data->names);
    releaseObj(data->workoutTextField);
    free(data);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    SetupWorkoutModalData *data =
    (SetupWorkoutModalData *) object_getIvar(self, SetupWorkoutModalDataRef);
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutTitle"));
    id workoutLabel = createLabel(pickerTitle, TextFootnote, 4, false);
    CFStringRef defaultTitle = CFArrayGetValueAtIndex(data->names, 0);
    data->workoutTextField = createTextfield(nil, defaultTitle, pickerTitle, 1, 0, 0);
    id workoutContainer = createStackView((id []){workoutLabel, data->workoutTextField},
                                          2, 1, 2, (Padding){30, 8, 0, 8});

    id _wkPicker = allocClass(objc_getClass("UIPickerView"));
    id workoutPicker = getObject(_wkPicker, sel_getUid("init"));
    setDelegate(workoutPicker, self);
    setObject(data->workoutTextField, sel_getUid("setInputView:"), workoutPicker);
    setInputAccessory(data->workoutTextField, parent->toolbar);

    addArrangedSubview(parent->vStack, workoutContainer);
    setSpacingAfter(parent->vStack, workoutContainer, 20);

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

    switch (data->output.type) {
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

    SetupWorkoutModalData *data =
    (SetupWorkoutModalData *) object_getIvar(self, SetupWorkoutModalDataRef);
    WorkoutParams *output = &data->output;
    id *fields = ((InputVCData *) object_getIvar(self, InputVCDataRef))->children;
    switch (output->type) {
        case WorkoutStrength:
            output->weight = ((InputViewData *)
                              object_getIvar(fields[2], InputViewDataRef))->result;
        case WorkoutSE:
            output->sets = ((InputViewData *) object_getIvar(fields[0], InputViewDataRef))->result;
            output->reps = ((InputViewData *) object_getIvar(fields[1], InputViewDataRef))->result;
            break;

        case WorkoutEndurance:
            output->reps = ((InputViewData *) object_getIvar(fields[0], InputViewDataRef))->result;
        default: ;
    }
    homeCoordinator_finishedSettingUpCustomWorkout(data->delegate, output);
}

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id picker _U_) {
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long section _U_) {
    return CFArrayGetCount(((SetupWorkoutModalData *)
                            object_getIvar(self, SetupWorkoutModalDataRef))->names);
}

CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd _U_,
                                       id picker _U_, long row, long section _U_) {
    CFArrayRef names = ((SetupWorkoutModalData *)
                        object_getIvar(self, SetupWorkoutModalDataRef))->names;
    return CFArrayGetValueAtIndex(names, row);
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_, id picker _U_, long row, long section _U_) {
    SetupWorkoutModalData *data =
    (SetupWorkoutModalData *) object_getIvar(self, SetupWorkoutModalDataRef);
    data->output.index = (int) row;
    CFStringRef name = CFArrayGetValueAtIndex(data->names, row);
    setLabelText(data->workoutTextField, name);
}
