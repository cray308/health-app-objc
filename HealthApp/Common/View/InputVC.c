#include "InputVC.h"
#include <CoreFoundation/CFNotificationCenter.h>
#include "AppUserData.h"
#include "ViewControllerHelpers.h"

#define toggleScrolling(_v, _enable) msg1(void, bool, _v, sel_getUid("setScrollEnabled:"), _enable)

#define setScrollInsets(_v, _mg) msg1(void, HAInsets, _v, sel_getUid("setContentInset:"), _mg)

extern id UIKeyboardDidShowNotification;
extern id UIKeyboardWillHideNotification;
extern id UIKeyboardFrameEndUserInfoKey;

Class InputVCClass;
Class InputViewClass;
size_t InputVCSize;

static CFStringRef inputFieldError;

static void keyboardShown(CFNotificationCenterRef center _U_,
                          void *observer, CFNotificationName name _U_,
                          const void *object _U_, CFDictionaryRef userinfo) {
    InputVC *data = (InputVC *) ((char *)observer + VCSize);
    toggleScrolling(data->scrollView, true);
    id value = (id) CFDictionaryGetValue(userinfo, (CFStringRef) UIKeyboardFrameEndUserInfoKey);
    CGRect kbRect, viewRect, fieldRect, fieldInView;
    id view = getView((id) observer);
    getRect(data->activeField, &fieldRect, RectBounds);
    getRect(view, &viewRect, RectFrame);

#if defined(__arm64__)
    kbRect = msg0(CGRect, value, sel_getUid("CGRectValue"));
    fieldInView = msg2(CGRect, CGRect, id, view,
                       sel_getUid("convertRect:fromView:"), fieldRect, data->activeField);
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&kbRect, value, sel_getUid("CGRectValue"));
    (((void(*)(CGRect*,id,SEL,CGRect,id))objc_msgSend_stret)
     (&fieldInView, view, sel_getUid("convertRect:fromView:"), fieldRect, data->activeField));
#endif

    viewRect.size.height -= kbRect.size.height;
    setScrollInsets(data->scrollView, ((HAInsets){data->topOffset, 0, kbRect.size.height, 0}));

    CGPoint upperY = {fieldInView.origin.x, fieldInView.origin.y + fieldInView.size.height};
    if (!(CGRectContainsPoint(viewRect, fieldInView.origin) &&
          CGRectContainsPoint(viewRect, upperY))) {
        msg2(void, CGRect, bool, data->scrollView,
             sel_getUid("scrollRectToVisible:animated:"), fieldRect, true);
    }
}

static void keyboardWillHide(CFNotificationCenterRef center _U_,
                             void *observer, CFNotificationName name _U_,
                             const void *object _U_, CFDictionaryRef userinfo _U_) {
    InputVC *data = (InputVC *) ((char *)observer + VCSize);
    setScrollInsets(data->scrollView, ((HAInsets){data->topOffset, 0, data->bottomOffset, 0}));
    CGRect bounds;
    getRect(data->vStack, &bounds, RectBounds);
    toggleScrolling(data->scrollView, (int) bounds.size.height >= data->scrollHeight);
}

void initValidatorStrings(CFBundleRef bundle) {
    inputFieldError = CFBundleCopyLocalizedString(bundle, CFSTR("inputFieldError"), NULL, NULL);
}

static void inputView_reset(InputView *data, short value) {
    data->valid = true;
    data->result = value;
    hideView(data->errorLabel, true);
    setAccessibilityLabel(data->field, getText(data->hintLabel));
}

static void showInputError(InputView *child) {
    child->valid = false;
    hideView(child->errorLabel, false);
    CFStringRef hintText = getText(child->hintLabel);
    CFStringRef errorText = getText(child->errorLabel);
    CFStringRef text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@. %@"), hintText, errorText);
    setAccessibilityLabel(child->field, text);
    CFRelease(text);
}

void inputVC_addChild(id self, CFStringRef hint, short min, short max) {
    InputVC *d = (InputVC *) ((char *)self + VCSize);

    int index = d->count++;
    id view = createNew(InputViewClass);
    d->children[index] = (InputView *) ((char *)view + ViewSize);
    InputView *ptr = d->children[index];
    ptr->minVal = min;
    ptr->maxVal = max;
    CFStringRef errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    CFRetain(hint);
    ptr->hintLabel = createLabel(hint, UIFontTextStyleFootnote, false);
    ptr->field = createTextfield(self, CFSTR(""), hint, 4, 4, index);
    if (d->setKB)
        setKBColor(ptr->field, 1);
    ptr->errorLabel = createLabel(errorText, UIFontTextStyleFootnote, false);
    setTextColor(ptr->errorLabel, createColor(ColorRed));

    id vStack = createStackView((id []){ptr->hintLabel, ptr->field, ptr->errorLabel},
                                3, 1, 4, (Padding){4, 8, 4, 8});
    setUsesAutolayout(vStack);
    addSubview(view, vStack);
    pin(vStack, view);
    addArrangedSubview(d->vStack, view);
    releaseObj(vStack);
    hideView(ptr->errorLabel, true);
    setInputAccessory(ptr->field, d->toolbar);
}

void inputVC_updateFields(InputVC *self, short *vals) {
    int count = self->count;
    for (int i = 0; i < count; ++i) {
        short value = vals[i];
        InputView *child = self->children[i];
        CFStringRef str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), value);
        setLabelText(child->field, str);
        inputView_reset(child, value);
        CFRelease(str);
    }
    enableButton(self->button, true);
}

