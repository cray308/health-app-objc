#include "UpdateMaxesVC.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "UserData.h"
#include "Views.h"

#define StepperMin 1
#define StepperMax 10

extern uint64_t UIAccessibilityTraitAdjustable;

Class StepperViewClass;
Class UpdateMaxesVCClass;

#pragma mark - Stepper

static id stepperViewInit(id *ref, CFLocaleRef locale CF_CONSUMED) {
    id self = new(StepperViewClass);
    StepperView *v = (StepperView *)getIVV(self);

    setIsAccessibilityElement(self, true);
    setAccessibilityTraits(self, UIAccessibilityTraitAdjustable);
    CFStringRef stepperDescr = localize(CFSTR("stepperLabelDescr"));
    setAccessibilityLabel(self, stepperDescr);
    CFRelease(stepperDescr);

    CFStringRef repsFmt = localize(CFSTR("stepperLabelInit"));
    CFStringRef repsStr = formatStr(locale, repsFmt, StepperMin);
    CFRelease(repsFmt);
    v->range = findNumber(repsStr, locale, oneStr, NULL);
    CFRelease(locale);
    v->repsStr = CFStringCreateMutableCopy(NULL, 64, repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), repsStr);

    v->label = createLabel(repsStr, UIFontTextStyleBody, ColorLabel);
    v->stepper = new(objc_getClass("UIStepper"));
    *ref = msg0(id, v->stepper, sel_getUid("retain"));
    msg1(void, double, v->stepper, sel_getUid("setValue:"), StepperMin);
    msg1(void, double, v->stepper, sel_getUid("setMinimumValue:"), StepperMin);
    msg1(void, double, v->stepper, sel_getUid("setMaximumValue:"), StepperMax);
    addTarget(v->stepper, self, sel_getUid("valueDidChange"), UIControlEventValueChanged);
    msg2(void, float, long, v->stepper, sel_getUid("setContentHuggingPriority:forAxis:"),
         LayoutPriorityRequired, UILayoutConstraintAxisHorizontal);
    msg2(void, float, long, v->stepper, sel_getUid("setContentCompressionResistancePriority:forAxis:"),
         LayoutPriorityRequired, UILayoutConstraintAxisHorizontal);

    id stack = createHStack((id []){v->label, v->stepper});
    setTrans(stack);
    addSubview(self, stack);
    pin(stack, self);
    setHeight(self, ViewHeightDefault, true, false);
    releaseV(stack);
    return self;
}

void stepperView_deinit(id self, SEL _cmd) {
    StepperView *v = (StepperView *)getIVV(self);
    CFRelease(v->repsStr);
    releaseV(v->stepper);
    releaseV(v->label);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

static void handleNewStepperValue(id self, StepperView *v, int value) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef newVal = formatStr(locale, CFSTR("%d"), value);
    CFRelease(locale);
    updateRange(v->repsStr, &v->range, newVal);
    setText(v->label, v->repsStr);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), v->repsStr);
}

void stepperView_updatedStepper(id self, SEL _cmd _U_) {
    StepperView *v = (StepperView *)getIVV(self);
    handleNewStepperValue(self, v, (int)msg0(double, v->stepper, sel_getUid("value")));
}

static void stepperChangeGeneric(id self, int change) {
    StepperView *v = (StepperView *)getIVV(self);
    int value = (int)msg0(double, v->stepper, sel_getUid("value")) + change;
    if (value >= StepperMin && value <= StepperMax) {
        msg1(void, double, v->stepper, sel_getUid("setValue:"), value);
        handleNewStepperValue(self, v, value);
    }
}

void stepperView_increment(id self, SEL _cmd _U_) { stepperChangeGeneric(self, 1); }

void stepperView_decrement(id self, SEL _cmd _U_) { stepperChangeGeneric(self, -1); }

#pragma mark - VC

id updateMaxesVC_init(id parent, int index) {
    id self = new(UpdateMaxesVCClass);
    UpdateMaxesVC *d = (UpdateMaxesVC *)getIVIVC(self);
    d->parent = parent;
    d->index = index;
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    releaseV(((UpdateMaxesVC *)getIVIVC(self))->stepper);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *p = (InputVC *)getIVVC(self);
    UpdateMaxesVC *d = (UpdateMaxesVC *)getIVIVCS(p);
    p->button = createButton(localize(CFSTR("finish")),
                             ColorBlue, UIFontTextStyleBody, self, sel_getUid("buttonTapped:"));
    setupNavItem(self, CFSTR("updateMaxesTitle"), (id []){nil, p->button});
    setEnabled(p->button, false);

    if (objc_getClass("UINavigationBarAppearance"))
        msg1(void, bool, self, sel_getUid("setModalInPresentation:"), true);

    setSpacing(p->vStack, GroupSpacing);
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef liftKey = formatStr(NULL, CFSTR("exNames%02d"), d->index);
    CFStringRef liftVal = localize(liftKey);
    CFRelease(liftKey);
    CFMutableStringRef adjLift = CFStringCreateMutableCopy(NULL, 128, liftVal);
    CFRelease(liftVal);
    CFStringLowercase(adjLift, locale);
    CFStringRef fieldKey = localize(CFSTR("maxWeight"));
    int kb = getKBForLocale(locale);
    inputVC_addChild(self, formatStr(NULL, fieldKey, adjLift), kb, 1, FieldMaxDefault);
    CFRelease(adjLift);
    CFRelease(fieldKey);

    id stepperView = stepperViewInit(&d->stepper, locale);
    addArrangedSubview(p->vStack, stepperView);
    releaseV(stepperView);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_, id btn _U_) {
    InputVC *p = (InputVC *)getIVVC(self);
    UpdateMaxesVC *d = (UpdateMaxesVC *)getIVIVCS(p);
    short extra = d->index == LiftPullup ? bodyweight : 0;
    float initWeight = ((p->children[0].data->result * toSavedMass) + extra) * 36;
    float reps = 37.f - (float)msg0(double, d->stepper, sel_getUid("value"));
    short weight = (short)lrintf(initWeight / reps) - extra;
    dismissPresentedVC(self, ^{ workoutVC_finishedBottomSheet(d->parent, d->index, weight); });
}
