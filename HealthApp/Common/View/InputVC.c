#include "InputVC.h"
#include <unicode/uregex.h>
#include "UserData.h"
#include "Views.h"

extern CFStringRef UIKeyboardDidShowNotification;
extern CFStringRef UIKeyboardWillHideNotification;
extern CFStringRef UIKeyboardFrameEndUserInfoKey;
extern CFStringRef UIFontTextStyleFootnote;

Class InputViewClass;
Class InputVCClass;

static CFStringRef inputErrorFormat;
static CFCharacterSetRef metricChars;
static CFNumberFormatterRef formatter;
static URegularExpression *regex;
static SEL vcrv;
static generateRectFunctionSignature(cgRectValue);

static void keyboardShown(CFNotificationCenterRef, void *,
                          CFStringRef, const void *, CFDictionaryRef);
static void keyboardWillHide(CFNotificationCenterRef, void *,
                             CFStringRef, const void *, CFDictionaryRef);

void initValidatorData(void) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (isMetric(locale)) {
        CFCharacterSetRef baseSet = CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit);
        CFMutableCharacterSetRef mutableSet = CFCharacterSetCreateMutableCopy(NULL, baseSet);
        CFStringRef decimal = CFLocaleGetValue(locale, kCFLocaleDecimalSeparator);
        CFCharacterSetAddCharactersInString(mutableSet, decimal);
        metricChars = CFCharacterSetCreateCopy(NULL, mutableSet);
        CFRelease(mutableSet);
        regex = uregex_open(u"[٠١٢٣٤٥٦٧٨٩0123456789]+(?:[٫.,][٠١٢٣٤٥٦٧٨٩0123456789]{0,2})?",
                            60, 0, NULL, &(UErrorCode){0});
    }
    formatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);
    CFRelease(locale);
    inputErrorFormat = localize(CFSTR("inputFieldError"));
    vcrv = sel_getUid("CGRectValue");
    cgRectValue = getRectMethodImplementation(objc_getClass("NSValue"), vcrv);
}

static void scrollToView(id scrollView, id child) {
    CGRect fieldRect, fieldInView;
    getBounds(fieldRect, child);
    convertRect(fieldInView, scrollView, fieldRect, child);
    fieldInView.size.height += 32;
    fieldInView.origin.y -= 16;
    scrollRectToVisible(scrollView, fieldInView);
}

#pragma mark - Input View

void inputView_deinit(id self, SEL _cmd) {
    InputView *v = getIVV(InputView, self);
    releaseView(v->hintLabel);
    releaseView(v->field);
    releaseView(v->errorLabel);
    msgSupV(supSig(), self, View, _cmd);
}

static void resetInput(InputView *v, float value) {
    if (!v->state) setHidden(v->errorLabel, true);
    v->state = 1;
    v->result = value;
}

#pragma mark - VC - Public Functions

void inputVC_addField(id self, CFStringRef hint, int keyboardType, int min, int max) {
    InputVC *d = getIVVC(InputVC, self);

    int index = d->count++;
    IVPair *pair = &d->children[index];
    pair->view = new(InputViewClass);
    pair->data = getIVV(InputView, pair->view);
    pair->data->min = min;
    pair->data->max = max;
    pair->data->state = UCHAR_MAX;
    pair->data->chars = keyboardType == KeyboardTypeNumberPad
                        ? CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit) : metricChars;

    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef errorText = formatStr(locale, inputErrorFormat, min, max);
    CFRelease(locale);

    id hintLabel = createLabel(CFRetain(hint), UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(hintLabel, false);
    pair->data->hintLabel = hintLabel;

    pair->data->field = createTextField(self, d->toolbar, hint, index);
    setKeyboardType(pair->data->field, keyboardType);
    setAccessibilityHint(pair->data->field, errorText);
    setKeyboardAppearance(pair->data->field, d->keyboardAppearance);

    pair->data->errorLabel = createLabel(errorText, UIFontTextStyleFootnote, ColorRed);
    setIsAccessibilityElement(pair->data->errorLabel, false);
    setHidden(pair->data->errorLabel, true);

    id vStack = createVStack((id []){hintLabel, pair->data->field, pair->data->errorLabel}, 3);
    useStackConstraints(vStack);
    setSpacing(vStack, ViewSpacing);
    setLayoutMargins(vStack, ViewMargins);
    addSubview(pair->view, vStack);
    pin(vStack, pair->view);
    addArrangedSubview(d->vStack, pair->view);
    releaseView(vStack);
}

