#include "InputVC.h"
#include "ViewControllerHelpers.h"

#define toggleScrolling(_v, _enable) setBool(_v, sel_getUid("setScrollEnabled:"), _enable)

#define setScrollInsets(_v, _mg) (((void(*)(id,SEL,HAInsets))objc_msgSend)\
((_v), sel_getUid("setContentInset:"), (_mg)))

extern id UIKeyboardDidShowNotification;
extern id UIKeyboardWillHideNotification;
extern id UIKeyboardFrameEndUserInfoKey;

Class InputVCClass;
Class InputViewClass;
size_t InputVCSize;

static CFStringRef inputFieldError;

void initValidatorStrings(void) {
    inputFieldError = localize(CFSTR("inputFieldError"));
}

static void showInputError(InputVC *data, InputView *child) {
    enableButton(data->button, false);
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
    d->children[index] = createNew(InputViewClass);
    id view = d->children[index];
    InputView *ptr = (InputView *) ((char *)view + ViewSize);
    ptr->minVal = min;
    ptr->maxVal = max;
    CFStringRef errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    ptr->hintLabel = createLabel(hint, TextFootnote, 4, false);
    ptr->field = createTextfield(self, CFSTR(""), hint, 4, 4, index);
    ptr->errorLabel = createLabel(errorText, TextFootnote, 4, false);
    setTextColor(ptr->errorLabel, createColor(ColorRed));

    id vStack = createStackView((id []){ptr->hintLabel, ptr->field, ptr->errorLabel},
                                3, 1, 4, (Padding){4, 8, 4, 8});
    addSubview(view, vStack);
    pin(vStack, view, (Padding){0}, 0);
    addArrangedSubview(d->vStack, view);
    releaseObj(vStack);
    CFRelease(errorText);
    hideView(ptr->errorLabel, true);
    setInputAccessory(ptr->field, d->toolbar);
}

