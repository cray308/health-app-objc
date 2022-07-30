#include "InputVC.h"
#include <unicode/uregex.h>
#include "Views.h"

extern CFStringRef UIKeyboardDidShowNotification;
extern CFStringRef UIKeyboardWillHideNotification;
extern CFStringRef UIKeyboardFrameEndUserInfoKey;
extern CFStringRef UIFontTextStyleFootnote;

Class InputViewClass;
Class InputVCClass;

enum {
    UIBarButtonItemStylePlain,
    UIBarButtonItemStyleDone = 2
};

enum {
    UIBarButtonSystemItemFlexibleSpace = 5
};

static CFStringRef inputFieldError;
static CFCharacterSetRef charSet;
static CFNumberFormatterRef numFormatter;
static URegularExpression *regex;

static void keyboardShown(CFNotificationCenterRef, void *,
                          CFNotificationName, const void *, CFDictionaryRef);
static void keyboardWillHide(CFNotificationCenterRef, void *,
                             CFNotificationName, const void *, CFDictionaryRef);

void initValidatorData(void) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (CFBooleanGetValue(CFLocaleGetValue(locale, kCFLocaleUsesMetricSystem))) {
        CFCharacterSetRef baseSet = CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit);
        CFMutableCharacterSetRef ms = CFCharacterSetCreateMutableCopy(NULL, baseSet);
        CFCharacterSetAddCharactersInString(ms, CFLocaleGetValue(locale, kCFLocaleDecimalSeparator));
        charSet = CFCharacterSetCreateCopy(NULL, ms);
        CFRelease(ms);
        const UChar *pattern = u"([٠١٢٣٤٥٦٧٨٩0123456789]+)([٫.,][٠١٢٣٤٥٦٧٨٩0123456789]{0,2})?";
        regex = uregex_open(pattern, 60, 0, NULL, &(UErrorCode){0});
    }
    numFormatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);
    CFRelease(locale);
    inputFieldError = localize(CFSTR("inputFieldError"));
}

static void checkScrollPos(id scroll, id child) {
    CGRect bounds, rect;
    getRect(bounds, child, Vtbl.vc.bnd);
    convertRect(rect, bounds, scroll, child);
    rect.size.height += 32;
    rect.origin.y -= 16;
    scrollRect(scroll, rect);
}

#pragma mark - Input View

void inputView_deinit(id self, SEL _cmd) {
    InputView *v = (InputView *)getIVV(self);
    releaseV(v->field);
    releaseV(v->errorLabel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

static void inputView_reset(InputView *v, float value) {
    v->valid = true;
    v->result = value;
    setHidden(v->errorLabel, true);
}

static void showInputError(IVPair *pair, InputVC *d) {
    setEnabled(d->button, false);
    pair->data->valid = false;
    setHidden(pair->data->errorLabel, false);
    layoutIfNeeded(pair->view);
    checkScrollPos(d->scrollView, pair->view);
    if (UIAccessibilityIsVoiceOverRunning()) {
        CFStringRef message = getText(pair->data->errorLabel);
        dispatch_after(dispatch_time(0, 1500000000), dispatch_get_main_queue(), ^{
            UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)message);
        });
    }
}

#pragma mark - VC - Public Functions

void inputVC_addChild(id self, CFStringRef hint, int kb, short min, short max) {
    InputVC *d = (InputVC *)getIVVC(self);

    int index = d->count++;
    id view = new(InputViewClass);
    InputView *v = (InputView *)getIVV(view);
    d->children[index].view = view;
    d->children[index].data = v;
    v->minVal = min;
    v->maxVal = max;
    v->set = kb == UIKeyboardTypeNumberPad
             ? CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit) : charSet;

    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef errorText = formatStr(locale, inputFieldError, min, max);
    CFRelease(locale);

    id hintLabel = createLabel(CFRetain(hint), UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(hintLabel, false);

    v->field = createTextfield(self, d->toolbar, hint, index);
    setKeyboardType(v->field, kb);
    setAccessibilityHint(v->field, errorText);

    v->errorLabel = createLabel(errorText, UIFontTextStyleFootnote, ColorRed);
    setIsAccessibilityElement(v->errorLabel, false);
    setHidden(v->errorLabel, true);

    id vStack = createVStack((id []){hintLabel, v->field, v->errorLabel}, 3);
    setTrans(vStack);
    setSpacing(vStack, ViewSpacing);
    setLayoutMargins(vStack, ViewMargins);
    addSubview(view, vStack);
    pin(vStack, view);
    addArrangedSubview(d->vStack, view);
    releaseV(vStack);
    releaseV(hintLabel);
}

