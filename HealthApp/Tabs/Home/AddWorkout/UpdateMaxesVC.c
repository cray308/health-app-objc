#include "UpdateMaxesVC.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

#define StepperMin 1
#define StepperMax 10

#define setStepperValue(s, v) msgV(objSig(void, double), (s), sel_getUid("setValue:"), (v))

extern uint64_t UIAccessibilityTraitAdjustable;

Class StepperViewClass;
Class UpdateMaxesVCClass;

static Class Stepper;
static SEL sgv;
static double (*getValue)(id, SEL);

void initUpdateMaxesData(void) {
    Stepper = objc_getClass("UIStepper");
    sgv = sel_getUid("value");
    getValue = (double(*)(id, SEL))class_getMethodImplementation(Stepper, sgv);
}

#pragma mark - Stepper

static id stepperView_init(id *stepperRef, CFLocaleRef locale CF_CONSUMED) {
    id self = new(StepperViewClass);
    StepperView *v = getIVV(StepperView, self);

    CFStringRef stepperDescription = localize(CFSTR("stepperDescription")), one = getOneStr(locale);
    CFStringRef repsFormat = localize(CFSTR("stepperLabel"));
    CFStringRef reps = formatStr(locale, repsFormat, StepperMin);
    v->range = findNumber(reps, locale, one, NULL);
    v->reps = CFStringCreateMutableCopy(NULL, 64, reps);
    makeCustomViewAccessible(self)
    setAccessibilityTraits(self, UIAccessibilityTraitAdjustable);
    setAccessibilityLabel(self, stepperDescription);
    setAccessibilityValue(self, reps);
    setHeight(self, ViewHeightDefault, true, false);
    CFRelease(stepperDescription);
    CFRelease(one);
    CFRelease(repsFormat);
    CFRelease(locale);

    v->label = createLabel(reps, UIFontTextStyleBody, ColorLabel);
    v->stepper = new(Stepper);
    *stepperRef = retainView(v->stepper);
    setStepperValue(v->stepper, StepperMin);
    msgV(objSig(void, double), v->stepper, sel_getUid("setMinimumValue:"), StepperMin);
    msgV(objSig(void, double), v->stepper, sel_getUid("setMaximumValue:"), StepperMax);
    addTarget(v->stepper, self, getValueChangedSel(), ControlEventValueChanged);
    msgV(objSig(void, float, long), v->stepper, sel_getUid("setContentHuggingPriority:forAxis:"),
         LayoutPriorityRequired, ConstraintAxisHorizontal);
    msgV(objSig(void, float, long), v->stepper,
         sel_getUid("setContentCompressionResistancePriority:forAxis:"),
         LayoutPriorityRequired, ConstraintAxisHorizontal);

    id stack = createHStack((id []){v->label, v->stepper});
    useStackConstraints(stack);
    addSubview(self, stack);
    pin(stack, self);
    releaseView(stack);
    return self;
}

void stepperView_deinit(id self, SEL _cmd) {
    StepperView *v = getIVV(StepperView, self);
    CFRelease(v->reps);
    releaseView(v->stepper);
    releaseView(v->label);
    msgSupV(supSig(), self, View, _cmd);
}

static void updateStepperValue(id self, StepperView *v, int value) {
    CFLocaleRef locale = copyLocale();
    updateRange(v->reps, &v->range, formatStr(locale, CFSTR("%d"), value), locale);
    setText(v->label, v->reps);
    setAccessibilityValue(self, v->reps);
}

void stepperView_changedValue(id self, SEL _cmd _U_) {
    StepperView *v = getIVV(StepperView, self);
    updateStepperValue(self, v, (int)getValue(v->stepper, sgv));
}

static void accessibilityChanged(id self, int offset) {
    StepperView *v = getIVV(StepperView, self);
    int value = (int)getValue(v->stepper, sgv) + offset;
    if (value >= StepperMin && value <= StepperMax) {
        setStepperValue(v->stepper, value);
        updateStepperValue(self, v, value);
    }
}

void stepperView_accessibilityIncrement(id self, SEL _cmd _U_) { accessibilityChanged(self, 1); }

void stepperView_accessibilityDecrement(id self, SEL _cmd _U_) { accessibilityChanged(self, -1); }

#pragma mark - VC

id updateMaxesVC_init(id delegate, int index) {
    id self = new(UpdateMaxesVCClass);
    UpdateMaxesVC *d = getIVVCC(UpdateMaxesVC, InputVC, self);
    d->delegate = delegate;
    d->index = index;
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    releaseView(getIVVCC(UpdateMaxesVC, InputVC, self)->repsStepper);
    inputVC_deinit(self, _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    inputVC_viewDidLoad(self, _cmd);

    InputVC *p = getIVVC(InputVC, self);
    UpdateMaxesVC *d = getIVVCS(UpdateMaxesVC, p);
    p->button = createButton(localize(CFSTR("finish")), ColorBlue, self, getTapSel());
    setupNavItem(self, CFSTR("updateMaxesTitle"), (id []){nil, p->button});
    setEnabled(p->button, false);

    if (getTabBarAppearanceClass())
        msgV(objSig(void, bool), self, sel_getUid("setModalInPresentation:"), true);

    setSpacing(p->vStack, GroupSpacing);
    CFLocaleRef locale = copyLocale();
    CFStringRef liftKey = formatStr(NULL, CFSTR("exNames%02d"), d->index);
    CFStringRef liftName = localize(liftKey), fieldKey = localize(CFSTR("maxWeight"));
    CFMutableStringRef adjustedName = CFStringCreateMutableCopy(NULL, 128, liftName);
    CFStringLowercase(adjustedName, locale);
    int kbType = getKeyboardForLocale(locale);
    inputVC_addField(self, formatStr(NULL, fieldKey, adjustedName), kbType, 1, FieldMaxDefault);
    CFRelease(liftKey);
    CFRelease(liftName);
    CFRelease(fieldKey);
    CFRelease(adjustedName);

    id stepperView = stepperView_init(&d->repsStepper, locale);
    addArrangedSubview(p->vStack, stepperView);
    releaseView(stepperView);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_, id button _U_) {
    InputVC *p = getIVVC(InputVC, self);
    UpdateMaxesVC *d = getIVVCS(UpdateMaxesVC, p);
    CFLocaleRef locale = copyLocale();
    int extra = d->index == LiftPullup ? Bodyweight : 0;
    float initWeight = ((p->children[0].data->result * getSavedMassFactor(locale)) + extra) * 36;
    float reps = 37.f - (float)getValue(d->repsStepper, sgv);
    int weight = (int)lrintf(initWeight / reps) - extra;
    CFRelease(locale);
    dismissPresentedVC(self, ^{ workoutVC_finishedBottomSheet(d->delegate, d->index, weight); });
}
