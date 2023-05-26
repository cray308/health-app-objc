#include "SetupWorkoutVC.h"
#include "AppDelegate.h"
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

id setupWorkoutVC_init(id parent, uint8_t type) {
    id self = new(SetupWorkoutVCClass);
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    d->parent = parent;
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
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    CFRelease(d->names);
    if (d->normalDict) {
        CFRelease(d->normalDict);
        CFRelease(d->selectedDict);
    }
    releaseView(d->workoutField);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    inputVC_viewDidLoad(self, _cmd);

    InputVC *p = (InputVC *)((char *)self + VCSize);
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)p + sizeof(InputVC));
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

    d->workoutField = createTextfield(self, p->toolbar, pickerTitle, -1);
    setFieldText(d->workoutField, CFArrayGetValueAtIndex(d->names, 0));
    msg1(void, long, d->workoutField, sel_getUid("setTextAlignment:"), 1);
    addArrangedSubview(p->vStack, d->workoutField);
    setCustomSpacing(p->vStack, GroupSpacing, d->workoutField);

    id workoutPicker = new(objc_getClass("UIPickerView"));
    setDelegate(workoutPicker, self);
    msg1(void, id, d->workoutField, sel_getUid("setInputView:"), workoutPicker);
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
            inputVC_addChild(self, localize(rows[i]), KeyboardTypeNumberPad, mins[i], maxes[i]);
    }
}

void setupWorkoutVC_tappedButton(id self, SEL _cmd _U_, id button) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    if (!getTag(button)) {
        dismissPresentedVC(nil);
        return;
    }

    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)sup + sizeof(InputVC));
    short weight = 0, sets = 0, reps = 0;
    switch (d->type) {
        case WorkoutStrength:
            weight = (short)sup->children[2].data->result;
        case WorkoutSE:
            sets = (short)sup->children[0].data->result;
            reps = (short)sup->children[1].data->result;
            break;

        case WorkoutEndurance:
            reps = (short)sup->children[0].data->result;
        default:
            break;
    }
    WorkoutParams params = {d->index, sets, reps, weight, d->type, UCHAR_MAX};
    Workout *workout = getWorkoutFromLibrary(&params, getUserData()->lifts);
    id parent = d->parent;
    dismissPresentedVC(^{ homeVC_navigateToWorkout(parent, workout); });
}

#pragma mark - Picker Delegate

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id p _U_) { return 1; }

long setupWorkoutVC_numberOfComponentsLegacy(id self _U_, SEL _cmd _U_, id picker) {
    setBackgroundColor(picker, getColor(ColorTertiaryBG));
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id p _U_, long s _U_) {
    return CFArrayGetCount(((SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC)))->names);
}

CFStringRef setupWorkoutVC_getTitle(id self, SEL _cmd _U_, id p _U_, long row, long s _U_) {
    CFArrayRef names = ((SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC)))->names;
    return CFArrayGetValueAtIndex(names, row);
}

CFAttributedStringRef setupWorkoutVC_getAttrTitle(id self, SEL _cmd _U_, id p _U_, long row, long s _U_) {
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    CFAttributedStringRef attrString = CFAttributedStringCreate(
      NULL, CFArrayGetValueAtIndex(d->names, row), row == d->index ? d->selectedDict : d->normalDict);
    CFAutorelease(attrString);
    return attrString;
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_, id p _U_, long row, long s _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)sup + sizeof(InputVC));
    d->index = (int)row;
    CFStringRef name = CFArrayGetValueAtIndex(d->names, row);
    setFieldText(d->workoutField, name);
}

void setupWorkoutVC_didSelectRowLegacy(id self, SEL _cmd _U_, id picker, long row, long s _U_) {
    setupWorkoutVC_didSelectRow(self, nil, picker, row, 0);
    reloadComponent(picker, prc, 0);
}