void inputVC_updateFields(InputVC *d, const short *vals) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (CFBooleanGetValue(CFLocaleGetValue(locale, kCFLocaleUsesMetricSystem))) {
        for (int i = 0; i < 4; ++i) {
            float value = vals[i] * ToKg;
            InputView *v = d->children[i].data;
            CFStringRef str = formatStr(locale, CFSTR("%.2f"), value);
            setText(v->field, str);
            inputView_reset(v, value);
            CFRelease(str);
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            InputView *v = d->children[i].data;
            CFStringRef str = formatStr(locale, CFSTR("%d"), vals[i]);
            setText(v->field, str);
            inputView_reset(v, vals[i]);
            CFRelease(str);
        }
    }
    setEnabled(d->button, true);
    CFRelease(locale);
}

#pragma mark - Lifecycle

void inputVC_deinit(id self, SEL _cmd) {
    InputVC *d = (InputVC *)getIVVC(self);
    for (int i = 0; i < 4; ++i) {
        if (d->children[i].view) releaseV(d->children[i].view);
    }
    releaseV(d->toolbar);
    releaseV(d->scrollView);
    releaseV(d->vStack);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    InputVC *d = (InputVC *)getIVVC(self);

    CFStringRef doneLabel = localize(CFSTR("done"));
    Class BarItem = objc_getClass("UIBarButtonItem");
    SEL btnInit = sel_getUid("initWithImage:style:target:action:");
    id items[] = {
        (((id(*)(id,SEL,id,long,id,SEL))objc_msgSend)
         (alloc(BarItem), btnInit,
          sysImg(CFSTR("chevron.up")), UIBarButtonItemStylePlain, self, sel_getUid("jumpToPrev"))),
        (((id(*)(id,SEL,id,long,id,SEL))objc_msgSend)
         (alloc(BarItem), btnInit,
          sysImg(CFSTR("chevron.down")), UIBarButtonItemStylePlain, self, sel_getUid("jumpToNext"))),
        msg3(id, long, id, SEL, alloc(BarItem),
             sel_getUid("initWithBarButtonSystemItem:target:action:"),
             UIBarButtonSystemItemFlexibleSpace, nil, nil),
        (((id(*)(id,SEL,CFStringRef,long,id,SEL))objc_msgSend)
         (alloc(BarItem), sel_getUid("initWithTitle:style:target:action:"),
          doneLabel, UIBarButtonItemStyleDone, self, sel_getUid("dismissKeyboard")))
    };
    CFRelease(doneLabel);

    id _t = alloc(objc_getClass("UIToolbar"));
    d->toolbar = msg1(id, CGRect, _t, sel_getUid("initWithFrame:"), ((CGRect){{0}, {100, 100}}));
    msg1(void, id, d->toolbar, sel_getUid("setTintColor:"), getColor(ColorRed));
    msg0(void, d->toolbar, sel_getUid("sizeToFit"));
    CFArrayRef array = CFArrayCreate(NULL, (const void **)items, 4, &retainedArrCallbacks);
    msg2(void, CFArrayRef, bool, d->toolbar, sel_getUid("setItems:animated:"), array, false);
    CFRelease(array);
    for (int i = 0; i < 4; ++i) {
        releaseO(items[i]);
    }

    SEL retain = sel_getUid("retain");
    d->scrollView = msg0(id, createScrollView(), retain);
    d->vStack = msg0(id, createVStack(nil, 0), retain);
    setLayoutMargins(d->vStack, VCMargins);
    setupHierarchy(self, d->vStack, d->scrollView, ColorSecondaryBG);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, keyboardShown, UIKeyboardDidShowNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, keyboardWillHide, UIKeyboardWillHideNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

#pragma mark - Keyboard

void keyboardShown(CFNotificationCenterRef ctr _U_, void *self,
                   CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info) {
    InputVC *d = (InputVC *)getIVVC(self);
    int index = (int)getTag(d->activeField);
    if (index < 0) return;

    id value = (id)CFDictionaryGetValue(info, UIKeyboardFrameEndUserInfoKey);
    CGRect kbRect;
    getRect(kbRect, value, sel_getUid("CGRectValue"));
    setContentInset(d->scrollView, ((HAInsets){.bottom = kbRect.size.height}));
    checkScrollPos(d->scrollView, d->children[index].view);
}

void keyboardWillHide(CFNotificationCenterRef ctr _U_, void *self,
                      CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    setContentInset(((InputVC *)getIVVC(self))->scrollView, ((HAInsets){0}));
}

void inputVC_dismissKeyboard(id self, SEL _cmd _U_) {
    msg1(bool, bool, msg0(id, self, sel_getUid("view")), sel_getUid("endEditing:"), true);
}

static void handleFieldChange(id self, int offset) {
    InputVC *d = (InputVC *)getIVVC(self);
    long nextTag = getTag(d->activeField) + offset;
    if (!nextTag) {
        if (d->count) {
            becomeFirstResponder(d->children[0].data->field);
        } else {
            inputVC_dismissKeyboard(self, nil);
        }
        return;
    }

    id next = viewWithTag(d->vStack, nextTag);
    if (next) {
        becomeFirstResponder(next);
    } else {
        inputVC_dismissKeyboard(self, nil);
    }
}

void inputVC_jumpToPrev(id self, SEL _cmd _U_) { handleFieldChange(self, -1); }

void inputVC_jumpToNext(id self, SEL _cmd _U_) { handleFieldChange(self, 1); }

#pragma mark - UITextFieldDelegate

void inputVC_fieldBeganEditing(id self, SEL _cmd _U_, id field) {
    ((InputVC *)getIVVC(self))->activeField = field;
}

void inputVC_fieldStoppedEditing(id self, SEL _cmd _U_, id field _U_) {
    ((InputVC *)getIVVC(self))->activeField = nil;
}

bool inputVC_fieldShouldReturn(id self _U_, SEL _cmd _U_, id field) {
    return msg0(bool, field, sel_getUid("resignFirstResponder"));
}

bool inputVC_fieldChanged(id self, SEL _cmd _U_, id field, CFRange range, CFStringRef replacement) {
    static UChar buffer[32];
    InputVC *d = (InputVC *)getIVVC(self);

    IVPair *pair = &d->children[getTag(field)];
    int len = (int)CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            UniChar c = CFStringGetCharacterFromInlineBuffer(&buf, i);
            if (!CFCharacterSetIsCharacterMember(pair->data->set, c)) return false;
        }
    }

    CFStringRef text = getText(field);
    long oldLen = CFStringGetLength(text);
    if (range.location + range.length > oldLen) return false;

    int newLen = (int)(oldLen + len - range.length);
    if (newLen > 16) return false;
    else if (!newLen) {
        showInputError(pair, d);
        return true;
    }

    CFMutableStringRef newText = CFStringCreateMutableWithExternalCharactersNoCopy(
      NULL, buffer, 0, 32, kCFAllocatorNull);
    CFStringAppend(newText, text);
    CFStringReplace(newText, range, replacement);

    if (len && pair->data->set == charSet) {
        UErrorCode err = 0;
        uregex_setText(regex, buffer, newLen, &err);
        UBool matches = uregex_matches(regex, 0, &err);
        uregex_reset(regex, 0, &err);
        if (!matches) {
            CFRelease(newText);
            return false;
        }
    }

    CFNumberRef num = CFNumberFormatterCreateNumberFromString(NULL, numFormatter, newText, NULL, 0);
    float newVal = -1;
    CFRelease(newText);
    if (num != NULL) {
        CFNumberGetValue(num, kCFNumberFloatType, &newVal);
        CFRelease(num);
    }

    if (newVal < pair->data->minVal || newVal > pair->data->maxVal) {
        showInputError(pair, d);
        return true;
    }

    inputView_reset(pair->data, newVal);
    for (int i = 0; i < d->count; ++i) {
        if (!d->children[i].data->valid) return true;
    }

    setEnabled(d->button, true);
    return true;
}
