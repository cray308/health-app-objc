#include "SetupWorkoutVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "UserData.h"
#include "Views.h"

Class SetupWorkoutVCClass;

#pragma mark - Lifecycle

id setupWorkoutVC_init(id parent, uint8_t type, VCacheRef tbl, CCacheRef clr) {
    id self = msg2(id, VCacheRef, CCacheRef, Sels.alloc(SetupWorkoutVCClass, Sels.alo),
                   sel_getUid("initWithVCache:cCache:"), tbl, clr);
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)self + VCSize + sizeof(InputVC));
    d->parent = parent;
    d->names = createWorkoutNames(type);
    d->type = type;
    if (!objc_getClass("UITabBarAppearance")) {
        id font = clsF1(id, CFStringRef, objc_getClass("UIFont"),
                        sel_getUid("preferredFontForTextStyle:"), UIFontTextStyleTitle3);
        const void *keys[] = {NSFontAttributeName, NSForegroundColorAttributeName};
         d->normalDict = CFDictionaryCreate(NULL, keys, (const void *[]){
             font, clr->getColor(clr->cls, clr->sc, ColorDisabled)
         }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
         d->selectedDict = CFDictionaryCreate(NULL, keys, (const void *[]){
             font, clr->getColor(clr->cls, clr->sc, ColorLabel)
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
    Sels.viewRel(d->workoutTextField, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void setupWorkoutVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    SetupWorkoutVC *d = (SetupWorkoutVC *)((char *)sup + sizeof(InputVC));
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBG));
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    setVCTitle(navItem, localize(CFSTR("setupWorkoutTitle")));

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutPickerTitle"));
    id workoutLabel = createLabel(tbl, sup->clr, CFRetain(pickerTitle),
                                  UIFontTextStyleSubheadline, ColorLabel);
    tbl->label.setLines(workoutLabel, tbl->label.snl, 0);
    tbl->view.setIsAcc(workoutLabel, tbl->view.sace, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, workoutLabel);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 4, workoutLabel);

    d->workoutTextField = createTextfield(tbl, sup->clr, self, sup->toolbar, pickerTitle, -1);
    tbl->field.setText(d->workoutTextField, tbl->label.stxt, CFArrayGetValueAtIndex(d->names, 0));
    msg1(void, long, d->workoutTextField, sel_getUid("setTextAlignment:"), 1);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, d->workoutTextField);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, d->workoutTextField);

    id workoutPicker = Sels.new(objc_getClass("UIPickerView"), Sels.nw);
    msg1(void, id, workoutPicker, tbl->field.sdg, self);
    msg1(void, id, d->workoutTextField, sel_getUid("setInputView:"), workoutPicker);

    SEL tapSel = sel_getUid("buttonTapped:");
    id cancelButton = createButton(tbl, sup->clr, localize(CFSTR("cancel")),
                                   ColorBlue, UIFontTextStyleBody, self, tapSel);
    sup->button = createButton(tbl, sup->clr, localize(CFSTR("go")),
                               ColorBlue, UIFontTextStyleBody, self, tapSel);
    tbl->view.setTag(sup->button, tbl->view.stg, 1);

    setNavButtons(navItem, (id []){cancelButton, sup->button});
    tbl->button.setEnabled(sup->button, tbl->button.en, false);

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
        tbl->button.setEnabled(sup->button, tbl->button.en, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (rows[i])
            inputVC_addChild(self, localize(rows[i]), KeyboardTypeNumberPad, mins[i], maxes[i]);
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
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    sup->tbl->view.setBG(picker, sup->tbl->view.sbg,
                         sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorTertiaryBG));
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
    sup->tbl->field.setText(d->workoutTextField, sup->tbl->label.stxt, name);
}

void setupWorkoutVC_didSelectRowLegacy(id self, SEL _cmd _U_, id picker, long row, long s _U_) {
    setupWorkoutVC_didSelectRow(self, nil, picker, row, 0);
    msg1(void, long, picker, sel_getUid("reloadComponent:"), 0);
}
