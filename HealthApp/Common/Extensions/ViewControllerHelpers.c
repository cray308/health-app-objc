//
//  ViewControllerHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "ViewControllerHelpers.h"
#include <CoreGraphics/CoreGraphics.h>
#include "SwiftBridging.h"

gen_uset_source(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal,
                DSDefault_shallowCopy, DSDefault_shallowDelete)

struct AnchorNames anchors = {
    "topAnchor",
    "bottomAnchor",
    "leadingAnchor",
    "trailingAnchor",
    "widthAnchor",
    "heightAnchor"
};

static void disableAutoresizing(id view) {
    ((void(*)(id,SEL,bool))objc_msgSend)
    (view, sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), false);
}

static void setAlignment(id view, int alignment) {
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setTextAlignment:"), alignment);
}

static void setDynamicFont(id view) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setAdjustsFontSizeToFitWidth:"), true);
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(view, sel_getUid("setMinimumScaleFactor:"), 0.85);
}

static void setCornerRadius(id view) {
    id layer = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("layer"));
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(layer, sel_getUid("setCornerRadius:"), 5);
}

static void setFont(id view, id font) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setFont:"), font);
}

static void addTarget(id view, id target, SEL action, int event) {
    ((void(*)(id,SEL,id,SEL,int))objc_msgSend)
    (view, sel_getUid("addTarget:action:forControlEvents:"), target, action, event);
}

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
    CFArrayRef array = CFArrayCreate(NULL, (const void **)buttons, 2, &kCocoaArrCallbacks);
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
    id constraint = createConstraint(getAnchor(view, anchors.height), nil, 1);
    activateConstraints((id[]){constraint}, 1);
    return view;
}

USet_char *createNumberCharacterSet(void) {
    unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    return uset_new_fromArray(char, nums, 10);
}

bool checkTextfield(id field, CFRange range, CFStringRef replacement, USet_char *set, id button,
                    id *fields, int count, short *maxes, short *results, bool *valid) {
    int len = (int) CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            if (!uset_contains(char, set, CFStringGetCharacterFromInlineBuffer(&buf, i)))
                return false;
        }
    }

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

#pragma mark - VC Functions

id createVCWithDelegate(const char *name, void *delegate) {
    return ((id(*)(id,SEL,void*))objc_msgSend)(allocClass(name),
                                               sel_getUid("initWithDelegate:"), delegate);
}

void setupNavVC(id navVC, id firstVC) {
    id ctrls[] = {firstVC};
    CFArrayRef array = CFArrayCreate(NULL, (const void **)ctrls, 1, &kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
    CFRelease(array);
}

id getFirstVC(id navVC) {
    CFArrayRef ctrls = ((CFArrayRef(*)(id,SEL))objc_msgSend)(navVC, sel_getUid("viewControllers"));
    return (id) CFArrayGetValueAtIndex(ctrls, 0);
}

id allocNavVC(void) {
    return allocClass("UINavigationController");
}

void presentVC(id presenter, id child) {
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id presenter, id modal) {
    id container = ((id(*)(id,SEL,id))objc_msgSend)
    (allocNavVC(), sel_getUid("initWithRootViewController:"), modal);
    presentVC(presenter, container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter) {
    ((void(*)(id,SEL,bool,id))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, nil);
}

id getRootView(id vc) {
    return ((id(*)(id,SEL))objc_msgSend)(vc, sel_getUid("view"));
}

void addSubview(id view, id subview) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("addSubview:"), subview);
}

id createAlertController(CFStringRef title, CFStringRef message) {
    return ((id(*)(Class,SEL,CFStringRef,CFStringRef,int))objc_msgSend)
    (objc_getClass("UIAlertController"),
     sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

id createAlertAction(CFStringRef title, int style, AlertCallback handler) {
    return ((id(*)(Class,SEL,CFStringRef,int,void(^)(id)))objc_msgSend)
    (objc_getClass("UIAlertAction"), sel_getUid("actionWithTitle:style:handler:"), title, style,
     ^(id action _U_) {
        if (handler)
            handler();
    });
}

void addAlertAction(id ctrl, id action) {
    ((void(*)(id,SEL,id))objc_msgSend)(ctrl, sel_getUid("addAction:"), action);
}

#pragma mark - View Functions

id getAnchor(id view, const char *name) {
    return ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid(name));
}

id createConstraint(id a1, id a2, CGFloat constant) {
    id result;
    if (a2) {
        result = ((id(*)(id,SEL,id,CGFloat))objc_msgSend)
        (a1, sel_getUid("constraintEqualToAnchor:constant:"), a2, constant);
    } else {
        result = ((id(*)(id,SEL,CGFloat))objc_msgSend)(a1, sel_getUid("constraintEqualToConstant:"),
                                                       constant);
    }
    return result;
}

id createObjectWithFrame(const char *name, CGRect rect) {
    return ((id(*)(id,SEL,CGRect))objc_msgSend)(allocClass(name),
                                                sel_getUid("initWithFrame:"), rect);
}

id createView(id color, bool rounded) {
    id view = createObjectWithFrame("UIView", CGRectZero);
    disableAutoresizing(view);
    setBackground(view, color);
    if (rounded)
        setCornerRadius(view);
    return view;
}

id createStackView(id *subviews, int count, int axis, CGFloat spacing,
                   int distribution, HAEdgeInsets margins) {
    id view;
    const char *name = "UIStackView";
    if (count) {
        CFArrayRef array = CFArrayCreate(NULL, (const void **)subviews, count, &kCocoaArrCallbacks);
        view = ((id(*)(id,SEL,CFArrayRef))objc_msgSend)
        (allocClass(name), sel_getUid("initWithArrangedSubviews:"), array);
        CFRelease(array);
    } else {
        view = createObjectWithFrame(name, CGRectZero);
    }
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setAxis:"), axis);
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(view, sel_getUid("setSpacing:"), spacing);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setDistribution:"), distribution);
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setLayoutMarginsRelativeArrangement:"),
                                         true);
    setLayoutMargins(view, &margins);
    return view;
}

