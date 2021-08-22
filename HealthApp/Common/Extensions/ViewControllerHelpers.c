//
//  ViewControllerHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "ViewControllerHelpers.h"
#include <CoreGraphics/CoreGraphics.h>

gen_uset_source(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal,
                DSDefault_shallowCopy, DSDefault_shallowDelete)

void createToolbar(id target, SEL doneSelector, id *fields) {
    CGRect bounds;
    getScreenBounds(&bounds);
    CGFloat width = bounds.size.width;

    id toolbar = createObjectWithFrame("UIToolbar", (CGRect){{0}, {width, 50}});
    objc_singleArg(toolbar, sel_getUid("sizeToFit"));

    const char *btnName = "UIBarButtonItem";
    id flexSpace = ((id(*)(id,SEL,int,id,SEL))objc_msgSend)
    (allocClass(btnName), sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = ((id(*)(id,SEL,CFStringRef,int,id,SEL))objc_msgSend)
    (allocClass(btnName),
     sel_getUid("initWithTitle:style:target:action:"), CFSTR("Done"), 0, target, doneSelector);

    id buttons[] = {flexSpace, doneButton};
    CFArrayRef array = CFArrayCreate(NULL, (const void **)buttons, 2, kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)(toolbar,
                                            sel_getUid("setItems:animated:"), array, false);
    ((void(*)(id,SEL,bool))objc_msgSend)(toolbar, sel_getUid("setUserInteractionEnabled:"), true);

    for (int i = 0; fields[i]; ++i) {
        ((void(*)(id,SEL,id))objc_msgSend)(fields[i],
                                           sel_getUid("setInputAccessoryView:"), toolbar);
    }

    CFRelease(array);
    releaseObj(toolbar);
    releaseObj(flexSpace);
    releaseObj(doneButton);
}

void setNavButton(id navItem, bool left, id button, CGFloat totalWidth) {
    ((void(*)(id,SEL,CGRect))objc_msgSend)(button, sel_getUid("setFrame:"),
                                           (CGRect){{0}, {totalWidth / 3, 30}});
    id item = ((id(*)(id,SEL,id))objc_msgSend)(allocClass("UIBarButtonItem"),
                                               sel_getUid("initWithCustomView:"), button);
    if (left)
        ((void(*)(id,SEL,id))objc_msgSend)(navItem, sel_getUid("setLeftBarButtonItem:"), item);
    else
        ((void(*)(id,SEL,id))objc_msgSend)(navItem, sel_getUid("setRightBarButtonItem:"), item);
    releaseObj(item);
}

id createDivider(void) {
    id view = createView(createColor("separatorColor"), false);
    id heightAnchor = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("heightAnchor"));
    id constraint = ((id(*)(id,SEL,CGFloat))objc_msgSend)
    (heightAnchor, sel_getUid("constraintEqualToConstant:"), 1);
    activateConstraints((id[]){constraint}, 1);
    return view;
}

USet_char *createNumberCharacterSet(void) {
    unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    return uset_new_fromArray(char, nums, 10);
}

bool validateNumericInput(USet_char *set, CFStringRef str, CFStringInlineBuffer *buf) {
    int len = (int) CFStringGetLength(str);
    if (!len) return true;

    CFStringInitInlineBuffer(str, buf, CFRangeMake(0, len));
    for (int i = 0; i < len; ++i) {
        if (!uset_contains(char, set, CFStringGetCharacterFromInlineBuffer(buf, i))) return false;
    }
    return true;
}

bool checkTextfield(id field, CFRange range, CFStringRef replacement, USet_char *set, id button,
                    id *fields, int count, short *maxes, short *results, bool *valid) {
    CFStringInlineBuffer buf;
    if (!validateNumericInput(set, replacement, &buf)) return false;

    int i = 0;
    for (; i < count; ++i) {
        if (fields[i] && field == fields[i]) break;
    }
    if (i == count) return true;

    CFStringRef text = ((CFStringRef(*)(id,SEL))objc_msgSend)(field, sel_getUid("text"));
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text ? text : CFSTR(""));
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        enableButton(button, false);
        valid[i] = false;
        return true;
    }

    short newVal = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < 0 || newVal > maxes[i]) {
        enableButton(button, false);
        valid[i] = false;
        return true;
    }

    valid[i] = true;
    results[i] = newVal;

    for (i = 0; i < count; ++i) {
        if (!valid[i]) {
            enableButton(button, false);
            return true;
        }
    }
    
    enableButton(button, true);
    return true;
}
