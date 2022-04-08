#include "SetupWorkoutVC.h"
#include <CoreFoundation/CFAttributedString.h>
#include "AppDelegate.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

Class SetupWorkoutVCClass;

CFArrayRef createWorkoutNames(unsigned char type);
void homeVC_navigateToAddWorkout(id self, void *workout);

id setupWorkoutVC_init(id parent, unsigned char type, VCacheRef tbl, CCacheRef clr) {
    id self = msg2(id, VCacheRef, CCacheRef, Sels.alloc(SetupWorkoutVCClass, Sels.alo),
                   sel_getUid("initWithVCache:cCache:"), tbl, clr);
    SetupWorkoutVC *data = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    data->parent = parent;
    data->names = createWorkoutNames(type);
    data->type = type;
    if (!objc_getClass("UITabBarAppearance")) {
        id font = clsF1(id, CFStringRef, objc_getClass("UIFont"),
                        sel_getUid("preferredFontForTextStyle:"), UIFontTextStyleTitle3);
        const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
        const void *normalVals[] = {clr->getColor(clr->cls, clr->sc, ColorSecondaryLabel), font};
        const void *selectedVals[] = {clr->getColor(clr->cls, clr->sc, ColorLabel), font};
        data->normalDict = createDict(keys, normalVals, 2, &kCFTypeDictionaryValueCallBacks);
        data->selectedDict = createDict(keys, selectedVals, 2, &kCFTypeDictionaryValueCallBacks);
    }
    return self;
}

void setupWorkoutVC_deinit(id self, SEL _cmd) {
    SetupWorkoutVC *data = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    CFRelease(data->names);
    if (data->normalDict) {
        CFRelease(data->normalDict);
        CFRelease(data->selectedDict);
    }
    Sels.viewRel(data->workoutTextField, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    SetupWorkoutVC *data = (SetupWorkoutVC *)((char *)sup + sizeof(InputVC));
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBG));
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    setVCTitle(navItem, localize(bundle, CFSTR("setupWorkoutTitle")));

    CFStringRef pickerTitle = localize(bundle, CFSTR("setupWorkoutPickerTitle"));
    id workoutLabel = createLabel(tbl, sup->clr, CFRetain(pickerTitle),
                                  UIFontTextStyleSubheadline, ColorLabel);
    tbl->label.setLines(workoutLabel, tbl->label.snl, 0);
    tbl->view.setIsAcc(workoutLabel, tbl->view.sace, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, workoutLabel);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 4, workoutLabel);

    data->workoutTextField = createTextfield(
      tbl, sup->clr, self, sup->toolbar, CFArrayGetValueAtIndex(data->names, 0), pickerTitle, -1);
    msg1(void, long, data->workoutTextField, sel_getUid("setTextAlignment:"), 1);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, data->workoutTextField);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, data->workoutTextField);

    id workoutPicker = Sels.new(objc_getClass("UIPickerView"), Sels.nw);
    msg1(void, id, workoutPicker, tbl->field.sdg, self);
    msg1(void, id, data->workoutTextField, sel_getUid("setInputView:"), workoutPicker);

    SEL tapSel = sel_getUid("buttonTapped:");
    id cancelButton = createButton(tbl, sup->clr, localize(bundle, CFSTR("cancel")),
                                   ColorBlue, UIFontTextStyleBody, self, tapSel);
    sup->button = createButton(tbl, sup->clr, localize(bundle, CFSTR("go")),
                               ColorBlue, UIFontTextStyleBody, self, tapSel);
    tbl->view.setTag(sup->button, tbl->view.stg, 1);

    setNavButtons(navItem, (id []){cancelButton, sup->button});
    tbl->button.setEnabled(sup->button, tbl->button.en, false);

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
        tbl->button.setEnabled(sup->button, tbl->button.en, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (rows[i]) {
            inputVC_addChild(self, localize(bundle, rows[i]), mins[i], maxes[i]);
        }
    }

    Sels.viewRel(workoutLabel, Sels.rel);
    Sels.viewRel(workoutPicker, Sels.rel);
}

void setupWorkoutVC_tappedButton(id self, SEL _cmd _U_, id btn) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    if (!sup->tbl->view.getTag(btn, sup->tbl->view.gtg)) {
        dismissPresentedVC(nil);
        return;
    }

    SetupWorkoutVC *data = (SetupWorkoutVC *)((char *)sup + sizeof(InputVC));
    short weight = 0, sets = 0, reps = 0;
    switch (data->type) {
        case WorkoutStrength:
            weight = sup->children[2].data->result;
        case WorkoutSE:
            sets = sup->children[0].data->result;
            reps = sup->children[1].data->result;
            break;

        case WorkoutEndurance:
            reps = sup->children[0].data->result;
        default:
            break;
    }
    Workout *w = getWorkoutFromLibrary(&(WorkoutParams){data->index, sets, reps, weight, data->type, 0xff});
    id parent = data->parent;
    dismissPresentedVC(^{ homeVC_navigateToAddWorkout(parent, w); });
}

long setupWorkoutVC_numberOfComponents(id self _U_, SEL _cmd _U_, id picker _U_) { return 1; }

long setupWorkoutVC_numberOfComponentsLegacy(id self _U_, SEL _cmd _U_, id picker) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    sup->tbl->view.setBG(picker, sup->tbl->view.sbg,
                         sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorTertiaryBG));
    return 1;
}

long setupWorkoutVC_numberOfRows(id self, SEL _cmd _U_, id picker _U_, long section _U_) {
    return CFArrayGetCount(((SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC)))->names);
}

CFAttributedStringRef setupWorkoutVC_attrTitleForRow(id self, SEL _cmd _U_,
                                                     id picker _U_, long row, long section _U_) {
    SetupWorkoutVC *data = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    CFDictionaryRef dict = row == data->index ? data->selectedDict : data->normalDict;
    CFStringRef str = CFArrayGetValueAtIndex(data->names, row);
    CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, str, dict);
    CFAutorelease(attrString);
    return attrString;
}

CFStringRef setupWorkoutVC_titleForRow(id self, SEL _cmd _U_, id picker _U_, long row, long section _U_) {
    CFArrayRef names = ((SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC)))->names;
    return CFArrayGetValueAtIndex(names, row);
}

void setupWorkoutVC_didSelectRow(id self, SEL _cmd _U_, id picker _U_, long row, long section _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    SetupWorkoutVC *data = (SetupWorkoutVC *)((char *)sup + sizeof(InputVC));
    data->index = (int)row;
    CFStringRef name = CFArrayGetValueAtIndex(data->names, row);
    sup->tbl->field.setText(data->workoutTextField, sup->tbl->label.stxt, name);
}

void setupWorkoutVC_didSelectRowLegacy(id self, SEL _cmd _U_, id picker, long row, long section _U_) {
    setupWorkoutVC_didSelectRow(self, nil, picker, row, 0);
    msg1(void, long, picker, sel_getUid("reloadComponent:"), 0);
}
