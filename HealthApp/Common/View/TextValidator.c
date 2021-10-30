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

void initValidatorStrings(void) {
    inputFieldError = localize(CFSTR("inputFieldError"));
}

void validator_setup(Validator *this, short margins, bool createSet, id target, SEL doneSelector) {
    if (createSet) {
        unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        this->set = uset_new_fromArray(char, nums, 10);
    }
    memcpy(&this->padding, &(Padding){4, margins, 4, margins}, sizeof(Padding));

    CGRect bounds;
    id screen = staticMethod(objc_getClass("UIScreen"), sel_getUid("mainScreen"));
#if defined(__arm64__)
    bounds = ((CGRect(*)(id,SEL))objc_msgSend)(screen, sel_getUid("bounds"));
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&bounds, screen, sel_getUid("bounds"));
#endif
    CGFloat width = bounds.size.width;

    this->toolbar = createObjectWithFrame("UIToolbar", (CGRect){{0}, {width, 50}});
    voidFunc(this->toolbar, sel_getUid("sizeToFit"));

    const char *btnName = "UIBarButtonItem";
    this->flexSpace = ((id(*)(id,SEL,int,id,SEL))objc_msgSend)
    (allocClass(btnName), sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    this->doneButton = ((id(*)(id,SEL,CFStringRef,int,id,SEL))objc_msgSend)
    (allocClass(btnName),
     sel_getUid("initWithTitle:style:target:action:"), CFSTR("Done"), 0, target, doneSelector);

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){this->flexSpace, this->doneButton},
                                     2, &kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)(this->toolbar,
                                                    sel_getUid("setItems:animated:"), array, false);
    enableInteraction(this->toolbar, true);
    CFRelease(array);
}

void validator_free(Validator *this) {
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
    releaseObj(this->flexSpace);
    releaseObj(this->doneButton);
}

id validator_add(Validator *v, id delegate, CFStringRef hint, short min, short max) {
    struct InputView *child = &v->children[v->count];
    child->view = createView(nil, false, -1, -1);
    child->minVal = min;
    child->maxVal = max;
    child->hintText = hint;
    child->errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    child->hintLabel = createLabel(hint, TextFootnote, 4, false);
    child->field = createTextfield(delegate, NULL, hint, 4, 4, v->count++);
    child->errorLabel = createLabel(child->errorText, TextFootnote, 4, false);
    setTextColor(child->errorLabel, createColor("systemRedColor"));
    id vStack = createStackView((id []){child->hintLabel, child->field, child->errorLabel},
                                3, 1, 4, v->padding);
    addSubview(child->view, vStack);
    pin(vStack, child->view, (Padding){0}, 0);
    releaseObj(vStack);
    hideView(child->errorLabel, true);
    setObject(child->field, sel_getUid("setInputAccessoryView:"), v->toolbar);
    return child->view;
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
