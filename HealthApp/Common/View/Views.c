//
//  Views.c
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#include "Views.h"
#include <objc/message.h>
#include "CocoaHelpers.h"

extern void setLayoutMargins(id v, Padding margins);

static inline void disableAutoresizing(id view) {
    setBool(view, sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), false);
}

static inline void setAlignment(id view, int alignment) {
    setInt(view, sel_getUid("setTextAlignment:"), alignment);
}

static inline void adjustFontForSizeCategory(id view) {
    setBool(view, sel_getUid("setAdjustsFontForContentSizeCategory:"), true);
}

static inline void setDynamicFont(id view) {
    setBool(view, sel_getUid("setAdjustsFontSizeToFitWidth:"), true);
    adjustFontForSizeCategory(view);
    setCGFloat(view, sel_getUid("setMinimumScaleFactor:"), 0.85);
}

static inline void setCornerRadius(id view) {
    setCGFloat(getLayer(view), sel_getUid("setCornerRadius:"), 5);
}

static inline void addTarget(id view, id target, SEL action, int event) {
    ((void(*)(id,SEL,id,SEL,int))objc_msgSend)
    (view, sel_getUid("addTarget:action:forControlEvents:"), target, action, event);
}

static inline void setLabelFont(id view, int style) {
    setObject(view, sel_getUid("setFont:"), createFont(style));
}

#pragma mark - Simple getters/setters

void addSubview(id view, id subview) {
    setObject(view, sel_getUid("addSubview:"), subview);
}

void removeView(id v) {
    voidFunc(v, sel_getUid("removeFromSuperview"));
}

id getLayer(id view) {
    return getObject(view, sel_getUid("layer"));
}

void setTag(id view, int tag) {
    setInt(view, sel_getUid("setTag:"), tag);
}

void hideView(id view, bool hide) {
    setBool(view, sel_getUid("setHidden:"), hide);
}

void setBackground(id view, id color) {
    setObject(view, sel_getUid("setBackgroundColor:"), color);
}

void setAccessibilityLabel(id view, CFStringRef text) {
    setString(view, sel_getUid("setAccessibilityLabel:"), text);
}

void setLabelText(id view, CFStringRef text) {
    setString(view, sel_getUid("setText:"), text);
}

void setTextColor(id view, id color) {
    setObject(view, sel_getUid("setTextColor:"), color);
}

void enableButton(id view, bool enabled) {
    setBool(view, sel_getUid("setEnabled:"), enabled);
}

void enableInteraction(id view, bool enabled) {
    setBool(view, sel_getUid("setUserInteractionEnabled:"), enabled);
}

void setButtonTitle(id view, CFStringRef title, int state) {
    ((void(*)(id,SEL,CFStringRef,int))objc_msgSend)(view, sel_getUid("setTitle:forState:"),
                                                    title, state);
}

void setButtonColor(id view, id color, int state) {
    ((void(*)(id,SEL,id,int))objc_msgSend)(view, sel_getUid("setTitleColor:forState:"),
                                           color, state);
}

#pragma mark - View initializers

id createObjectWithFrame(const char *name, CGRect rect) {
    return ((id(*)(id,SEL,CGRect))objc_msgSend)(allocClass(name),
                                                sel_getUid("initWithFrame:"), rect);
}

id createView(id color, bool rounded, int width, int height) {
    id view = createObjectWithFrame("UIView", CGRectZero);
    disableAutoresizing(view);
    setBackground(view, color);
    if (rounded)
        setCornerRadius(view);
    if (width >= 0)
        setWidth(view, width);
    if (height >= 0)
        setHeight(view, height);
    return view;
}

id createStackView(id *subviews, int count, int axis, int spacing, Padding margins) {
    id view;
    const char *name = "UIStackView";
    if (count) {
        CFArrayRef array = CFArrayCreate(NULL, (const void **)subviews, count, &kCocoaArrCallbacks);
        view = getObjectWithArr(allocClass(name), sel_getUid("initWithArrangedSubviews:"), array);
        CFRelease(array);
    } else {
        view = createObjectWithFrame(name, CGRectZero);
    }
    disableAutoresizing(view);
    setInt(view, sel_getUid("setAxis:"), axis);
    setCGFloat(view, sel_getUid("setSpacing:"), spacing);
    setBool(view, sel_getUid("setLayoutMarginsRelativeArrangement:"), true);
    setLayoutMargins(view, margins);
    return view;
}

id createScrollView(void) {
    id view = createObjectWithFrame("UIScrollView", CGRectZero);
    disableAutoresizing(view);
    setInt(view, sel_getUid("setAutoresizingMask:"), 16);
    setBool(view, sel_getUid("setBounces:"), true);
    setBool(view, sel_getUid("setShowsVerticalScrollIndicator:"), true);
    return view;
}

id createLabel(CFStringRef text, int style, int alignment, bool accessible) {
    id view = createObjectWithFrame("UILabel", CGRectZero);
    disableAutoresizing(view);
    setLabelText(view, text);
    setLabelFont(view, style);
    setDynamicFont(view);
    setTextColor(view, createColor("labelColor"));
    setAlignment(view, alignment);
    setBool(view, sel_getUid("setIsAccessibilityElement:"), accessible);
    return view;
}

id createButton(CFStringRef title, id color, int params,
                int tag, id target, SEL action, int height) {
    id view = ((id(*)(Class,SEL,int))objc_msgSend)(objc_getClass("UIButton"),
                                                   sel_getUid("buttonWithType:"), 1);
    disableAutoresizing(view);
    setButtonTitle(view, title, 0);
    setButtonColor(view, color, 0);
    setButtonColor(view, createColor("secondaryLabelColor"), 2);
    id label = getObject(view, sel_getUid("titleLabel"));
    setLabelFont(label, (params & BtnLargeFont) ? TextHead : TextBody);
    setDynamicFont(label);
    if (params & BtnBackground)
        setBackground(view, createColor("secondarySystemGroupedBackgroundColor"));
    if (params & BtnRounded)
        setCornerRadius(view);
    setTag(view, tag);
    addTarget(view, target, action, 64);
    if (height >= 0)
        setHeight(view, height);
    return view;
}

id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height) {
    CFStringRef segments[count];
    fillStringArray(segments, format, count);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, count, &kCocoaArrCallbacks);
    id view = getObjectWithArr(allocClass("UISegmentedControl"),
                               sel_getUid("initWithItems:"), array);
    disableAutoresizing(view);
    setInt(view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    if (action)
        addTarget(view, target, action, 4096);
    if (height >= 0)
        setHeight(view, height);
    CFRelease(array);
    return view;
}

id createTextfield(id delegate, CFStringRef text, CFStringRef hint,
                   int alignment, int keyboard, int tag) {
    id view = createObjectWithFrame("UITextField", CGRectZero);
    disableAutoresizing(view);
    setBackground(view, createColor("tertiarySystemBackgroundColor"));
    setLabelText(view, text);
    setLabelFont(view, TextBody);
    adjustFontForSizeCategory(view);
    setAlignment(view, alignment);
    setTag(view, tag);
    setInt(view, sel_getUid("setBorderStyle:"), 3);
    setInt(view, sel_getUid("setKeyboardType:"), keyboard);
    setObject(view, sel_getUid("setDelegate:"), delegate);
    setMinHeight(view, 44);
    setAccessibilityLabel(view, hint);
    return view;
}
