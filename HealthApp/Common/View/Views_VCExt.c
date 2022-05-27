#include "Views_VCExt.h"
#include "SwiftBridging.h"

#define statusBarStyle(dm) (dm ? UIStatusBarStyleBlackOpaque : UIStatusBarStyleDefault)

extern CGFloat UIFontWeightSemibold;

Class DMNavVC;

enum {
    UIStatusBarStyleDefault,
    UIStatusBarStyleBlackOpaque = 2
};

enum {
    UIAlertControllerStyleAlert = 1
};

static int barStyle;

static int getStatusBarStyle(id, SEL);
static id alertCtrlCreate(id, SEL, CFStringRef, CFStringRef);
static id alertCtrlCreateLegacy(id, SEL, CFStringRef, CFStringRef);

bool initVCData(bool modern, unsigned char darkMode, void (*cascade)(bool)) {
    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    objc_registerClassPair(DMNavVC);

    IMP alertImp = (IMP)alertCtrlCreate;
    if (!modern) {
        barStyle = statusBarStyle(darkMode);
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"), (IMP)getStatusBarStyle, "i@:");
        alertImp = (IMP)alertCtrlCreateLegacy;
        setupAppColors(darkMode, false);
    }
    class_addMethod(objc_getMetaClass("UIAlertController"),
                    sel_registerName("getCtrlWithTitle:message:"), alertImp, "@@:@@");
    cascade(modern);
    return setupCharts(darkMode);
}

#pragma mark - VC Setup

void setupNavItem(id vc, CFStringRef titleKey, id *buttons) {
    static const char *const setters[] = {"setLeftBarButtonItem:", "setRightBarButtonItem:"};
    id navItem = msg0(id, vc, sel_getUid("navigationItem"));
    CFStringRef title = localize(titleKey);
    msg1(void, CFStringRef, navItem, sel_getUid("setTitle:"), title);
    CFRelease(title);
    if (!buttons) return;

    Class BarItem = objc_getClass("UIBarButtonItem");
    SEL itemInit = sel_getUid("initWithCustomView:");
    for (int i = 0; i < 2; ++i) {
        id btn = buttons[i];
        if (btn) {
            setTrans(btn);
            id item = msg1(id, id, alloc(BarItem), itemInit, btn);
            msg1(void, id, navItem, sel_getUid(setters[i]), item);
            releaseO(item);
        }
    }
}

void setupHierarchy(id vc, id vStack, id scrollView, int color) {
    id view = msg0(id, vc, sel_getUid("view"));
    setBackgroundColor(view, getColor(color));
    setTrans(scrollView);
    setTrans(vStack);
    addSubview(view, scrollView);
    addSubview(scrollView, vStack);
    pin(scrollView, msg0(id, view, sel_getUid("safeAreaLayoutGuide")));
    pin(vStack, scrollView);
    id width = makeConstraint(vStack, NSLayoutAttributeWidth, 0,
                              scrollView, NSLayoutAttributeWidth, 0);
    setActive(width);
    releaseV(scrollView);
    releaseV(vStack);
}

#pragma mark - VC Appearance

int getStatusBarStyle(id self _U_, SEL _cmd _U_) { return barStyle; }

static void setupNavBarColor(id bar) {
    const void *keys[] = {NSForegroundColorAttributeName};
    const void *vals[] = {getColor(ColorLabel)};
    CFDictionaryRef dict = CFDictionaryCreate(
      NULL, keys, vals, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    msg1(void, CFDictionaryRef, bar, sel_getUid("setTitleTextAttributes:"), dict);
    CFRelease(dict);
}

static void setupBarGeneric(id bar, Class appearanceClass, id color) {
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = new(appearanceClass);
        msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
        setBackgroundColor(appearance, color);
        msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
        if (msg1(bool, SEL, bar, sel_getUid("respondsToSelector:"), scrollEdge))
            msg1(void, id, bar, scrollEdge, appearance);
        releaseO(appearance);
    } else {
        msg1(void, bool, bar, sel_getUid("setTranslucent:"), false);
        msg1(void, id, bar, sel_getUid("setBarTintColor:"), color);
    }
}