void inputView_deinit(id self, SEL _cmd) {
    InputView *ptr = (InputView *) ((char *)self + ViewSize);
    struct objc_super super = {self, ViewClass};
    releaseObj(ptr->hintLabel);
    releaseObj(ptr->field);
    releaseObj(ptr->errorLabel);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputView_reset(InputView *data, short value) {
    data->valid = true;
    data->result = value;
    hideView(data->errorLabel, true);
    setAccessibilityLabel(data->field, getText(data->hintLabel));
}

id inputVC_init(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    return ((id(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputVC_deinit(id self, SEL _cmd) {
    InputVC *data = (InputVC *) ((char *)self + VCSize);
    struct objc_super super = {self, VCClass};

    for (int i = 0; i < 4; ++i) {
        if (data->children[i])
            releaseObj(data->children[i]);
    }
    releaseObj(data->toolbar);
    releaseObj(data->scrollView);
    releaseObj(data->vStack);
    id center = getDeviceNotificationCenter();
    SEL sig = sel_getUid("removeObserver:name:object:");
    ((void(*)(id,SEL,id,id,id))objc_msgSend)(center, sig, self, UIKeyboardDidShowNotification, nil);
    ((void(*)(id,SEL,id,id,id))objc_msgSend)(center, sig,
                                             self, UIKeyboardWillHideNotification, nil);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CGRect bounds;
    getScreenBounds(&bounds);
    CGFloat width = bounds.size.width;

    InputVC *data = (InputVC *) ((char *)self + VCSize);
    data->scrollView = createScrollView();
    data->vStack = createStackView(nil, 0, 1, 0, (Padding){0});
    data->toolbar = createObjectWithFrame(objc_getClass("UIToolbar"), ((CGRect){{0}, {width, 50}}));
    if (osVersion < 13)
        setBarTint(data->toolbar, getBarColor(ColorBarModal));
    voidFunc(data->toolbar, sel_getUid("sizeToFit"));

    Class btnClass = objc_getClass("UIBarButtonItem");
    id flexSpace = (((id(*)(id,SEL,int,id,SEL))objc_msgSend)
                    (allocClass(btnClass), sel_getUid("initWithBarButtonSystemItem:target:action:"),
                     5, nil, nil));
    id doneButton = (((id(*)(id,SEL,CFStringRef,int,id,SEL))objc_msgSend)
                     (allocClass(btnClass), sel_getUid("initWithTitle:style:target:action:"),
                      localize(CFSTR("done")), 0, self, sel_getUid("dismissKeyboard")));

    setTintColor(doneButton, createColor(ColorRed));

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){flexSpace, doneButton},
                                     2, &retainedArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)(data->toolbar,
                                                    sel_getUid("setItems:animated:"), array, false);
    toggleInteraction(data->toolbar, true);

    id view = getView(self);
    addSubview(view, data->scrollView);
    id guide = getLayoutGuide(view);
    pin(data->scrollView, guide, (Padding){0}, 0);
    addVStackToScrollView(data->vStack, data->scrollView);

    CFRelease(array);
    releaseObj(flexSpace);
    releaseObj(doneButton);

    id center = getDeviceNotificationCenter();
    SEL sig = sel_getUid("addObserver:selector:name:object:");
    (((void(*)(id,SEL,id,SEL,id,id))objc_msgSend)
     (center, sig, self, sel_getUid("keyboardShown:"), UIKeyboardDidShowNotification, nil));
    (((void(*)(id,SEL,id,SEL,id,id))objc_msgSend)
     (center, sig, self, sel_getUid("keyboardWillHide:"), UIKeyboardWillHideNotification, nil));
}

void inputVC_viewDidAppear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    InputVC *data = (InputVC *) ((char *)self + VCSize);
    if (!data->scrollHeight) {
        CGRect bounds;
        HAInsets insets;
        getRect(data->scrollView, &bounds, 1);
        data->scrollHeight = (int) bounds.size.height;
#if defined(__arm64__)
        insets = ((HAInsets(*)(id,SEL))objc_msgSend)(data->scrollView, sel_getUid("contentInset"));
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
    int tag = d->activeField ? getTag(d->activeField) : 255;
    if (tag >= d->count - 1) {
        id view = getView(self);
        setBool(view, sel_getUid("endEditing:"), true);
    } else {
        InputView *next = (InputView *) ((char *)d->children[tag + 1] + ViewSize);
        getBool(next->field, sel_getUid("becomeFirstResponder"));
    }
}

void inputVC_keyboardShown(id self, SEL _cmd _U_, id notif) {
    InputVC *data = (InputVC *) ((char *)self + VCSize);
    toggleScrolling(data->scrollView, true);
    id userInfo = getObject(notif, sel_getUid("userInfo"));
    id info = getObjectWithObject(userInfo,
                                  sel_getUid("objectForKey:"), UIKeyboardFrameEndUserInfoKey);

    CGRect kbRect, viewRect, fieldRect, fieldInView;
    id view = getView(self);
    getRect(data->activeField, &fieldRect, 1);
    getRect(view, &viewRect, 0);

#if defined(__arm64__)
    kbRect = ((CGRect(*)(id,SEL))objc_msgSend)(info, sel_getUid("CGRectValue"));
    fieldInView = (((CGRect(*)(id,SEL,CGRect,id))objc_msgSend)
                   (view, sel_getUid("convertRect:fromView:"), fieldRect, data->activeField));
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&kbRect, info, sel_getUid("CGRectValue"));
    (((void(*)(CGRect*,id,SEL,CGRect,id))objc_msgSend_stret)
     (&fieldInView, view, sel_getUid("convertRect:fromView:"), fieldRect, data->activeField));
#endif

    viewRect.size.height -= kbRect.size.height;
    setScrollInsets(data->scrollView, ((HAInsets){data->topOffset, 0, kbRect.size.height, 0}));

    CGPoint upperY = fieldInView.origin;
    upperY.y += fieldInView.size.height;
    if (!(CGRectContainsPoint(viewRect, fieldInView.origin) &&
          CGRectContainsPoint(viewRect, upperY))) {
        (((void(*)(id,SEL,CGRect,bool))objc_msgSend)
         (data->scrollView, sel_getUid("scrollRectToVisible:animated:"), fieldRect, true));
    }
}

void inputVC_keyboardWillHide(id self, SEL _cmd _U_, id notif _U_) {
    InputVC *data = (InputVC *) ((char *)self + VCSize);
    setScrollInsets(data->scrollView, ((HAInsets){data->topOffset, 0, data->bottomOffset, 0}));
    CGRect bounds;
    getRect(data->vStack, &bounds, 1);
    toggleScrolling(data->scrollView, (int) bounds.size.height >= data->scrollHeight);
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

    int i = getTag(field);
    InputView *child = (InputView *) ((char *)d->children[i] + ViewSize);

    CFStringRef text = getText(field);
    if (range.location + range.length > CFStringGetLength(text)) return false;
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text);
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        showInputError(d, child);
        return true;
    }

    short newVal = (short) CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < child->minVal || newVal > child->maxVal) {
        showInputError(d, child);
        return true;
    }

    inputView_reset(child, newVal);
    for (i = 0; i < d->count; ++i) {
        child = (InputView *) ((char *)d->children[i] + ViewSize);
        if (!child->valid) return true;
    }

    enableButton(d->button, true);
    return true;
}
