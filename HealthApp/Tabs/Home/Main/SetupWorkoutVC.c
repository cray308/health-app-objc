#include "SetupWorkoutVC.h"
#include "InputVC.h"
#include "UserData.h"
#include "Views.h"

Class SetupWorkoutVCClass;

enum {
    NSTextAlignmentCenter = 1
};

enum {
    UITextAutocorrectionTypeNo = 1
};

enum {
    IndexSets,
    IndexReps,
    IndexWeight
};

#pragma mark - Lifecycle

id setupWorkoutVC_init(id parent, unsigned char type) {
    id self = new(SetupWorkoutVCClass);
    SetupWorkoutVC *d = (SetupWorkoutVC *)getIVIVC(self);
    d->parent = parent;
    d->names = createWorkoutNames(type);
    d->type = type;
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutVC *d = (SetupWorkoutVC *)getIVIVC(self);
    CFRelease(d->names);
    releaseV(d->workoutTextField);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *p = (InputVC *)getIVVC(self);
    SetupWorkoutVC *d = (SetupWorkoutVC *)getIVIVCS(p);
    SEL tapSel = sel_getUid("buttonTapped:");
    id cancel = createButton(localize(CFSTR("cancel")), ColorBlue, UIFontTextStyleBody, self, tapSel);
    p->button = createButton(localize(CFSTR("go")), ColorBlue, UIFontTextStyleBody, self, tapSel);
    setTag(p->button, 1);
    setupNavItem(self, CFSTR("setupWorkoutTitle"), (id []){cancel, p->button});
    setEnabled(p->button, false);

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutPickerTitle"));
    id workoutLabel = createLabel(CFRetain(pickerTitle), UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(workoutLabel, false);
    addArrangedSubview(p->vStack, workoutLabel);
    setCustomSpacing(p->vStack, ViewSpacing, workoutLabel);

    d->workoutTextField = createTextfield(self, p->toolbar, pickerTitle, -1);
    msg1(void, long, d->workoutTextField,
         sel_getUid("setAutocorrectionType:"), UITextAutocorrectionTypeNo);
    setText(d->workoutTextField, CFArrayGetValueAtIndex(d->names, 0));
    msg1(void, long, d->workoutTextField, sel_getUid("setTextAlignment:"), NSTextAlignmentCenter);
    addArrangedSubview(p->vStack, d->workoutTextField);
    setCustomSpacing(p->vStack, GroupSpacing, d->workoutTextField);

    id workoutPicker = new(objc_getClass("UIPickerView"));
    setDelegate(workoutPicker, self);
    msg1(void, id, d->workoutTextField, sel_getUid("setInputView:"), workoutPicker);

    short maxes[] = {5, 5, 100}, mins[] = {1, 1, 1};
    CFStringRef rows[] = {CFSTR("setupWorkoutSets"), CFSTR("setupWorkoutReps"), NULL};

    if (d->type == WorkoutStrength) {
        rows[IndexWeight] = CFSTR("setupWorkoutMaxWeight");
    } else if (d->type == WorkoutSE) {
        maxes[IndexSets] = 3;
        maxes[IndexReps] = 50;
    } else if (d->type == WorkoutEndurance) {
        rows[IndexSets] = NULL;
        rows[IndexReps] = CFSTR("setupWorkoutDuration");
        maxes[IndexReps] = 180;
        mins[IndexReps] = MinWorkoutDuration;
    } else {
        rows[0] = rows[1] = NULL;
        setEnabled(p->button, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (rows[i])
            inputVC_addChild(self, localize(rows[i]), UIKeyboardTypeNumberPad, mins[i], maxes[i]);
    }

    releaseV(workoutLabel);
    releaseV(workoutPicker);
}

void setupWorkoutVC_tappedButton(id self, SEL _cmd _U_, id btn) {
    if (!getTag(btn)) {
        dismissPresentedVC(self, NULL);
        return;
    }

    InputVC *p = (InputVC *)getIVVC(self);
    SetupWorkoutVC *d = (SetupWorkoutVC *)getIVIVCS(p);
    short weight = 0, sets = 0, reps = 0;
    switch (d->type) {
        case WorkoutStrength:
            weight = (short)p->children[IndexWeight].data->result;
        case WorkoutSE:
            sets = (short)p->children[IndexSets].data->result;
            reps = (short)p->children[IndexReps].data->result;
            break;

        case WorkoutEndurance:
            reps = (short)p->children[0].data->result;
        default:
            break;
    }
    WorkoutParams params = {d->index, sets, reps, weight, d->type, UCHAR_MAX};
    Workout *w = getWorkoutFromLibrary(&params, getUserInfo()->liftMaxes);
    dismissPresentedVC(self, ^{ homeVC_navigateToAddWorkout(d->parent, w); });
}

#pragma mark - Picker Delegate

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id picker _U_) { return 1; }

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long section _U_) {
    return CFArrayGetCount(((SetupWorkoutVC *)getIVIVC(self))->names);
}

CFStringRef setupWorkoutVC_getTitle(id self, SEL _cmd _U_, id picker _U_, long row, long section _U_) {
    return CFArrayGetValueAtIndex(((SetupWorkoutVC *)getIVIVC(self))->names, row);
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_, id picker _U_, long row, long section _U_) {
    SetupWorkoutVC *d = (SetupWorkoutVC *)getIVIVC(self);
    d->index = (int)row;
    setText(d->workoutTextField, CFArrayGetValueAtIndex(d->names, row));
}