void setupTabVC(id vc, Class TabAppear) {
    Class NavAppear = objc_getClass("UINavigationBarAppearance");
    id tabBar = msg0(id, vc, sel_getUid("tabBar"));
    id color = clsF1(id, int, UIColor, sel_getUid("getBarColorWithType:"), BarColorNav);
    SEL nb = sel_getUid("navigationBar");
    setupBarGeneric(tabBar, TabAppear, color);
    if (!TabAppear)
        msg1(void, id, tabBar, sel_getUid("setUnselectedItemTintColor:"), getColor(ColorGray));
    CFArrayRef ctrls = msg0(CFArrayRef, vc, sel_getUid("viewControllers"));
    for (int i = 0; i < 3; ++i) {
        id navVC = (id)CFArrayGetValueAtIndex(ctrls, i);
        id navBar = msg0(id, navVC, nb);
        setupBarGeneric(navBar, NavAppear, color);
        if (!TabAppear) {
            setupNavBarColor(navBar);
            msg0(void, navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
}

void handleTintChange(id window, bool darkMode) {
    barStyle = statusBarStyle(darkMode);
    setupAppColors(darkMode, true);
    msg1(void, id, window, sel_getUid("setTintColor:"), getColor(ColorRed));
    setupTabVC(msg0(id, window, sel_getUid("rootViewController")), nil);
    setupCharts(darkMode);
}

#pragma mark - VC Presentation

static inline void presentVC(id presenter, id child) {
    msg3(void, id, bool, Callback, presenter,
         sel_getUid("presentViewController:animated:completion:"), child, true, NULL);
}

void presentModalVC(id presenter, id modal) {
    id nav = msg1(id, id, alloc(DMNavVC), sel_getUid("initWithRootViewController:"), modal);
    id color = clsF1(id, int, UIColor, sel_getUid("getBarColorWithType:"), BarColorModal);
    id bar = msg0(id, nav, sel_getUid("navigationBar"));
    Class appear = objc_getClass("UINavigationBarAppearance");
    setupBarGeneric(bar, appear, color);
    if (!appear) setupNavBarColor(bar);
    presentVC(presenter, nav);
    releaseVC(nav);
    releaseVC(modal);
}

void dismissPresentedVC(id vc, Callback handler) {
    id nav = msg0(id, vc, sel_getUid("navigationController"));
    msg2(void, bool, Callback, msg0(id, nav, sel_getUid("presentingViewController")),
         sel_getUid("dismissViewControllerAnimated:completion:"), true, handler);
}

#pragma mark - Alerts

id createAlertController(CFStringRef titleKey, CFStringRef msgKey) {
    CFStringRef title = localize(titleKey), message = localize(msgKey);
    id ctrl = (((id(*)(Class,SEL,CFStringRef,CFStringRef))objc_msgSend)
               (objc_getClass("UIAlertController"),
                sel_getUid("getCtrlWithTitle:message:"), title, message));
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

id alertCtrlCreate(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    return (((id(*)(Class,SEL,CFStringRef,CFStringRef,long))objc_msgSend)
            ((Class)self, sel_getUid("alertControllerWithTitle:message:preferredStyle:"),
             title, message, UIAlertControllerStyleAlert));
}

id alertCtrlCreateLegacy(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    id vc = alertCtrlCreate(self, nil, title, message);
    id fg = getColor(ColorLabel);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    Class Font = objc_getClass("UIFont");
    SEL gf = sel_getUid("systemFontOfSize:weight:");
    const void *tv[] = {
        fg, ((id(*)(Class,SEL,CGFloat,CGFloat))objc_msgSend)(Font, gf, 17, UIFontWeightSemibold)
    };
    const void *mv[] = {
        fg, ((id(*)(Class,SEL,CGFloat,CGFloat))objc_msgSend)(Font, gf,
                                                             FontSizeReg, UIFontWeightRegular)
    };
    CFDictionaryRef titleDict = CFDictionaryCreate(
      NULL, keys, tv, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = CFDictionaryCreate(
      NULL, keys, mv, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    SEL sKV = sel_getUid("setValue:forKey:"), gsv = sel_getUid("subviews");
    msg2(void, id, CFStringRef, vc, sKV, (id)titleString, CFSTR("attributedTitle"));
    msg2(void, id, CFStringRef, vc, sKV, (id)msgString, CFSTR("attributedMessage"));

    id view = msg0(id, vc, sel_getUid("view"));
    for (int i = 0; i < 3; ++i) {
        view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, gsv), 0);
    }
    setBackgroundColor(view, getColor(ColorTertiaryBG));

    CFRelease(titleDict);
    CFRelease(msgDict);
    CFRelease(titleString);
    CFRelease(msgString);
    return vc;
}

void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler) {
    CFStringRef title = localize(titleKey);
    id action = (((id(*)(Class,SEL,CFStringRef,long,void(^)(id)))objc_msgSend)
                 (objc_getClass("UIAlertAction"),
                  sel_getUid("actionWithTitle:style:handler:"), title, style, ^(id act _U_) {
        if (handler) handler();
        msg1(void, id, getAppWindow(), sel_getUid("setTintColor:"), getColor(ColorRed));
    }));
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}

void showAlert(id ctrl) {
    id window = getAppWindow();
    msg1(void, id, window, sel_getUid("setTintColor:"), nil);
    presentVC(msg0(id, window, sel_getUid("rootViewController")), ctrl);
}
