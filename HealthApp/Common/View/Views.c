#include "Views.h"

#define adjustFontForSizeCategory(v)\
 msg1(void,bool,v,sel_getUid("setAdjustsFontForContentSizeCategory:"),true)

#define setControlTextAttribs(v, d, s)\
 msg2(void,CFDictionaryRef,unsigned long,v,sel_getUid("setTitleTextAttributes:forState:"),d,s)

extern CGFloat UIFontWeightMedium;

size_t VCSize;
Class VCClass;
size_t ViewSize;
Class ViewClass;
Class FontClass;
Class ConstraintCls;

static void setDynamicFont(id view) {
    msg1(void, bool, view, sel_getUid("setAdjustsFontSizeToFitWidth:"), true);
    adjustFontForSizeCategory(view);
    msg1(void, CGFloat, view, sel_getUid("setMinimumScaleFactor:"), 0.85);
}

void setHeight(id v, int height, bool optional) {
    id c = createConstraint(v, 8, 0, nil, 0, height);
    if (optional)
        lowerPriority(c);
    activateConstraint(c);
}

void pin(id v, id container) {
    id constraints[] = {
        createConstraint(v, 3, 0, container, 3, 0),
        createConstraint(v, 4, 0, container, 4, 0),
        createConstraint(v, 5, 0, container, 5, 0),
        createConstraint(v, 6, 0, container, 6, 0)
    };
    CFArrayRef array = CFArrayCreate(NULL, (const void **)constraints, 4, NULL);
    activateConstraintsArray(array);
    CFRelease(array);
}

#pragma mark - View initializers

id createStackView(id *subviews, int count, int axis, int spacing, Padding margins) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)subviews, count, NULL);
    id _v = allocClass(objc_getClass("UIStackView"));
    id view = msg1(id, CFArrayRef, _v, sel_getUid("initWithArrangedSubviews:"), arr);
    CFRelease(arr);
    msg1(void, long, view, sel_getUid("setAxis:"), axis);
    msg1(void, CGFloat, view, sel_getUid("setSpacing:"), spacing);
    msg1(void, bool, view, sel_getUid("setLayoutMarginsRelativeArrangement:"), true);
    HAInsets insets = {margins.top, margins.left, margins.bottom, margins.right};
    setMargins(view, insets);
    return view;
}

id createScrollView(void) {
    id view = createNew(objc_getClass("UIScrollView"));
    setUsesAutolayout(view);
    msg1(void, long, view, sel_getUid("setAutoresizingMask:"), 16);
    return view;
}

id createLabel(CFStringRef text, id style, bool accessible) {
    id view = createNew(objc_getClass("UILabel"));
    setLabelText(view, text);
    if (text)
        CFRelease(text);
    setFont(view, getPreferredFont(style));
    setDynamicFont(view);
    setTextColor(view, createColor(ColorLabel));
    msg1(void, bool, view, sel_getUid("setIsAccessibilityElement:"), accessible);
    return view;
}

id createButton(CFStringRef title, int color, int tag, id target, SEL action) {
    id view = clsF1(id, long, objc_getClass("UIButton"), sel_getUid("buttonWithType:"), 1);
    setButtonTitle(view, title, 0);
    if (title)
        CFRelease(title);
    setButtonColor(view, createColor(color), 0);
    setButtonColor(view, createColor(ColorSecondaryLabel), 2);
    id label = getTitleLabel(view);
    setFont(label, getPreferredFont(UIFontTextStyleBody));
    setDynamicFont(label);
    setTag(view, tag);
    addTarget(view, target, action, 64);
    return view;
}

id createSegmentedControl(CFBundleRef bundle, CFStringRef format, int startIndex) {
    CFStringRef segments[3];
    fillStringArray(bundle, segments, format, 3);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, 3, NULL);
    id _obj = allocClass(objc_getClass("UISegmentedControl"));
    id view = msg1(id, CFArrayRef, _obj, sel_getUid("initWithItems:"), array);
    setSelectedSegment(view, startIndex);
    CFRelease(array);
    CFRelease(segments[0]);
    CFRelease(segments[1]);
    CFRelease(segments[2]);
    return view;
}