void inputVC_updateFields(InputVC *d, int const *values) {
    CFLocaleRef l = CFLocaleCopyCurrent();
    if (isMetric(l)) {
        for (int i = 0; i < 4; ++i) {
            InputView *v = d->children[i].data;
            float value = values[i] * ToKg;
            CFStringRef str = formatStr(l, CFSTR("%.2f"), value);
            setFieldText(v->field, str);
            resetInput(v, value);
            CFRelease(str);
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            InputView *v = d->children[i].data;
            CFStringRef str = formatStr(l, CFSTR("%d"), values[i]);
            setFieldText(v->field, str);
            resetInput(v, values[i]);
            CFRelease(str);
        }
    }
    setEnabled(d->button, true);
    CFRelease(l);
}

#pragma mark - Lifecycle

void inputVC_deinit(id self, SEL _cmd) {
    InputVC *d = getIVVC(InputVC, self);
    for (int i = 0; i < 4; ++i) {
        if (d->children[i].view) releaseView(d->children[i].view);
    }
    releaseView(d->toolbar);
    releaseView(d->scrollView);
    releaseView(d->vStack);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    msgSupV(supSig(), self, VC, _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    msgSupV(supSig(), self, VC, _cmd);

    InputVC *d = getIVVC(InputVC, self);
    d->activeTag = -1;

    CFStringRef doneText = localize(CFSTR("done"));
    id items[] = {
        createBarButtonItemWithImage(CFSTR("ico_chevron_up"), self, getJumpToPrevSel()),
        createBarButtonItemWithImage(CFSTR("ico_chevron_down"), self, getJumpToNextSel()),
        msgV(objSig(id, long, id, SEL), alloc(BarButtonItem),
             sel_getUid("initWithBarButtonSystemItem:target:action:"),
             5, nil, nil),
        msgV(objSig(id, CFStringRef, long, id, SEL), alloc(BarButtonItem),
             sel_getUid("initWithTitle:style:target:action:"),
             doneText, 2, self, getDismissKeyboardSel())
    };
    CFRelease(doneText);

    d->toolbar = msgV(objSig(id, CGRect), alloc(objc_getClass("UIToolbar")),
                      sel_getUid("initWithFrame:"), (CGRect){{0}, {100, 100}});
    uint8_t darkMode = getUserData()->darkMode;
    if (isCharValid(darkMode)) {
        setBarTintColor(d->toolbar, getBarColor(BarColorModal));
        d->keyboardAppearance = darkMode;
    }
    setTintColor(d->toolbar, getColor(ColorRed));
    msgV(objSig(void), d->toolbar, sel_getUid("sizeToFit"));
    CFArrayRef itemArr = CFArrayCreate(NULL, (const void **)items, 4, &RetainedArrCallbacks);
    msgV(objSig(void, CFArrayRef), d->toolbar, sel_getUid("setItems:"), itemArr);
    CFRelease(itemArr);
    for (int i = 0; i < 4; ++i) {
        releaseObject(items[i]);
    }

    d->scrollView = retainView(createScrollView());
    d->vStack = retainView(createVStack(NULL, 0));
    setupHierarchy(self, d->vStack, d->scrollView, ColorSecondaryBG);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, keyboardShown, UIKeyboardDidShowNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, keyboardWillHide, UIKeyboardWillHideNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

#pragma mark - Keyboard

void keyboardShown(CFNotificationCenterRef center _U_, void *self,
                   CFStringRef name _U_, const void *object _U_, CFDictionaryRef userInfo) {
    InputVC *d = getIVVC(InputVC, self);
    if (d->activeTag < 0) return;

    id value = (id)CFDictionaryGetValue(userInfo, UIKeyboardFrameEndUserInfoKey);
    CGRect kbRect;
    callRectMethod(cgRectValue, kbRect, value, vcrv);
    setContentInset(d->scrollView, ((HAInsets){.bottom = kbRect.size.height}));
    scrollToView(d->scrollView, d->children[d->activeTag].view);
}

void keyboardWillHide(CFNotificationCenterRef center _U_, void *self,
                      CFStringRef name _U_, const void *object _U_, CFDictionaryRef userInfo _U_) {
    setContentInset(getIVVC(InputVC, self)->scrollView, ((HAInsets){0}));
}

void inputVC_dismissKeyboard(id self, SEL _cmd _U_) {
    msgV(objSig(bool, bool), getView(self), sel_getUid("endEditing:"), true);
}

static void changeFocusedField(id self, int offset) {
    InputVC *d = getIVVC(InputVC, self);
    int nextTag = d->activeTag + offset;
    if (!nextTag) {
        if (d->count) {
            becomeFirstResponder(d->children[0].data->field);
        } else {
            inputVC_dismissKeyboard(self, nil);
        }
        return;
    }

    id nextView = viewWithTag(d->vStack, nextTag);
    if (nextView) {
        becomeFirstResponder(nextView);
    } else {
        inputVC_dismissKeyboard(self, nil);
    }
}

void inputVC_jumpToPrev(id self, SEL _cmd _U_) { changeFocusedField(self, -1); }

void inputVC_jumpToNext(id self, SEL _cmd _U_) { changeFocusedField(self, 1); }

#pragma mark - UITextFieldDelegate

void inputVC_textFieldDidBeginEditing(id self, SEL _cmd _U_, id field) {
    getIVVC(InputVC, self)->activeTag = (int)getTag(field);
}

void inputVC_textFieldDidEndEditing(id self, SEL _cmd _U_, id field _U_) {
    getIVVC(InputVC, self)->activeTag = -2;
}

static void showError(InputVC *d, IVPair *pair) {
    if (pair->data->state) {
        setEnabled(d->button, false);
        pair->data->state = 0;
        setHidden(pair->data->errorLabel, false);
        msgV(objSig(void), pair->view, sel_getUid("layoutIfNeeded"));
    }
    scrollToView(d->scrollView, pair->view);
    if (UIAccessibilityIsVoiceOverRunning()) {
        CFStringRef message = getText(pair->data->errorLabel);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1500000000), dispatch_get_main_queue(), ^{
            UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)message);
        });
    }
}

