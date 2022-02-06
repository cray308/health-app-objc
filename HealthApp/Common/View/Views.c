#include "Views.h"
#include "AppUserData.h"
#include "CocoaHelpers.h"

#define getObjectWithArr(_obj, _cmd, _arg) \
(((id(*)(id,SEL,CFArrayRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define adjustFontForSizeCategory(_v) \
setBool(_v, sel_getUid("setAdjustsFontForContentSizeCategory:"), true)

#define setControlTextAttribs(_v, _dict, _state) (((void(*)(id,SEL,CFDictionaryRef,int))objc_msgSend)\
((_v), sel_getUid("setTitleTextAttributes:forState:"), (_dict), (_state)))

extern id UIFontTextStyleTitle1;
extern id UIFontTextStyleTitle2;
extern id UIFontTextStyleTitle3;
extern id UIFontTextStyleHeadline;
extern id UIFontTextStyleSubheadline;
extern id UIFontTextStyleBody;
extern id UIFontTextStyleFootnote;

extern CGFloat UIFontWeightRegular;
extern CGFloat UIFontWeightMedium;
extern CGFloat UIFontWeightSemibold;

extern id NSForegroundColorAttributeName;
extern id NSFontAttributeName;

size_t ViewSize;
Class ViewClass;

static void setDynamicFont(id view) {
    setBool(view, sel_getUid("setAdjustsFontSizeToFitWidth:"), true);
    adjustFontForSizeCategory(view);
    setCGFloat(view, sel_getUid("setMinimumScaleFactor:"), 0.85);
}

static inline void setCornerRadius(id view) {
    id layer = getLayer(view);
    setCGFloat(layer, sel_getUid("setCornerRadius:"), 5);
}

id createFont(int style) {
    id fStyle;
    switch (style) {
        case TextFootnote:
            fStyle = UIFontTextStyleFootnote;
            break;
        case TextSubhead:
            fStyle = UIFontTextStyleSubheadline;
            break;
        case TextBody:
            fStyle = UIFontTextStyleBody;
            break;
        case TextHead:
            fStyle = UIFontTextStyleHeadline;
            break;
        case TextTitle1:
            fStyle = UIFontTextStyleTitle1;
            break;
        default:
            fStyle = UIFontTextStyleTitle3;
    }
    return staticMethodWithString(objc_getClass("UIFont"),
                                  sel_getUid("preferredFontForTextStyle:"), (CFStringRef) fStyle);
}

static void setLabelFont(id view, int style) {
    setObject(view, sel_getUid("setFont:"), createFont(style));
}

id createCustomFont(int style, int size) {
    CGFloat weight;
    switch (style) {
        case WeightSemiBold:
            weight = UIFontWeightSemibold;
            break;
        case WeightMed:
            weight = UIFontWeightMedium;
            break;
        default:
            weight = UIFontWeightRegular;
    }
    return (((id(*)(Class,SEL,CGFloat,CGFloat))objc_msgSend)
            (objc_getClass("UIFont"), sel_getUid("systemFontOfSize:weight:"), size, weight));
}

CFDictionaryRef createTitleTextDict(id color, id font) {
    const void *keys[] = {
        (CFStringRef) NSForegroundColorAttributeName, (CFStringRef) NSFontAttributeName
    };
    const void *vals[] = {color, font};
    return CFDictionaryCreate(NULL, keys, vals, font ? 2 : 1,
                              &kCFCopyStringDictionaryKeyCallBacks, NULL);
}

#pragma mark - View initializers

id createObjectWithFrame(Class cls, CGRect frame) {
    id _obj = allocClass(cls);
    return ((id(*)(id,SEL,CGRect))objc_msgSend)(_obj, sel_getUid("initWithFrame:"), frame);
}

id createBackgroundView(int colorCode, int height, bool optional) {
    id view = createNew(ViewClass);
    disableAutoresizing(view);
    setBackground(view, createColor(colorCode));
    setHeight(view, height, optional);
    return view;
}

id createView(int size) {
    id view = createNew(ViewClass);
    disableAutoresizing(view);
    if (size >= 0) {
        setCornerRadius(view);
        setWidth(view, size);
        setHeight(view, size, true);
    }
    return view;
}

id createStackView(id *subviews, int count, int axis, int spacing, Padding margins) {
    id view;
    Class svClass = objc_getClass("UIStackView");
    if (count) {
        CFArrayRef arr = CFArrayCreate(NULL, (const void **)subviews, count, NULL);
        view = getObjectWithArr(allocClass(svClass), sel_getUid("initWithArrangedSubviews:"), arr);
        CFRelease(arr);
    } else {
        view = createNew(svClass);
    }
    disableAutoresizing(view);
    setInt(view, sel_getUid("setAxis:"), axis);
    setCGFloat(view, sel_getUid("setSpacing:"), spacing);
    setBool(view, sel_getUid("setLayoutMarginsRelativeArrangement:"), true);
    HAInsets insets = {margins.top, margins.left, margins.bottom, margins.right};
    setMargins(view, insets);
    return view;
}

id createScrollView(void) {
    id view = createNew(objc_getClass("UIScrollView"));
    disableAutoresizing(view);
    setInt(view, sel_getUid("setAutoresizingMask:"), 16);
    setBool(view, sel_getUid("setBounces:"), true);
    setBool(view, sel_getUid("setShowsVerticalScrollIndicator:"), true);
    return view;
}

id createLabel(CFStringRef text, int style, int alignment, bool accessible) {
    id view = createNew(objc_getClass("UILabel"));
    disableAutoresizing(view);
    setLabelText(view, text);
    setLabelFont(view, style);
    setDynamicFont(view);
    setTextColor(view, createColor(ColorLabel));
    setTextAlignment(view, alignment);
    setBool(view, sel_getUid("setIsAccessibilityElement:"), accessible);
    return view;
}

id createButton(CFStringRef title, int color, int params,
                int tag, id target, SEL action, int height) {
    id view = ((id(*)(Class,SEL,int))objc_msgSend)(objc_getClass("UIButton"),
                                                   sel_getUid("buttonWithType:"), 1);
    disableAutoresizing(view);
    setButtonTitle(view, title, 0);
    setButtonColor(view, createColor(color), 0);
    setButtonColor(view, createColor(ColorSecondaryLabel), 2);
    id label = getTitleLabel(view);
    setLabelFont(label, (params & BtnLargeFont) ? TextHead : TextBody);
    setDynamicFont(label);
    if (params & BtnBackground)
        setBackground(view, createColor(ColorSecondaryBGGrouped));
    if (params & BtnRounded)
        setCornerRadius(view);
    setTag(view, tag);
    addTarget(view, target, action, 64);
    if (height >= 0)
        setHeight(view, height, true);
    return view;
}

id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height) {
    CFStringRef segments[count];
    fillStringArray(segments, format, count);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, count, NULL);
    id _obj = allocClass(objc_getClass("UISegmentedControl"));
    id view = getObjectWithArr(_obj, sel_getUid("initWithItems:"), array);
    disableAutoresizing(view);
    setInt(view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    if (action)
        addTarget(view, target, action, 4096);
    if (height >= 0)
        setHeight(view, height, false);
    CFRelease(array);
    if (osVersion < 13)
        updateSegmentedControl(view);
    return view;
}

id createTextfield(id delegate, CFStringRef text, CFStringRef hint,
                   int alignment, int keyboard, int tag) {
    id view = createNew(objc_getClass("UITextField"));
    disableAutoresizing(view);
    setBackground(view, createColor(ColorTertiaryBG));
    setLabelText(view, text);
    setLabelFont(view, TextBody);
    setTextColor(view, createColor(ColorLabel));
    adjustFontForSizeCategory(view);
    setTextAlignment(view, alignment);
    setTag(view, tag);
    setInt(view, sel_getUid("setBorderStyle:"), 3);
    setInt(view, sel_getUid("setKeyboardType:"), keyboard);
    setDelegate(view, delegate);
    setMinHeight(view, 44);
    setAccessibilityLabel(view, hint);
    if (userData->darkMode == 1)
        setKBColor(view, 1);
    return view;
}

void addVStackToScrollView(id vStack, id scrollView) {
    addSubview(scrollView, vStack);
    pin(vStack, scrollView, (Padding){0}, 0);
    setEqualWidths(vStack, scrollView);
}

void updateSegmentedControl(id view) {
    id fg = createColor(ColorLabel);
    float redGreen = 0.78f, blue = 0.8f;
    if (userData->darkMode) {
        redGreen = 0.28f;
        blue = 0.29f;
    }
    setTintColor(view, getColorRef(redGreen, redGreen, blue, 1));
    CFDictionaryRef normalDict = createTitleTextDict(fg, createCustomFont(WeightReg, 13));
    CFDictionaryRef selectedDict = createTitleTextDict(fg, createCustomFont(WeightMed, 13));
    setControlTextAttribs(view, normalDict, 0);
    setControlTextAttribs(view, selectedDict, 4);
    CFRelease(normalDict);
    CFRelease(selectedDict);
}

void updateButtonColors(id view, int color) {
    setButtonColor(view, createColor(color), 0);
    setButtonColor(view, createColor(ColorSecondaryLabel), 2);
    setBackground(view, createColor(ColorSecondaryBGGrouped));
}