id createTextfield(id delegate, CFStringRef text, CFStringRef hint,
                   int alignment, int keyboard, int tag) {
    id view = createNew(objc_getClass("UITextField"));
    setBackground(view, createColor(ColorTertiaryBG));
    setLabelText(view, text);
    setFont(view, getPreferredFont(UIFontTextStyleBody));
    setTextColor(view, createColor(ColorLabel));
    adjustFontForSizeCategory(view);
    msg1(void, long, view, sel_getUid("setTextAlignment:"), alignment);
    setTag(view, tag);
    msg1(void, long, view, sel_getUid("setBorderStyle:"), 3);
    msg1(void, long, view, sel_getUid("setKeyboardType:"), keyboard);
    setDelegate(view, delegate);
    activateConstraint(createConstraint(view, 8, 1, nil, 0, 44));
    setAccessibilityLabel(view, hint);
    CFRelease(hint);
    return view;
}

void addVStackToScrollView(id view, id vStack, id scrollView) {
    setUsesAutolayout(vStack);
    addSubview(view, scrollView);
    addSubview(scrollView, vStack);
    pin(scrollView, msg0(id, view, sel_getUid("safeAreaLayoutGuide")));
    pin(vStack, scrollView);
    activateConstraint(createConstraint(vStack, 7, 0, scrollView, 7, 0));
}

void setNavButtons(id vc, id *buttons) {
    Class itemClass = objc_getClass("UIBarButtonItem");
    SEL itemInit = sel_getUid("initWithCustomView:");
    id navItem = getNavItem(vc);
    const char *const setters[] = {"setLeftBarButtonItem:", "setRightBarButtonItem:"};
    for (int i = 0; i < 2; ++i) {
        id btn = buttons[i];
        if (btn) {
            setUsesAutolayout(btn);
            id item = msg1(id, id, allocClass(itemClass), itemInit, btn);
            msg1(void, id, navItem, sel_getUid(setters[i]), item);
            releaseObj(item);
        }
    }
}

void setVCTitle(id vc, CFStringRef title) {
    msg1(void, CFStringRef, getNavItem(vc), sel_getUid("setTitle:"), title);
    CFRelease(title);
}

void updateSegmentedControl(id view, id foreground, unsigned char darkMode) {
    float redGreen = 0.78f, blue = 0.8f;
    if (darkMode) {
        redGreen = 0.28f;
        blue = 0.29f;
    }
    id tint = msg4(id, CGFloat, CGFloat, CGFloat, CGFloat, allocClass(ColorClass),
                   sel_getUid("initWithRed:green:blue:alpha:"), redGreen, redGreen, blue, 1);
    setTintColor(view, tint);
    releaseObj(tint);
    const void *keys[] = {
        (CFStringRef) NSForegroundColorAttributeName, (CFStringRef) NSFontAttributeName
    };
    const void *normalVals[] = {foreground, getSystemFont(13, UIFontWeightRegular)};
    const void *selectedVals[] = {foreground, getSystemFont(13, UIFontWeightMedium)};
    CFDictionaryRef normalDict = CFDictionaryCreate(NULL, keys, normalVals, 2,
                                                    &kCFCopyStringDictionaryKeyCallBacks, NULL);
    CFDictionaryRef selectedDict = CFDictionaryCreate(NULL, keys, selectedVals, 2,
                                                      &kCFCopyStringDictionaryKeyCallBacks, NULL);
    setControlTextAttribs(view, normalDict, 0);
    setControlTextAttribs(view, selectedDict, 4);
    CFRelease(normalDict);
    CFRelease(selectedDict);
}
