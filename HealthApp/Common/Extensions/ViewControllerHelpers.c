#include "ViewControllerHelpers.h"
#include "AppDelegate.h"
#include "AppUserData.h"

#define getNavBar(_navVC) getObject(_navVC, sel_getUid("navigationBar"))

#define getSubviews(_view) getArray(_view, sel_getUid("subviews"))

#define updateKVPair(_o, _k, _v) ((void(*)(id,SEL,id,CFStringRef))objc_msgSend)\
((_o), sel_getUid("setValue:forKey:"), (_v), (_k))

Class DMTabVC;
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

void setVCTitle(id vc, CFStringRef title) {
    id navItem = getNavItem(vc);
    setString(navItem, sel_getUid("setTitle:"), title);
}

void dmTabVC_updateColors(id self, SEL _cmd _U_) {
    SEL setter = sel_getUid("setTitleTextAttributes:");
    SEL barUpdate = sel_getUid("setNeedsStatusBarAppearanceUpdate");
    id tabBar = getObject(self, sel_getUid("tabBar"));
    id tint = createColor(ColorPrimaryBG), unselected = createColor(ColorGray);
    setObject(tabBar, sel_getUid("setBarTintColor:"), tint);
    setObject(tabBar, sel_getUid("setUnselectedItemTintColor:"), unselected);
    CFArrayRef ctrls = getViewControllers(self);
    int count = (int) CFArrayGetCount(ctrls);
    for (int i = 0; i < count; ++i) {
        id navVC = (id) CFArrayGetValueAtIndex(ctrls, i);
        id navBar = getNavBar(navVC);
        setBarTint(navBar);
        CFDictionaryRef dict = createTitleTextDict(createColor(ColorLabel), nil);
        ((void(*)(id,SEL,CFDictionaryRef))objc_msgSend)(navBar, setter, dict);
        CFRelease(dict);
        voidFunc(navVC, barUpdate);
    }
}

int dmNavVC_getStatusBarStyle(id self _U_, SEL _cmd _U_) {
    return userData->darkMode ? 2 : 0;
}

#pragma mark - VC Functions

void setupNavVC(id navVC, id firstVC) {
    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){firstVC}, 1, &(CFArrayCallBacks){0});
    setArray(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
    CFRelease(array);
}

id getFirstVC(id navVC) {
    CFArrayRef ctrls = getArray(navVC, sel_getUid("viewControllers"));
    return (id) CFArrayGetValueAtIndex(ctrls, 0);
}

void presentVC(id presenter, id child) {
    if (osVersion < 14)
        appDel_setWindowTint(nil);
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id presenter, id modal) {
    id _obj = allocNavVC();
    id container = getObjectWithObject(_obj, sel_getUid("initWithRootViewController:"), modal);
    if (osVersion < 13) {
        id navBar = getNavBar(container);
        setBarTint(navBar);
    }
    presentVC(presenter, container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter, Callback handler) {
    ((void(*)(id,SEL,bool,Callback))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler)
            handler();
        if (osVersion < 14)
            appDel_setWindowTint(createColor(ColorRed));
    });
}

id createAlertController(CFStringRef title, CFStringRef message) {
    id vc = ((id(*)(Class,SEL,CFStringRef,CFStringRef,int))objc_msgSend)
    (objc_getClass("UIAlertController"),
     sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
    if (osVersion < 13) {
        id fg = createColor(ColorLabel);
        CFDictionaryRef titleDict = createTitleTextDict(fg, createCustomFont(WeightSemiBold, 17));
        CFDictionaryRef msgDict = createTitleTextDict(fg, createCustomFont(WeightReg, 13));
        id titleString = createAttribString(title, titleDict);
        id msgString = createAttribString(message, msgDict);
        updateKVPair(vc, CFSTR("attributedTitle"), titleString);
        updateKVPair(vc, CFSTR("attributedMessage"), msgString);

        id view = getView(vc);
        for (int i = 0; i < 3; ++i) {
            CFArrayRef subviews = getSubviews(view);
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
    id action = ((id(*)(Class,SEL,CFStringRef,int,void(^)(id)))objc_msgSend)
    (objc_getClass("UIAlertAction"), sel_getUid("actionWithTitle:style:handler:"), title, style,
     ^(id action _U_) {
        if (handler)
            handler();
        if (osVersion < 14)
            appDel_setWindowTint(createColor(ColorRed));
    });
    setObject(ctrl, sel_getUid("addAction:"), action);
}