bool inputVC_shouldChange(id self, SEL _cmd _U_, id field, CFRange range, CFStringRef replacement) {
    static UChar buffer[32];
    InputVC *d = getIVVC(InputVC, self);

    IVPair *pair = &d->children[d->activeTag];
    int len = (int)CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int c = 0; c < len; ++c) {
            if (!CFCharacterSetIsCharacterMember(
                   pair->data->chars, CFStringGetCharacterFromInlineBuffer(&buf, c))) return false;
        }
    }

    CFStringRef text = getFieldText(field);
    long oldLen = CFStringGetLength(text);
    if (range.location + range.length > oldLen) return false;
    int newLen = (int)(oldLen + len - range.length);
    if (newLen > 16) return false;
    if (!newLen) {
        showError(d, pair);
        return true;
    }

    CFMutableStringRef newText = CFStringCreateMutableWithExternalCharactersNoCopy(
      NULL, buffer, 0, 32, kCFAllocatorNull);
    CFStringAppend(newText, text);
    CFStringReplace(newText, range, replacement);

    if (len && pair->data->chars == metricChars) {
        UErrorCode err = 0;
        uregex_setText(regex, buffer, newLen, &err);
        UBool matches = uregex_matches(regex, 0, &err);
        uregex_reset(regex, 0, &err);
        if (!matches) {
            CFRelease(newText);
            return false;
        }
    }

    CFNumberRef num = CFNumberFormatterCreateNumberFromString(NULL, formatter, newText, NULL, 0);
    float newVal = -1;
    CFRelease(newText);
    if (num != NULL) {
        CFNumberGetValue(num, kCFNumberFloatType, &newVal);
        CFRelease(num);
    }

    if (newVal < pair->data->min || newVal > pair->data->max) {
        showError(d, pair);
        return true;
    }

    resetInput(pair->data, newVal);
    for (int i = 0; i < d->count; ++i) {
        if (d->children[i].data->state != 1) return true;
    }

    setEnabled(d->button, true);
    return true;
}
