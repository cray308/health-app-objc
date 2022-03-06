#include "SetupWorkoutVC.h"
#include <CoreFoundation/CFAttributedString.h>
#include "AppDelegate.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

Class SetupWorkoutVCClass;

CFArrayRef createWorkoutNames(unsigned char type);
void homeVC_navigateToAddWorkout(id self, void *workout);

id setupWorkoutVC_init(id parent, unsigned char type) {
    id self = createNew(SetupWorkoutVCClass);
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    data->parent = parent;
    data->names = createWorkoutNames(type);
    id font = getPreferredFont(UIFontTextStyleTitle3);
    const void *keys[] = {
        (CFStringRef) NSForegroundColorAttributeName, (CFStringRef) NSFontAttributeName
    };
    const void *normalVals[] = {createColor(ColorSecondaryLabel), font};
    const void *selectedVals[] = {createColor(ColorLabel), font};
    data->normalDict = CFDictionaryCreate(NULL, keys, normalVals, 2,
                                          &kCFCopyStringDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks);
    data->selectedDict = CFDictionaryCreate(NULL, keys, selectedVals, 2,
                                            &kCFCopyStringDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks);
    data->type = type;
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    struct objc_super super = {self, InputVCClass};
    CFRelease(data->names);
    CFRelease(data->normalDict);
    CFRelease(data->selectedDict);
    releaseObj(data->workoutTextField);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorSecondaryBG));

    CFStringRef pickerTitle = CFBundleCopyLocalizedString(bundle, CFSTR("setupWorkoutTitle"),
                                                          NULL, NULL);
    CFRetain(pickerTitle);
    id workoutLabel = createLabel(pickerTitle, UIFontTextStyleFootnote, false);
    CFStringRef defaultTitle = CFArrayGetValueAtIndex(data->names, 0);
    data->workoutTextField = createTextfield(nil, defaultTitle, pickerTitle, 1, 0, 0);
    id workoutContainer = createStackView((id []){workoutLabel, data->workoutTextField},
                                          2, 1, 2, (Padding){30, 8, 20, 8});

    id workoutPicker = createNew(objc_getClass("UIPickerView"));
    setDelegate(workoutPicker, self);
    msg1(void, id, data->workoutTextField, sel_getUid("setInputView:"), workoutPicker);
    setInputAccessory(data->workoutTextField, parent->toolbar);

    addArrangedSubview(parent->vStack, workoutContainer);

    SEL tapSel = sel_getUid("buttonTapped:");
    id cancelButton = createButton(CFBundleCopyLocalizedString(bundle, CFSTR("cancel"), NULL, NULL),
                                   ColorBlue, 0, self, tapSel);
    parent->button = createButton(CFBundleCopyLocalizedString(bundle, CFSTR("go"), NULL, NULL),
                                  ColorBlue, 1, self, tapSel);

    setNavButtons(self, (id []){cancelButton, parent->button});
    enableButton(parent->button, false);

    short maxes[] = {5, 5, 100}, mins[] = {1, 1, 1};
    CFStringRef rows[] = {CFSTR("setupWorkoutSets"), CFSTR("setupWorkoutReps"), NULL};

    if (data->type == WorkoutStrength) {
        rows[2] = CFSTR("setupWorkoutMaxWeight");
    } else if (data->type == WorkoutSE) {
        maxes[0] = 3;
        maxes[1] = 50;
    } else if (data->type == WorkoutEndurance) {
        rows[0] = NULL;
        rows[1] = CFSTR("setupWorkoutDuration");
        maxes[1] = 180;
        mins[1] = 15;
    } else {
        rows[0] = rows[1] = NULL;
        enableButton(parent->button, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (rows[i]) {
            inputVC_addChild(self, CFBundleCopyLocalizedString(bundle, rows[i], NULL, NULL),
                             mins[i], maxes[i]);
        }
    }

    releaseObj(workoutLabel);
    releaseObj(workoutContainer);
    releaseObj(workoutPicker);
}

void setupWorkoutVC_tappedButton(id self, SEL _cmd _U_, id btn) {
    if (!getTag(btn)) {
        dismissPresentedVC(nil);
        return;
    }

    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    InputView **fields = ((InputVC *) ((char *)self + VCSize))->children;
    short weight = 0, sets = 0, reps = 0;
    switch (data->type) {
        case WorkoutStrength:
            weight = fields[2]->result;
        case WorkoutSE:
            sets = fields[0]->result;
            reps = fields[1]->result;
            break;

        case WorkoutEndurance:
            reps = fields[0]->result;
        default:
            break;
    }
    Workout *w = getWorkoutFromLibrary(&(WorkoutParams){
        data->index, sets, reps, weight, data->type, 0xff
    });
    id parent = data->parent;
    dismissPresentedVC(^{ homeVC_navigateToAddWorkout(parent, w); });
}

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id picker _U_) { return 1; }

long setupWorkoutVC_numberOfComponentsLegacy(id self _U_, SEL _cmd _U_, id picker) {
    setBackground(picker, createColor(ColorTertiaryBG));
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long section _U_) {
    return CFArrayGetCount(((SetupWorkoutVC *) ((char *)self + InputVCSize))->names);
}

CFAttributedStringRef setupWorkoutVC_attrTitleForRow(id self, SEL _cmd _U_,
                                                     id picker _U_, long row, long section _U_) {
    SetupWorkoutVC *data = (SetupWorkoutVC *) ((char *)self + InputVCSize);
    CFDictionaryRef dict = row == data->index ? data->selectedDict : data->normalDict;
    CFStringRef str = CFArrayGetValueAtIndex(data->names, row);
    CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, str, dict);
    CFAutorelease(attrString);
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
    msg1(void, long, picker, sel_getUid("reloadComponent:"), 0);
}
