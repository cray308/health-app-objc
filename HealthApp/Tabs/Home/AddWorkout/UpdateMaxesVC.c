#include "UpdateMaxesVC.h"
#include "ExerciseManager.h"
#include "InputVC.h"
#include "Views.h"

#define StepperMin 1
#define StepperMax 10

#define setStepperValue(s, v) msg1(void, double, (s), sel_getUid("setValue:"), (v))

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
    StepperView *v = (StepperView *)((char *)self + ViewSize);

    CFStringRef stepperDescription = localize(CFSTR("stepperDescription"));
    CFStringRef one = formatStr(locale, CFSTR("%d"), 1);
    CFStringRef repsFormat = localize(CFSTR("stepperLabel"));
    CFStringRef reps = formatStr(locale, repsFormat, StepperMin);
    CFStringFindWithOptionsAndLocale(
      reps, one, (CFRange){0, CFStringGetLength(reps)}, 0, locale, &v->range);
    v->reps = CFStringCreateMutableCopy(NULL, 64, reps);
    setIsAccessibilityElement(self, true);
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
    msg1(void, double, v->stepper, sel_getUid("setMinimumValue:"), StepperMin);
    msg1(void, double, v->stepper, sel_getUid("setMaximumValue:"), StepperMax);
    addTarget(v->stepper, self, getValueChangedSel(), ControlEventValueChanged);

    id stack = createHStack((id []){v->label, v->stepper});
    useStackConstraints(stack);
    addSubview(self, stack);
    pin(stack, self);
    releaseView(stack);
    return self;
}

void stepperView_deinit(id self, SEL _cmd) {
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    CFRelease(v->reps);
    releaseView(v->stepper);
    releaseView(v->label);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

static void handleNewStepperValue(id self, StepperView *v, int value) {
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef newVal = formatStr(l, CFSTR("%d"), value);
    CFRelease(l);
    CFStringReplace(v->reps, v->range, newVal);
    v->range.length = CFStringGetLength(newVal);
    CFRelease(newVal);
    setText(v->label, v->reps);
    setAccessibilityValue(self, v->reps);
}

void stepperView_updatedStepper(id self, SEL _cmd _U_) {
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    handleNewStepperValue(self, v, (int)getValue(v->stepper, sgv));
}

static void stepperChangeGeneric(id self, int change) {
    StepperView *v = (StepperView *)((char *)self + ViewSize);
    int value = (int)getValue(v->stepper, sgv) + change;
    if (value >= StepperMin && value <= StepperMax) {
        setStepperValue(v->stepper, value);
        handleNewStepperValue(self, v, value);
    }
}

void stepperView_increment(id self, SEL _cmd _U_) { stepperChangeGeneric(self, 1); }

void stepperView_decrement(id self, SEL _cmd _U_) { stepperChangeGeneric(self, -1); }

#pragma mark - VC

id updateMaxesVC_init(id parent, int index, int bodyweight) {
    id self = new(UpdateMaxesVCClass);
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    d->parent = parent;
    d->index = index;
    d->bodyweight = bodyweight;
    return self;
}

void updateMaxesVC_deinit(id self, SEL _cmd) {
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)self + VCSize + sizeof(InputVC));
    releaseView(d->repsStepper);
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);
}

void updateMaxesVC_viewDidLoad(id self, SEL _cmd) {
    inputVC_viewDidLoad(self, _cmd);

    InputVC *p = (InputVC *)((char *)self + VCSize);
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)p + sizeof(InputVC));
    p->button = createButton(localize(CFSTR("finish")), ColorBlue, self, getTapSel());
    setupNavItem(self, CFSTR("updateMaxesTitle"), (id []){nil, p->button});
    setEnabled(p->button, false);

    if (getTabBarAppearanceClass())
        msg1(void, bool, self, sel_getUid("setModalInPresentation:"), true);

    setSpacing(p->vStack, GroupSpacing);
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef liftKey = formatStr(NULL, CFSTR("exNames%02d"), d->index);
    CFStringRef liftName = localize(liftKey), fieldKey = localize(CFSTR("maxWeight"));
    CFMutableStringRef adjustedName = CFStringCreateMutableCopy(NULL, 128, liftName);
    CFStringLowercase(adjustedName, locale);
    int kbType = getKeyboardForLocale(locale);
    inputVC_addChild(self, formatStr(NULL, fieldKey, adjustedName), kbType, 1, FieldMaxDefault);
    CFRelease(liftKey);
    CFRelease(liftName);
    CFRelease(fieldKey);
    CFRelease(adjustedName);

    id stepperView = stepperView_init(&d->repsStepper, locale);
    addArrangedSubview(p->vStack, stepperView);
    releaseView(stepperView);
}

void updateMaxesVC_tappedFinish(id self, SEL _cmd _U_, id button _U_) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    UpdateMaxesVC *d = (UpdateMaxesVC *)((char *)sup + sizeof(InputVC));
    CFLocaleRef locale = CFLocaleCopyCurrent();
    int extra = d->index == LiftPullup ? d->bodyweight : 0;
    float initWeight = ((sup->children[0].data->result * getSavedMassFactor(locale)) + extra) * 36;
    float reps = 37.f - (float)getValue(d->repsStepper, sgv);
    int weight = (int)lrintf(initWeight / reps) - extra;
    CFRelease(locale);
    dismissPresentedVC(self, ^{ workoutVC_finishedBottomSheet(d->parent, d->index, weight); });
}
