//
//  CocoaBridging.m
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#import "CocoaBridging.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

CFArrayCallBacks *kCocoaArrCallbacks;

static void disableAutoresizing(id view) {
    ((void(*)(id,SEL,bool))objc_msgSend)
    (view, sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), false);
}

static void setAlignment(id view, int alignment) {
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setTextAlignment:"), alignment);
}

static void setDynamicFont(id view, bool enable) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setAdjustsFontSizeToFitWidth:"), enable);
}

static void setTag(id view, int tag) {
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setTag:"), tag);
}

static void setBackground(id view, id color) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setBackgroundColor:"), color);
}

static void setCornerRadius(id view) {
    id layer = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("layer"));
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(layer, sel_getUid("setCornerRadius:"), 5);
}

static void setFont(id view, id style) {
    if (style) {
        id font = ((id(*)(Class,SEL,CFStringRef))objc_msgSend)
        (objc_getClass("UIFont"), sel_getUid("preferredFontForTextStyle:"), (CFStringRef)style);
        ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setFont:"), font);
    }
}

id objc_staticMethod(Class _self, SEL _cmd) {
    return ((id(*)(Class,SEL))objc_msgSend)(_self, _cmd);
}

void objc_singleArg(id obj, SEL _cmd) {
    ((void(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

id createChartEntry(int x, int y) {
    return [[ChartDataEntry alloc] initWithX:x y:y];
}

void getScreenBounds(CGRect *result) {
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)
    (result,
     objc_staticMethod(objc_getClass("UIScreen"), sel_getUid("mainScreen")), sel_getUid("bounds"));
}

id getUserDefaults(void) {
    return objc_staticMethod(objc_getClass("NSUserDefaults"), sel_getUid("standardUserDefaults"));
}

id createColor(const char *name) {
    return objc_staticMethod(objc_getClass("UIColor"), sel_getUid(name));
}

id createObjectWithFrame(const char *name, CGRect rect) {
    return ((id(*)(id,SEL,CGRect))objc_msgSend)(allocClass(name),
                                                sel_getUid("initWithFrame:"), rect);
}

id allocClass(const char *name) {
    return objc_staticMethod(objc_getClass(name), sel_getUid("alloc"));
}

void releaseObj(id obj) {
    objc_singleArg(obj, sel_getUid("release"));
}

id createVCWithDelegate(const char *name, void *delegate) {
    return ((id(*)(id,SEL,void*))objc_msgSend)(allocClass(name),
                                               sel_getUid("initWithDelegate:"), delegate);
}

void setupNavVC(id navVC, id firstVC) {
    id ctrls[] = {firstVC};
    CFArrayRef array = CFArrayCreate(NULL, (const void **)ctrls, 1, kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
    CFRelease(array);
}

id getFirstVC(id navVC) {
    CFArrayRef ctrls = ((CFArrayRef(*)(id,SEL))objc_msgSend)(navVC, sel_getUid("viewControllers"));
    return CFArrayGetValueAtIndex(ctrls, 0);
}

id allocNavVC(void) {
    return allocClass("UINavigationController");
}

void presentVC(id presenter, id modal) {
    id container = ((id(*)(id,SEL,id))objc_msgSend)
    (allocNavVC(), sel_getUid("initWithRootViewController:"), modal);
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), container, true, nil);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter) {
    ((void(*)(id,SEL,bool,id))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, nil);
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
        CFArrayRef array = CFArrayCreate(NULL, (const void **)subviews, count, kCocoaArrCallbacks);
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
    UIEdgeInsets insets = {margins.top, margins.left, margins.bottom, margins.right};
    ((void(*)(id,SEL,UIEdgeInsets))objc_msgSend)(view, sel_getUid("setLayoutMargins:"), insets);
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

id createLabel(CFStringRef text, id style, bool dynamicSize, int alignment) {
    id view = createObjectWithFrame("UILabel", CGRectZero);
    disableAutoresizing(view);
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(view, sel_getUid("setText:"), text);
    setFont(view, style);
    setDynamicFont(view, dynamicSize);
    ((void(*)(id,SEL,id))objc_msgSend)(view,
                                       sel_getUid("setTextColor:"), createColor("labelColor"));
    setAlignment(view, alignment);
    return view;
}

id createButton(CFStringRef title, id color, id disabledColor, id style,
                id background, bool rounded, bool dynamicSize, bool enabled, int tag) {
    id view = ((id(*)(Class,SEL,int))objc_msgSend)(objc_getClass("UIButton"),
                                                   sel_getUid("buttonWithType:"), 1);
    disableAutoresizing(view);
    ((void(*)(id,SEL,CFStringRef,int))objc_msgSend)(view, sel_getUid("setTitle:forState:"),
                                                    title, UIControlStateNormal);
    ((void(*)(id,SEL,id,int))objc_msgSend)(view, sel_getUid("setTitleColor:forState:"),
                                           color, UIControlStateNormal);
    if (disabledColor) {
        ((void(*)(id,SEL,id,int))objc_msgSend)(view, sel_getUid("setTitleColor:forState:"),
                                               disabledColor, UIControlStateDisabled);
    }
    id label = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("titleLabel"));
    setFont(label, style);
    setDynamicFont(label, dynamicSize);
    setBackground(view, background);
    if (rounded)
        setCornerRadius(view);
    setTag(view, tag);
    enableButton(view, enabled);
    return view;
}

id createSegmentedControl(CFStringRef *items, int count, int startIndex) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)items, count, kCocoaArrCallbacks);
    id view = ((id(*)(id,SEL,CFArrayRef))objc_msgSend)(allocClass("UISegmentedControl"),
                                                       sel_getUid("initWithItems:"), array);
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    setCornerRadius(view);
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setTintColor:"),
                                       createColor("systemGray2Color"));
    CFRelease(array);
    return view;
}

id createTextfield(id delegate, CFStringRef placeholder, int alignment, int keyboard) {
    id view = createObjectWithFrame("UITextField", CGRectZero);
    disableAutoresizing(view);
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setDelegate:"), delegate);
    setBackground(view, createColor("tertiarySystemBackgroundColor"));
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(view, sel_getUid("setPlaceholder:"), placeholder);
    setAlignment(view, alignment);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setBorderStyle:"), 3);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setKeyboardType:"), keyboard);
    return view;
}

void enableButton(id view, bool enabled) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setEnabled:"), enabled);
}

void activateConstraints(id *constraints, int count) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)constraints, count, kCocoaArrCallbacks);
    ((void(*)(Class,SEL,CFArrayRef))objc_msgSend)(objc_getClass("NSLayoutConstraint"),
                                                  sel_getUid("activateConstraints:"), array);
    CFRelease(array);
}
