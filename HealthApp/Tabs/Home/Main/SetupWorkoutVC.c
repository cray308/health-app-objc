#include "SetupWorkoutVC.h"
#include "ExerciseManager.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SetupWorkoutVCClass;

id setupWorkoutVC_init(id parent, unsigned char type) {
    id self = createNew(SetupWorkoutVCClass);
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    data->parent = parent;
    data->names = exerciseManager_createWorkoutNames(type);
    data->type = type;
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    struct objc_super super = {self, InputVCClass};
    CFRelease(data->names);
    releaseObj(data->workoutTextField);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutTitle"));
    id workoutLabel = createLabel(pickerTitle, TextFootnote, false);
    CFStringRef defaultTitle = CFArrayGetValueAtIndex(data->names, 0);
    data->workoutTextField = createTextfield(nil, defaultTitle, pickerTitle, 1, 0, 0);
    id workoutContainer = createStackView((id []){workoutLabel, data->workoutTextField},
                                          2, 1, 2, (Padding){30, 8, 20, 8});

    id workoutPicker = createNew(objc_getClass("UIPickerView"));
    setDelegate(workoutPicker, self);
    setObject(data->workoutTextField, sel_getUid("setInputView:"), workoutPicker);
    setInputAccessory(data->workoutTextField, parent->toolbar);

    addArrangedSubview(parent->vStack, workoutContainer);

    SEL tapSel = sel_getUid("buttonTapped:");
    id cancelButton = createButton(localize(CFSTR("cancel")), ColorBlue, 0, 0, self, tapSel);
    parent->button = createButton(localize(CFSTR("go")), ColorBlue, 0, 1, self, tapSel);

    CGRect frame;
    getRect(view, &frame, 0);
    setNavButton(self, true, cancelButton, (int) frame.size.width);
    setNavButton(self, false, parent->button, (int) frame.size.width);
    enableButton(parent->button, false);

    short maxes[] = {5, 5, 100}, mins[] = {1, 1, 1};
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
            mins[1] = 15;
            break;
        default:
            rows[0] = rows[1] = NULL;
            enableButton(parent->button, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (rows[i])
            inputVC_addChild(self, localize(rows[i]), mins[i], maxes[i]);
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

    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    id *fields = ((InputVC *) ((char *)self + VCSize))->children;
    short weight = 0, sets = 0, reps = 0;
    switch (data->type) {
        case WorkoutStrength:
            weight = ((InputView *) ((char *)fields[2] + ViewSize))->result;
        case WorkoutSE:
            sets = ((InputView *) ((char *)fields[0] + ViewSize))->result;
            reps = ((InputView *) ((char *)fields[1] + ViewSize))->result;
            break;

        case WorkoutEndurance:
            reps = ((InputView *) ((char *)fields[0] + ViewSize))->result;
        default:
            break;
    }
    Workout *w = exerciseManager_getWorkoutFromLibrary(&(WorkoutParams){
        data->index, sets, reps, weight, data->type, 0xff
    });
    id parent = data->parent;
    dismissPresentedVC(parent, ^{
        homeVC_navigateToAddWorkout(parent, w);
    });
}

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id picker _U_) { return 1; }

long setupWorkoutVC_numberOfComponentsLegacy(id self _U_, SEL _cmd _U_, id picker) {
    setBackground(picker, createColor(ColorTertiaryBG));
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long section _U_) {
    return CFArrayGetCount(((SetupWorkoutVC *) ((char *)self + InputVCSize))->names);
}

id setupWorkoutVC_attrTitleForRow(id self, SEL _cmd _U_,
                                  id picker _U_, long row, long section _U_) {
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    id color = createColor(row == data->index ? ColorLabel : ColorSecondaryLabel);
    CFDictionaryRef dict = createTitleTextDict(color, createFont(TextTitle3));
    id attrString = createAttribString(CFArrayGetValueAtIndex(data->names, row), dict);
    voidFunc(attrString, sel_getUid("autorelease"));
    CFRelease(dict);
    return attrString;
}

CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd _U_,
                                       id picker _U_, long row, long section _U_) {
    CFArrayRef names = ((SetupWorkoutVC *) ((char *)self + InputVCSize))->names;
    return CFArrayGetValueAtIndex(names, row);
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_, id picker _U_, long row, long section _U_) {
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    data->index = (int) row;
    CFStringRef name = CFArrayGetValueAtIndex(data->names, row);
    setLabelText(data->workoutTextField, name);
}

void setupWorkoutVC_didSelectRowLegacy(id self, SEL _cmd _U_,
                                       id picker, long row, long section _U_) {
    setupWorkoutVC_didSelectRow(self, nil, picker, row, 0);
    setInt(picker, sel_getUid("reloadComponent:"), 0);
}
