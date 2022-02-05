#include "ViewControllerHelpers.h"
#include "AppDelegate.h"
#include "AppUserData.h"

#define updateKVPair(_o, _k, _v) ((void(*)(id,SEL,id,CFStringRef))objc_msgSend)\
((_o), sel_getUid("setValue:forKey:"), (_v), (_k))

Class DMNavVC;

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
    if (osVersion > 12) {
        id appearance = createNew(appearanceClass);
        voidFunc(appearance, sel_getUid("configureWithOpaqueBackground"));
        setBackground(appearance, color);
        setObject(bar, sel_getUid("setStandardAppearance:"), appearance);
        if (osVersion == 15)
            setObject(bar, sel_getUid("setScrollEdgeAppearance:"), appearance);
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
    id tabBar = getTabBar(vc);
    bool extra = setupBarGeneric(tabBar, objc_getClass("UITabBarAppearance"), false);
    if (extra)
        setObject(tabBar, sel_getUid("setUnselectedItemTintColor:"), createColor(ColorGray));
    CFArrayRef ctrls = getViewControllers(vc);
    for (int i = 0; i < 3; ++i) {
        id navVC = (id) CFArrayGetValueAtIndex(ctrls, i);
        id navBar = getNavBar(navVC);
        setupBarGeneric(navBar, objc_getClass("UINavigationBarAppearance"), false);
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
}

int dmNavVC_getStatusBarStyle(id self _U_, SEL _cmd _U_) {
    return userData->darkMode ? 2 : 0;
}

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
    id vc = (((id(*)(Class,SEL,CFStringRef,CFStringRef,int))objc_msgSend)
             (objc_getClass("UIAlertController"),
              sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1));
    if (osVersion < 13) {
        id fg = createColor(ColorLabel);
        CFDictionaryRef titleDict = createTitleTextDict(fg, createCustomFont(WeightSemiBold, 17));
        CFDictionaryRef msgDict = createTitleTextDict(fg, createCustomFont(WeightReg, 13));
        id titleString = createAttribString(title, titleDict);
        id msgString = createAttribString(message, msgDict);
        updateKVPair(vc, CFSTR("attributedTitle"), titleString);
        updateKVPair(vc, CFSTR("attributedMessage"), msgString);

        id view = getView(vc);
        SEL sv = sel_getUid("subviews");
        for (int i = 0; i < 3; ++i) {
            CFArrayRef subviews = getArray(view, sv);
            view = (id) CFArrayGetValueAtIndex(subviews, 0);
        }
        setBackground(view, createColor(ColorTertiaryBG));

        CFRelease(titleDict);
        CFRelease(msgDict);
        releaseObj(titleString);
        releaseObj(msgString);
    }
    return vc;
}

void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler) {
    id action = (((id(*)(Class,SEL,CFStringRef,int,void(^)(id)))objc_msgSend)
                 (objc_getClass("UIAlertAction"), sel_getUid("actionWithTitle:style:handler:"),
                  title, style, ^(id hdlr _U_) {
        if (handler)
            handler();
        appDel_setWindowTint(createColor(ColorRed));
    }));
    setObject(ctrl, sel_getUid("addAction:"), action);
}
