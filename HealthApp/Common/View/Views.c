#include "Views.h"
#include "CocoaHelpers.h"

#define getObjectWithArr(_obj, _cmd, _arg) \
(((id(*)(id,SEL,CFArrayRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define adjustFontForSizeCategory(_v) \
setBool(_v, sel_getUid("setAdjustsFontForContentSizeCategory:"), true)

#define setControlTextAttribs(_v, _dict, _state) (((void(*)(id,SEL,CFDictionaryRef,unsigned long))objc_msgSend)\
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
                              &kCFCopyStringDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
}

#pragma mark - View initializers

id createObjectWithFrame(Class cls, CGRect frame) {
    id _obj = allocClass(cls);
    return ((id(*)(id,SEL,CGRect))objc_msgSend)(_obj, sel_getUid("initWithFrame:"), frame);
}

id createView(void) {
    id view = createNew(ViewClass);
    disableAutoresizing(view);
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

id createLabel(CFStringRef text, int style, bool accessible) {
    id view = createNew(objc_getClass("UILabel"));
    disableAutoresizing(view);
    setLabelText(view, text);
    if (text)
        CFRelease(text);
    setLabelFont(view, style);
    setDynamicFont(view);
    setTextColor(view, createColor(ColorLabel));
    setTextAlignment(view, 4);
    setBool(view, sel_getUid("setIsAccessibilityElement:"), accessible);
    return view;
}

id createButton(CFStringRef title, int color, int params, int tag, id target, SEL action) {
    id view = ((id(*)(Class,SEL,long))objc_msgSend)(objc_getClass("UIButton"),
                                                    sel_getUid("buttonWithType:"), 1);
    disableAutoresizing(view);
    setButtonTitle(view, title, 0);
    if (title)
        CFRelease(title);
    setButtonColor(view, createColor(color), 0);
    setButtonColor(view, createColor(ColorSecondaryLabel), 2);
    id label = getTitleLabel(view);
    setLabelFont(label, (params & BtnLargeFont) ? TextHead : TextBody);
    setDynamicFont(label);
    if (params & BtnBackground)
        setBackground(view, createColor(ColorSecondaryBGGrouped));
    if (params & BtnRounded)
        setCGFloat(getLayer(view), sel_getUid("setCornerRadius:"), 5);
    setTag(view, tag);
    addTarget(view, target, action, 64);
    return view;
}

id createSegmentedControl(CFBundleRef bundle, CFStringRef format, int startIndex) {
    CFStringRef segments[3];
    fillStringArray(bundle, segments, format, 3);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, 3, NULL);
    id _obj = allocClass(objc_getClass("UISegmentedControl"));
    id view = getObjectWithArr(_obj, sel_getUid("initWithItems:"), array);
    disableAutoresizing(view);
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
    CFRelease(hint);
    return view;
}

void addVStackToScrollView(id vStack, id scrollView) {
    addSubview(scrollView, vStack);
    pin(vStack, scrollView, (Padding){0}, 0);
    setEqualWidths(vStack, scrollView);
}

void updateSegmentedControl(id view, id foreground, unsigned char darkMode) {
    float redGreen = 0.78f, blue = 0.8f;
    if (darkMode) {
        redGreen = 0.28f;
        blue = 0.29f;
    }
    setTintColor(view, getColorRef(redGreen, redGreen, blue, 1));
    CFDictionaryRef normalDict = createTitleTextDict(foreground, createCustomFont(WeightReg, 13));
    CFDictionaryRef selectedDict = createTitleTextDict(foreground, createCustomFont(WeightMed, 13));
    setControlTextAttribs(view, normalDict, 0);
    setControlTextAttribs(view, selectedDict, 4);
    CFRelease(normalDict);
    CFRelease(selectedDict);
}
