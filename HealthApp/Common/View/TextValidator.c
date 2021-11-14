//
//  TextValidator.c
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#include "TextValidator.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include "CocoaHelpers.h"
#include "Views.h"

extern id UIKeyboardDidShowNotification;
extern id UIKeyboardWillHideNotification;
extern id UIKeyboardFrameEndUserInfoKey;
extern void setScrollViewInsets(id v, Padding margins);

static CFStringRef inputFieldError;

gen_uset_source(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal,
                DSDefault_shallowCopy, DSDefault_shallowDelete)

static inline void showInputError(Validator *validator, struct InputView *child) {
    enableButton(validator->button, false);
    child->valid = false;
    hideView(child->errorLabel, false);
    CFStringRef text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@. %@"),
                                                child->hintText, child->errorText);
    setAccessibilityLabel(child->field, text);
    CFRelease(text);
}

static inline void addNotifObserver(id target, SEL action, id name) {
    ((void(*)(id,SEL,id,SEL,id,id))objc_msgSend)
    (getDeviceNotificationCenter(), sel_getUid("addObserver:selector:name:object:"),
     target, action, name, nil);
}

static inline void removeNotifObserver(id target, id name) {
    ((void(*)(id,SEL,id,id,id))objc_msgSend)
    (getDeviceNotificationCenter(), sel_getUid("removeObserver:name:object:"), target, name, nil);
}

static inline void toggleScrolling(id view, bool enable) {
    setBool(view, sel_getUid("setScrollEnabled:"), enable);
}

void initValidatorStrings(void) {
    inputFieldError = localize(CFSTR("inputFieldError"));
}

void validator_setup(Validator *this, short margins, bool createSet, id target) {
    if (createSet) {
        unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        this->set = uset_new_fromArray(char, nums, 10);
    }
    memcpy(&this->padding, &(Padding){4, margins, 4, margins}, sizeof(Padding));

    addNotifObserver(target, sel_getUid("keyboardShown:"), UIKeyboardDidShowNotification);
    addNotifObserver(target, sel_getUid("keyboardWillHide:"), UIKeyboardWillHideNotification);

    CGRect bounds;
    getScreenBounds(&bounds);
    CGFloat width = bounds.size.width;

    this->toolbar = createObjectWithFrame("UIToolbar", (CGRect){{0}, {width, 50}});
    voidFunc(this->toolbar, sel_getUid("sizeToFit"));

    const char *btnName = "UIBarButtonItem";
    id flexSpace = ((id(*)(id,SEL,int,id,SEL))objc_msgSend)
    (allocClass(btnName), sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = ((id(*)(id,SEL,CFStringRef,int,id,SEL))objc_msgSend)
    (allocClass(btnName), sel_getUid("initWithTitle:style:target:action:"),
     CFSTR("Done"), 0, target, sel_getUid("dismissKeyboard"));

    if (osVersion < Version14)
        setTintColor(doneButton, createColor(ColorRed));

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){flexSpace, doneButton},
                                     2, &retainedArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)(this->toolbar,
                                                    sel_getUid("setItems:animated:"), array, false);
    enableInteraction(this->toolbar, true);
    CFRelease(array);
    releaseObj(flexSpace);
    releaseObj(doneButton);
}

void validator_free(Validator *this, id target) {
    if (this->set)
        uset_free(char, this->set);
    for (int i = 0; i < 4; ++i) {
        if (this->children[i].view) {
            releaseObj(this->children[i].hintLabel);
            releaseObj(this->children[i].field);
            releaseObj(this->children[i].errorLabel);
            releaseObj(this->children[i].view);
            CFRelease(this->children[i].errorText);
        }
    }
    releaseObj(this->toolbar);
    releaseObj(this->scrollView);
    releaseObj(this->vStack);
    removeNotifObserver(target, UIKeyboardDidShowNotification);
    removeNotifObserver(target, UIKeyboardWillHideNotification);
}

id validator_add(Validator *v, id delegate, CFStringRef hint, short min, short max) {
    struct InputView *child = &v->children[v->count];
    child->view = createView(false, -1, -1);
    child->minVal = min;
    child->maxVal = max;
    child->hintText = hint;
    child->errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    child->hintLabel = createLabel(hint, TextFootnote, 4, false);
    child->field = createTextfield(delegate, NULL, hint, 4, 4, v->count++);
    child->errorLabel = createLabel(child->errorText, TextFootnote, 4, false);
    DMLabel *errorPtr = (DMLabel *) child->errorLabel;
    errorPtr->colorCode = ColorRed;
    setTextColor(child->errorLabel, createColor(ColorRed));
    id vStack = createStackView((id []){child->hintLabel, child->field, child->errorLabel},
                                3, 1, 4, v->padding);
    addSubview(child->view, vStack);
    pin(vStack, child->view, (Padding){0}, 0);
    releaseObj(vStack);
    hideView(child->errorLabel, true);
    setObject(child->field, sel_getUid("setInputAccessoryView:"), v->toolbar);
    return child->view;
}

void validator_getScrollHeight(Validator *this) {
    CGRect bounds;
    getRect(this->scrollView, &bounds, 1);
    this->scrollHeight = bounds.size.height;
}

void validator_handleKeyboardShow(Validator *this, id view, id notif) {
    toggleScrolling(this->scrollView, true);
    id info = getObjectWithObject(getObject(notif, sel_getUid("userInfo")),
                                  sel_getUid("objectForKey:"), UIKeyboardFrameEndUserInfoKey);
    CGRect kbRect, viewRect, fieldRect;
#if defined(__arm64__)
    kbRect = ((CGRect(*)(id,SEL))objc_msgSend)(info, sel_getUid("CGRectValue"));
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&kbRect, info, sel_getUid("CGRectValue"));
#endif
    setScrollViewInsets(this->scrollView, (Padding){0, 0, kbRect.size.height, 0});

    getRect(this->activeField, &fieldRect, 0);
    getRect(view, &viewRect, 0);
    viewRect.size.height -= kbRect.size.height;
    if (!CGRectContainsPoint(viewRect, fieldRect.origin)) {
        ((void(*)(id,SEL,CGRect,bool))objc_msgSend)
        (this->scrollView, sel_getUid("scrollRectToVisible:animated:"), fieldRect, true);
    }
}

void validator_handleKeyboardHide(Validator *this) {
    setScrollViewInsets(this->scrollView, (Padding){0});
    CGRect bounds;
    getRect(this->vStack, &bounds, 1);
    toggleScrolling(this->scrollView, (int) bounds.size.height >= this->scrollHeight);
}

void inputView_reset(struct InputView *this, short value) {
    this->valid = true;
    this->result = value;
    hideView(this->errorLabel, true);
    setAccessibilityLabel(this->field, this->hintText);
}

bool checkInput(Validator *this, id field, CFRange range, CFStringRef replacement) {
    int len = (int) CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            if (!uset_contains(char, this->set, CFStringGetCharacterFromInlineBuffer(&buf, i)))
                return false;
        }
    }

    int i = getInt(field, sel_getUid("tag"));
    if (i == this->count) return true;
    struct InputView *child = &this->children[i];

    CFStringRef text = ((CFStringRef(*)(id,SEL))objc_msgSend)(field, sel_getUid("text"));
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text ? text : CFSTR(""));
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        showInputError(this, child);
        return true;
    }

    short newVal = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < child->minVal || newVal > child->maxVal) {
        showInputError(this, child);
        return true;
    }

    inputView_reset(child, newVal);

    for (i = 0; i < this->count; ++i) {
        if (!this->children[i].valid) return true;
    }

    enableButton(this->button, true);
    return true;
}
