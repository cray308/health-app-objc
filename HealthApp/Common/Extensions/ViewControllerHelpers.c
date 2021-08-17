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
    ((void (*)(CGRect*, id, SEL)) objc_msgSend_stret)
    (&bounds,
     objc_staticMethod(objc_getClass("UIScreen"), sel_getUid("mainScreen")), sel_getUid("bounds"));
    CGFloat width = bounds.size.width;

    id toolbar = ((id (*)(id, SEL, CGRect)) objc_msgSend)
    (objc_staticMethod(objc_getClass("UIToolbar"), sel_getUid("alloc")),
     sel_getUid("initWithFrame:"), (CGRect){{0}, {width, 50}});
    objc_singleArg(toolbar, sel_getUid("sizeToFit"));

    id flexSpace = ((id (*)(id, SEL, int, id, SEL)) objc_msgSend)
    (objc_staticMethod(objc_getClass("UIBarButtonItem"), sel_getUid("alloc")),
     sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);

    id doneButton = ((id (*)(id, SEL, CFStringRef, int, id, SEL)) objc_msgSend)
    (objc_staticMethod(objc_getClass("UIBarButtonItem"), sel_getUid("alloc")),
     sel_getUid("initWithTitle:style:target:action:"), CFSTR("Done"), 0, target, doneSelector);

    id array = ((id (*)(Class, SEL, id, ...)) objc_msgSend)
    (objc_getClass("NSArray"), sel_getUid("arrayWithObjects:"), flexSpace, doneButton, nil);
    ((void (*)(id, SEL, id, bool)) objc_msgSend)(toolbar,
                                                 sel_getUid("setItems:animated:"), array, false);
    ((void (*)(id, SEL, bool)) objc_msgSend)(toolbar,
                                             sel_getUid("setUserInteractionEnabled:"), true);

    for (int i = 0; fields[i]; ++i) {
        ((void (*)(id, SEL, id)) objc_msgSend)(fields[i],
                                               sel_getUid("setInputAccessoryView:"), toolbar);
    }

    objc_singleArg(toolbar, sel_getUid("release"));
    objc_singleArg(flexSpace, sel_getUid("release"));
    objc_singleArg(doneButton, sel_getUid("release"));
}

id createDivider(void) {
    id view = ((id (*)(id, SEL, CGRect)) objc_msgSend)
    (objc_staticMethod(objc_getClass("UIView"), sel_getUid("alloc")),
     sel_getUid("initWithFrame:"), CGRectZero);
    ((void (*)(id, SEL, bool)) objc_msgSend)
    (view, sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), false);
    ((void (*)(id, SEL, id)) objc_msgSend)
    (view, sel_getUid("setBackgroundColor:"),
     objc_staticMethod(objc_getClass("UIColor"), sel_getUid("separatorColor")));

    id heightAnchor = ((id (*)(id, SEL)) objc_msgSend)(view, sel_getUid("heightAnchor"));
    id constraint = ((id (*)(id, SEL, CGFloat)) objc_msgSend)
    (heightAnchor, sel_getUid("constraintEqualToConstant:"), 1);
    ((void (*)(id, SEL, bool)) objc_msgSend)(constraint, sel_getUid("setActive:"), true);
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
