//
//  ViewControllerHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "ViewControllerHelpers.h"
#include "AppDelegate.h"
#include "AppUserData.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>

static void toggleWindowTint(bool enable) {
    id color = enable ? createColor(ColorRed) : nil;
    id app = staticMethod(objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    AppDelegate *delegate = (AppDelegate *) getObject(app, sel_getUid("delegate"));
    setTintColor(delegate->window, color);
}

static inline id getNavBar(id navVC) {
    return getObject(navVC, sel_getUid("navigationBar"));
}

static inline CFArrayRef getSubviews(id view) {
    return getArray(view, sel_getUid("subviews"));
}

static inline void updateKVPair(id obj, CFStringRef key, id value) {
    ((void(*)(id,SEL,id,CFStringRef))objc_msgSend)(obj, sel_getUid("setValue:forKey:"), value, key);
}

static void setupBarTint(id navVC) {
    float redGreen = 0.95, blue = 0.97;
    if (userData->darkMode) {
        redGreen = 0.17;
        blue = 0.18;
    }
    setObject(getNavBar(navVC),
              sel_getUid("setBarTintColor:"), getColorRef(redGreen, redGreen, blue, 1));
}

static inline id createAttribString(CFStringRef text, CFDictionaryRef dict) {
    return ((id(*)(id,SEL,CFStringRef,CFDictionaryRef))objc_msgSend)
    (allocClass("NSAttributedString"), sel_getUid("initWithString:attributes:"), text, dict);
}

void setNavButton(id navItem, bool left, id button, CGFloat totalWidth) {
    ((void(*)(id,SEL,CGRect))objc_msgSend)(button, sel_getUid("setFrame:"),
                                           (CGRect){{0}, {totalWidth / 3, 30}});
    id item = getObjectWithObject(allocClass("UIBarButtonItem"),
                                  sel_getUid("initWithCustomView:"), button);
    if (left)
        setObject(navItem, sel_getUid("setLeftBarButtonItem:"), item);
    else
        setObject(navItem, sel_getUid("setRightBarButtonItem:"), item);
    releaseObj(item);
}

#pragma mark - VC Functions

id getView(id vc) {
    return getObject(vc, sel_getUid("view"));
}

void setupNavVC(id navVC, id firstVC) {
    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){firstVC}, 1, &(CFArrayCallBacks){0});
    setArray(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
    CFRelease(array);
}

void updateTabBar(id tabVC) {
    id bar = getObject(tabVC, sel_getUid("tabBar"));
    setObject(bar, sel_getUid("setBarTintColor:"), createColor(ColorSystemBackground));
    setObject(bar, sel_getUid("setUnselectedItemTintColor:"), createColor(ColorGray));
}

void updateNavBar(id navVC) {
    setupBarTint(navVC);
    CFDictionaryRef dict = createTitleTextDict(createColor(ColorLabel), nil);
    ((void(*)(id,SEL,CFDictionaryRef))objc_msgSend)(getNavBar(navVC),
                                                    sel_getUid("setTitleTextAttributes:"), dict);
    CFRelease(dict);
}

id getFirstVC(id navVC) {
    CFArrayRef ctrls = getArray(navVC, sel_getUid("viewControllers"));
    return (id) CFArrayGetValueAtIndex(ctrls, 0);
}

id allocNavVC(void) {
    return allocClass("UINavigationController");
}

void presentVC(id presenter, id child) {
    if (osVersion != Version14)
        toggleWindowTint(false);
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id presenter, id modal) {
    id container = getObjectWithObject(allocNavVC(),
                                       sel_getUid("initWithRootViewController:"), modal);
    if (osVersion == Version12)
        setupBarTint(container);
    presentVC(presenter, container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter, Callback handler) {
    ((void(*)(id,SEL,bool,Callback))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler)
            handler();
        if (osVersion != Version14)
            toggleWindowTint(true);
    });
}

id createAlertController(CFStringRef title, CFStringRef message) {
    id vc = ((id(*)(Class,SEL,CFStringRef,CFStringRef,int))objc_msgSend)
    (objc_getClass("UIAlertController"),
     sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
    if (osVersion == Version12) {
        id fg = createColor(ColorLabel);
        CFDictionaryRef titleDict = createTitleTextDict(fg, createCustomFont(WeightSemiBold, 17));
        CFDictionaryRef msgDict = createTitleTextDict(fg, createCustomFont(WeightReg, 13));
        id titleString = createAttribString(title, titleDict);
        id msgString = createAttribString(message, msgDict);
        updateKVPair(vc, CFSTR("attributedTitle"), titleString);
        updateKVPair(vc, CFSTR("attributedMessage"), msgString);

        id view = getView(vc);
        CFArrayRef subviews = getSubviews(view);
        view = (id) CFArrayGetValueAtIndex(subviews, 0);
        subviews = getSubviews(view);
        view = (id) CFArrayGetValueAtIndex(subviews, 0);
        subviews = getSubviews(view);
        view = (id) CFArrayGetValueAtIndex(subviews, 0);
        setBackground(view, createColor(ColorTertiarySystemBackground));

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
        if (osVersion != Version14)
            toggleWindowTint(true);
    });
    setObject(ctrl, sel_getUid("addAction:"), action);
}