void inputView_deinit(id self, SEL _cmd) {
    InputView *ptr = (InputView *) ((char *)self + ViewSize);
    struct objc_super super = {self, ViewClass};
    releaseObj(ptr->hintLabel);
    releaseObj(ptr->field);
    releaseObj(ptr->errorLabel);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

id inputVC_init(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    return ((id(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputVC_deinit(id self, SEL _cmd) {
    InputVC *data = (InputVC *) ((char *)self + VCSize);
    struct objc_super super = {self, VCClass};

    for (int i = 0; i < 4; ++i) {
        InputView *child = data->children[i];
        if (child)
            releaseObj((id) ((char *)child - ViewSize));
    }
    releaseObj(data->toolbar);
    releaseObj(data->scrollView);
    releaseObj(data->vStack);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CGRect bounds;
    getScreenBounds(&bounds);
    CGFloat width = bounds.size.width;
    unsigned char dark = userData->darkMode;

    InputVC *data = (InputVC *) ((char *)self + VCSize);
    data->scrollView = createScrollView();
    data->vStack = createStackView(nil, 0, 1, 0, (Padding){0});
    data->toolbar = createObjectWithFrame(objc_getClass("UIToolbar"), ((CGRect){{0}, {width, 50}}));
    if (!(dark & 128)) {
        setBarTint(data->toolbar, getBarColor(ColorBarModal));
        data->setKB = dark == 1;
    }
    msg0(void, data->toolbar, sel_getUid("sizeToFit"));

    CFStringRef doneLabel = CFBundleCopyLocalizedString(CFBundleGetMainBundle(),
                                                        CFSTR("done"), NULL, NULL);
    Class btnClass = objc_getClass("UIBarButtonItem");
    id flexSpace = msg3(id, long, id, SEL, allocClass(btnClass),
                        sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = msg4(id, CFStringRef, long, id, SEL, allocClass(btnClass),
                         sel_getUid("initWithTitle:style:target:action:"),
                         doneLabel, 0, self, sel_getUid("dismissKeyboard"));
    setTintColor(doneButton, createColor(ColorRed));
    CFRelease(doneLabel);

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){flexSpace, doneButton},
                                     2, &retainedArrCallbacks);
    msg2(void, CFArrayRef, bool, data->toolbar, sel_getUid("setItems:animated:"), array, false);

    addVStackToScrollView(getView(self), data->vStack, data->scrollView);
    CFRelease(array);
    releaseObj(flexSpace);
    releaseObj(doneButton);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, keyboardShown,
                                    (CFStringRef) UIKeyboardDidShowNotification, NULL,
                                    CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, keyboardWillHide,
                                    (CFStringRef) UIKeyboardWillHideNotification, NULL,
                                    CFNotificationSuspensionBehaviorDrop);
}

void inputVC_viewDidAppear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    InputVC *data = (InputVC *) ((char *)self + VCSize);
    if (!data->scrollHeight) {
        CGRect bounds;
        HAInsets insets;
        getRect(data->scrollView, &bounds, RectBounds);
        data->scrollHeight = (int) bounds.size.height;
#if defined(__arm64__)
        insets = msg0(HAInsets, data->scrollView, sel_getUid("contentInset"));
#else
        ((void(*)(HAInsets*,id,SEL))objc_msgSend_stret)(&insets, data->scrollView,
                                                        sel_getUid("contentInset"));
#endif
        data->topOffset = (short) insets.top;
        data->bottomOffset = (short) insets.bottom;
    }
}

void inputVC_dismissKeyboard(id self, SEL _cmd _U_) {
    InputVC *d = (InputVC *) ((char *)self + VCSize);
    int tag = d->activeField ? ((int) getTag(d->activeField)) : 255;
    if (tag >= d->count - 1) {
        id view = getView(self);
        msg1(void, bool, view, sel_getUid("endEditing:"), true);
    } else {
        msg0(bool, d->children[tag + 1]->field, sel_getUid("becomeFirstResponder"));
    }
}

void inputVC_fieldBeganEditing(id self, SEL _cmd _U_, id field) {
    ((InputVC *) ((char *)self + VCSize))->activeField = field;
}

void inputVC_fieldStoppedEditing(id self, SEL _cmd _U_, id field _U_) {
    ((InputVC *) ((char *)self + VCSize))->activeField = nil;
}

bool inputVC_fieldChanged(id self, SEL _cmd _U_, id field, CFRange range, CFStringRef replacement) {
    InputVC *d = (InputVC *) ((char *)self + VCSize);

    int len = (int) CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            UniChar val = CFStringGetCharacterFromInlineBuffer(&buf, i);
            if (val < 48 || val > 57) return false;
        }
    }

    int i = (int) getTag(field);
    InputView *child = d->children[i];

    CFStringRef text = getText(field);
    if (range.location + range.length > CFStringGetLength(text)) return false;
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text);
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        enableButton(d->button, false);
        showInputError(child);
        return true;
    }

    int newVal = (int) CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < child->minVal || newVal > child->maxVal) {
        enableButton(d->button, false);
        showInputError(child);
        return true;
    }

    inputView_reset(child, (short) newVal);
    for (i = 0; i < d->count; ++i) {
        if (!d->children[i]->valid) return true;
    }

    enableButton(d->button, true);
    return true;
}
