#include "ViewControllerHelpers.h"
#include <CoreFoundation/CFAttributedString.h>
#include "AppDelegate.h"

#define updateKVPair(_o, _k, _v) ((void(*)(id,SEL,id,CFStringRef))objc_msgSend)\
((_o), sel_getUid("setValue:forKey:"), (_v), (_k))

Class DMNavVC;
Class VCClass;
size_t VCSize;

void setNavButton(id vc, bool left, id button, int totalWidth) {
    id navItem = getNavItem(vc);
    ((void(*)(id,SEL,CGRect))objc_msgSend)(button, sel_getUid("setFrame:"),
                                           (CGRect){{0}, {totalWidth >> 2, 30}});
    id _obj = allocClass(objc_getClass("UIBarButtonItem"));
    id item = getObjectWithObject(_obj, sel_getUid("initWithCustomView:"), button);
    if (left)
        setObject(navItem, sel_getUid("setLeftBarButtonItem:"), item);
    else
        setObject(navItem, sel_getUid("setRightBarButtonItem:"), item);
    releaseObj(item);
}

static bool setupBarGeneric(id bar, Class appearanceClass, bool modal) {
    id color = getBarColor(modal);
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = createNew(appearanceClass);
        voidFunc(appearance, sel_getUid("configureWithOpaqueBackground"));
        setBackground(appearance, color);
        setObject(bar, sel_getUid("setStandardAppearance:"), appearance);
        if (((bool(*)(id,SEL,SEL))objc_msgSend)(bar, sel_getUid("respondsToSelector:"), scrollEdge))
            setObject(bar, scrollEdge, appearance);
        releaseObj(appearance);
        return false;
    } else {
        setBool(bar, sel_getUid("setTranslucent:"), false);
        setBarTint(bar, color);
        return true;
    }
}

void setupTabVC(id vc) {
    CFDictionaryRef dict = createTitleTextDict(createColor(ColorLabel), nil);
    Class navBarAppearance = objc_getClass("UINavigationBarAppearance");
    id tabBar = getTabBar(vc);
    bool extra = setupBarGeneric(tabBar, objc_getClass("UITabBarAppearance"), false);
    if (extra)
        setObject(tabBar, sel_getUid("setUnselectedItemTintColor:"), createColor(ColorGray));
    CFArrayRef ctrls = getViewControllers(vc);
    for (int i = 0; i < 3; ++i) {
        id navVC = (id) CFArrayGetValueAtIndex(ctrls, i);
        id navBar = getNavBar(navVC);
        setupBarGeneric(navBar, navBarAppearance, false);
        if (extra) {
            (((void(*)(id,SEL,CFDictionaryRef))objc_msgSend)
             (navBar, sel_getUid("setTitleTextAttributes:"), dict));
            voidFunc(navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
    CFRelease(dict);
}

void setupNavBar(id vc, bool modal) {
    setupBarGeneric(getNavBar(vc), objc_getClass("UINavigationBarAppearance"), modal);
}

void setVCTitle(id vc, CFStringRef title) {
    id navItem = getNavItem(vc);
    setString(navItem, sel_getUid("setTitle:"), title);
    CFRelease(title);
}

int dmNavVC_getStatusBarStyle(id self _U_, SEL _cmd _U_) { return 0; }

int dmNavVC_getStatusBarStyleDark(id self _U_, SEL _cmd _U_) { return 2; }

#pragma mark - VC Functions

id createNavVC(id child) {
    id _obj = allocClass(DMNavVC);
    return getObjectWithObject(_obj, sel_getUid("initWithRootViewController:"), child);
}

void presentVC(id presenter, id child) {
    appDel_setWindowTint(nil);
    (((void(*)(id,SEL,id,bool,id))objc_msgSend)
     (presenter, sel_getUid("presentViewController:animated:completion:"), child, true, nil));
}

void presentModalVC(id presenter, id modal) {
    id container = createNavVC(modal);
    setupNavBar(container, true);
    presentVC(presenter, container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter, Callback handler) {
    (((void(*)(id,SEL,bool,Callback))objc_msgSend)
     (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler)
            handler();
        appDel_setWindowTint(createColor(ColorRed));
    }));
}

id createAlertController(CFStringRef title, CFStringRef message) {
    id ctrl = (((id(*)(Class,SEL,CFStringRef,CFStringRef))objc_msgSend)
               (objc_getClass("UIAlertController"),
                sel_getUid("getCtrlWithTitle:message:"), title, message));
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

id alertCtrlCreate(id self _U_, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    return (((id(*)(Class,SEL,CFStringRef,CFStringRef,long))objc_msgSend)
            (objc_getClass("UIAlertController"),
             sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1));
}

id alertCtrlCreateLegacy(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    id vc = alertCtrlCreate(self, nil, title, message);
    id fg = createColor(ColorLabel);
    CFDictionaryRef titleDict = createTitleTextDict(fg, createCustomFont(WeightSemiBold, 17));
    CFDictionaryRef msgDict = createTitleTextDict(fg, createCustomFont(WeightReg, 13));
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    updateKVPair(vc, CFSTR("attributedTitle"), (id) titleString);
    updateKVPair(vc, CFSTR("attributedMessage"), (id) msgString);

    id view = getView(vc);
    SEL sv = sel_getUid("subviews");
    for (int i = 0; i < 3; ++i) {
        CFArrayRef subviews = getArray(view, sv);
        view = (id) CFArrayGetValueAtIndex(subviews, 0);
    }
    setBackground(view, createColor(ColorTertiaryBG));

    CFRelease(titleDict);
    CFRelease(msgDict);
    CFRelease(titleString);
    CFRelease(msgString);
    return vc;
}

void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler) {
    id action = (((id(*)(Class,SEL,CFStringRef,long,void(^)(id)))objc_msgSend)
                 (objc_getClass("UIAlertAction"), sel_getUid("actionWithTitle:style:handler:"),
                  title, style, ^(id hdlr _U_) {
        if (handler)
            handler();
        appDel_setWindowTint(createColor(ColorRed));
    }));
    setObject(ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}
