#include "SetupWorkoutVC.h"
#include "InputVC.h"
#include "UserData.h"
#include "Views.h"

Class SetupWorkoutVCClass;

static SEL prc;
static void (*reloadComponent)(id, SEL, long);

void initSetupWorkoutData(void) {
    Class Picker = objc_getClass("UIPickerView");
    prc = sel_getUid("reloadComponent:");
    reloadComponent = (void(*)(id, SEL, long))class_getMethodImplementation(Picker, prc);
}

#pragma mark - Lifecycle

id setupWorkoutVC_init(id delegate, uint8_t type) {
    id self = new(SetupWorkoutVCClass);
    SetupWorkoutVC *d = getIVVCC(SetupWorkoutVC, InputVC, self);
    d->delegate = delegate;
    d->names = createWorkoutNames(type);
    d->type = type;
    if (!getTabBarAppearanceClass()) {
        id font = getPreferredFont(UIFontTextStyleTitle3);
        const void *keys[] = {NSFontAttributeName, NSForegroundColorAttributeName};
        d->normalDict = CFDictionaryCreate(NULL, keys, (const void *[]){
            font, getColor(ColorDisabled)
        }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        d->selectedDict = CFDictionaryCreate(NULL, keys, (const void *[]){
            font, getColor(ColorLabel)
        }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutVC *d = getIVVCC(SetupWorkoutVC, InputVC, self);
    CFRelease(d->names);
    if (d->normalDict) {
        CFRelease(d->normalDict);
        CFRelease(d->selectedDict);
    }
    releaseView(d->workoutField);
    inputVC_deinit(self, _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    inputVC_viewDidLoad(self, _cmd);

    InputVC *p = getIVVC(InputVC, self);
    SetupWorkoutVC *d = getIVVCS(SetupWorkoutVC, p);
    SEL tapSel = getTapSel();
    id cancelButton = createButton(localize(CFSTR("cancel")), ColorBlue, self, tapSel);
    p->button = createButton(localize(CFSTR("go")), ColorBlue, self, tapSel);
    setTag(p->button, 1);
    setupNavItem(self, CFSTR("setupWorkoutTitle"), (id []){cancelButton, p->button});
    setEnabled(p->button, false);

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutPickerTitle"));
    id workoutLabel = createLabel(CFRetain(pickerTitle), UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(workoutLabel, false);
    addArrangedSubview(p->vStack, workoutLabel);
    setCustomSpacing(p->vStack, ViewSpacing, workoutLabel);
    releaseView(workoutLabel);

    d->workoutField = createTextField(self, p->toolbar, pickerTitle, -1);
    setFieldText(d->workoutField, CFArrayGetValueAtIndex(d->names, 0));
    msgV(objSig(void, long), d->workoutField, sel_getUid("setTextAlignment:"), 1);
    msgV(objSig(void, long), d->workoutField,
         sel_getUid("setAutocorrectionType:"), 1);
    addArrangedSubview(p->vStack, d->workoutField);
    setCustomSpacing(p->vStack, GroupSpacing, d->workoutField);

    id workoutPicker = new(objc_getClass("UIPickerView"));
    setDelegate(workoutPicker, self);
    msgV(objSig(void, id), workoutPicker, sel_getUid("setDataSource:"), self);
    msgV(objSig(void, id), d->workoutField, sel_getUid("setInputView:"), workoutPicker);
    releaseView(workoutPicker);

    int maxes[] = {5, 5, 100}, mins[] = {1, 1, 1};
    CFStringRef rows[] = {CFSTR("setupWorkoutSets"), CFSTR("setupWorkoutReps"), NULL};

    if (d->type == WorkoutStrength) {
        rows[2] = CFSTR("setupWorkoutMaxWeight");
    } else if (d->type == WorkoutSE) {
        maxes[0] = 3;
        maxes[1] = 50;
    } else if (d->type == WorkoutEndurance) {
        rows[0] = NULL;
        rows[1] = CFSTR("setupWorkoutDuration");
        maxes[1] = 180;
        mins[1] = 15;
    } else {
        rows[0] = rows[1] = NULL;
        setEnabled(p->button, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (rows[i])
            inputVC_addField(self, localize(rows[i]), KeyboardTypeNumberPad, mins[i], maxes[i]);
    }
}

void setupWorkoutVC_tappedButton(id self, SEL _cmd _U_, id button) {
    Callback callback = NULL;
    if (!getTag(button)) goto end;

    InputVC *p = getIVVC(InputVC, self);
    SetupWorkoutVC *d = getIVVCS(SetupWorkoutVC, p);
    WorkoutParams params = {.index = d->index, .type = d->type, .day = UCHAR_MAX};
    switch (d->type) {
        case WorkoutStrength:
            params.weight = (short)p->children[2].data->result;
        case WorkoutSE:
            params.sets = (short)p->children[0].data->result;
            params.reps = (short)p->children[1].data->result;
            break;

        case WorkoutEndurance:
            params.reps = (short)p->children[0].data->result;
        default:
            break;
    }
    Workout *workout = getWorkoutFromLibrary(&params, getUserData()->lifts);
    callback = ^{ homeVC_navigateToWorkout(d->delegate, workout); };
end:
    dismissPresentedVC(self, callback);
}

#pragma mark - Picker Delegate

long setupWorkoutVC_numberOfComponentsInPickerView(id self _U_, SEL _cmd _U_, id picker _U_) {
    if (getIVVCC(SetupWorkoutVC, InputVC, self)->normalDict)
        setBackgroundColor(picker, getColor(ColorTertiaryBG));
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long component _U_) {
    return CFArrayGetCount(getIVVCC(SetupWorkoutVC, InputVC, self)->names);
}

CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd _U_,
                                       id picker _U_, long row, long component _U_) {
    return CFArrayGetValueAtIndex(getIVVCC(SetupWorkoutVC, InputVC, self)->names, row);
}

CFAttributedStringRef setupWorkoutVC_attributedTitle(id self, SEL _cmd _U_,
                                                     id picker _U_, long row, long component _U_) {
    SetupWorkoutVC *d = getIVVCC(SetupWorkoutVC, InputVC, self);
    if (!d->normalDict) return NULL;

    CFAttributedStringRef attrString = CFAttributedStringCreate(
      NULL, CFArrayGetValueAtIndex(d->names, row), row == d->index ? d->selectedDict : d->normalDict);
    CFAutorelease(attrString);
    return attrString;
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_,
                                 id picker _U_, long row, long component _U_) {
    SetupWorkoutVC *d = getIVVCC(SetupWorkoutVC, InputVC, self);
    d->index = (int)row;
    setFieldText(d->workoutField, CFArrayGetValueAtIndex(d->names, row));
    if (d->normalDict) reloadComponent(picker, prc, 0);
}