id createScrollView(void) {
    id view = createObjectWithFrame("UIScrollView", CGRectZero);
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setAutoresizingMask:"), 16);
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setBounces:"), true);
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setShowsVerticalScrollIndicator:"),
                                         true);
    return view;
}

id createLabel(CFStringRef text, id style, int alignment) {
    id view = createObjectWithFrame("UILabel", CGRectZero);
    disableAutoresizing(view);
    setLabelText(view, text);
    setLabelFontWithStyle(view, style);
    setDynamicFont(view);
    ((void(*)(id,SEL,id))objc_msgSend)(view,
                                       sel_getUid("setTextColor:"), createColor("labelColor"));
    setAlignment(view, alignment);
    return view;
}

id createButton(CFStringRef title, id color, id disabledColor, id style,
                id background, bool rounded, bool enabled, int tag, id target, SEL action) {
    id view = ((id(*)(Class,SEL,int))objc_msgSend)(objc_getClass("UIButton"),
                                                   sel_getUid("buttonWithType:"), 1);
    disableAutoresizing(view);
    setButtonTitle(view, title, 0);
    setButtonColor(view, color, 0);
    if (disabledColor)
        setButtonColor(view, disabledColor, 2);
    id label = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("titleLabel"));
    setLabelFontWithStyle(label, style);
    setDynamicFont(label);
    setBackground(view, background);
    if (rounded)
        setCornerRadius(view);
    setTag(view, tag);
    enableButton(view, enabled);
    addTarget(view, target, action, 64);
    return view;
}

id createSegmentedControl(CFStringRef *items, int count, int startIndex, id target, SEL action) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)items, count, &kCocoaArrCallbacks);
    id view = ((id(*)(id,SEL,CFArrayRef))objc_msgSend)(allocClass("UISegmentedControl"),
                                                       sel_getUid("initWithItems:"), array);
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    setCornerRadius(view);
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setTintColor:"),
                                       createColor("systemGray2Color"));
    if (action)
        addTarget(view, target, action, 4096);
    CFRelease(array);
    return view;
}

id createTextfield(id delegate, CFStringRef text, CFStringRef placeholder,
                   int alignment, int keyboard) {
    id view = createObjectWithFrame("UITextField", CGRectZero);
    disableAutoresizing(view);
    setBackground(view, createColor("tertiarySystemBackgroundColor"));
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(view, sel_getUid("setPlaceholder:"), placeholder);
    setLabelText(view, text);
    setAlignment(view, alignment);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setBorderStyle:"), 3);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setKeyboardType:"), keyboard);
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setDelegate:"), delegate);
    return view;
}

void enableButton(id view, bool enabled) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setEnabled:"), enabled);
}

void activateConstraints(id *constraints, int count) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)constraints, count, &kCocoaArrCallbacks);
    ((void(*)(Class,SEL,CFArrayRef))objc_msgSend)(objc_getClass("NSLayoutConstraint"),
                                                  sel_getUid("activateConstraints:"), array);
    CFRelease(array);
}

void setTag(id view, int tag) {
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setTag:"), tag);
}

void setBackground(id view, id color) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setBackgroundColor:"), color);
}

void setLabelText(id view, CFStringRef text) {
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(view, sel_getUid("setText:"), text);
}

void setLabelFontWithStyle(id view, id style) {
    if (style) {
        id font = ((id(*)(Class,SEL,CFStringRef))objc_msgSend)
        (objc_getClass("UIFont"), sel_getUid("preferredFontForTextStyle:"), (CFStringRef)style);
        setFont(view, font);
    }
}

void setLabelFontWithSize(id view, CGFloat size) {
    id font = ((id(*)(Class,SEL,CGFloat))objc_msgSend)
    (objc_getClass("UIFont"), sel_getUid("systemFontOfSize:"), size);
    setFont(view, font);
}

void setButtonTitle(id view, CFStringRef title, int state) {
    ((void(*)(id,SEL,CFStringRef,int))objc_msgSend)(view, sel_getUid("setTitle:forState:"),
                                                    title, state);
}

void setButtonColor(id view, id color, int state) {
    ((void(*)(id,SEL,id,int))objc_msgSend)(view, sel_getUid("setTitleColor:forState:"),
                                           color, state);
}
