#include "ViewControllerHelpers.h"
#include <CoreFoundation/CFAttributedString.h>
#include "AppDelegate.h"

#define updateKVPair(o, k, v) msg2(void, id, CFStringRef, o, sel_getUid("setValue:forKey:"), v, k)

#define getNavBar(_navVC) msg0(id, _navVC, sel_getUid("navigationBar"))

extern CGFloat UIFontWeightSemibold;

Class DMNavVC;
Class VCClass;
size_t VCSize;

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

static void setupBarGeneric(id bar, Class appearanceClass, bool modal) {
    id color = getBarColor(modal);
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = createNew(appearanceClass);
        msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
        setBackground(appearance, color);
        msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
        if (msg1(bool, SEL, bar, sel_getUid("respondsToSelector:"), scrollEdge))
            msg1(void, id, bar, scrollEdge, appearance);
        releaseObj(appearance);
    } else {
        msg1(void, bool, bar, sel_getUid("setTranslucent:"), false);
        setBarTint(bar, color);
    }
}

void setupTabVC(id vc) {
    const void *keys[] = {(CFStringRef) NSForegroundColorAttributeName};
    const void *vals[] = {createColor(ColorLabel)};
    CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, vals, 1,
                                              &kCFCopyStringDictionaryKeyCallBacks, NULL);
    Class navBarAppearance = objc_getClass("UINavigationBarAppearance");
    id tabBar = msg0(id, vc, sel_getUid("tabBar"));
    setupBarGeneric(tabBar, objc_getClass("UITabBarAppearance"), false);
    if (!navBarAppearance)
        msg1(void, id, tabBar, sel_getUid("setUnselectedItemTintColor:"), createColor(ColorGray));
    CFArrayRef ctrls = getViewControllers(vc);
    for (int i = 0; i < 3; ++i) {
        id navVC = (id) CFArrayGetValueAtIndex(ctrls, i);
        id navBar = getNavBar(navVC);
        setupBarGeneric(navBar, navBarAppearance, false);
        if (!navBarAppearance) {
            msg1(void, CFDictionaryRef, navBar, sel_getUid("setTitleTextAttributes:"), dict);
            msg0(void, navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
    CFRelease(dict);
}

void setVCTitle(id vc, CFStringRef title) {
    msg1(void, CFStringRef, getNavItem(vc), sel_getUid("setTitle:"), title);
    CFRelease(title);
}

int dmNavVC_getStatusBarStyle(id self _U_, SEL _cmd _U_) { return 0; }

int dmNavVC_getStatusBarStyleDark(id self _U_, SEL _cmd _U_) { return 2; }

#pragma mark - VC Functions

void presentVC(id child) {
    id window = appDel_getWindow();
    setTintColor(window, nil);
    msg3(void, id, bool, id, getRootVC(window),
         sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id modal) {
    id container = createNavVC(modal);
    setupBarGeneric(getNavBar(container), objc_getClass("UINavigationBarAppearance"), true);
    presentVC(container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(Callback handler) {
    id window = appDel_getWindow();
    msg2(void, bool, Callback, getRootVC(window),
         sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler)
            handler();
        setTintColor(window, createColor(ColorRed));
    });
}

id createAlertController(CFStringRef title, CFStringRef message) {
    id ctrl = clsF2(id, CFStringRef, CFStringRef, objc_getClass("UIAlertController"),
                    sel_getUid("getCtrlWithTitle:message:"), title, message);
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

id alertCtrlCreate(id self _U_, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    return clsF3(id, CFStringRef, CFStringRef, long, objc_getClass("UIAlertController"),
                 sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

id alertCtrlCreateLegacy(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    id vc = alertCtrlCreate(self, nil, title, message);
    id foreground = createColor(ColorLabel);
    const void *keys[] = {
        (CFStringRef) NSForegroundColorAttributeName, (CFStringRef) NSFontAttributeName
    };
    const void *titleVals[] = {foreground, getSystemFont(17, UIFontWeightSemibold)};
    const void *msgVals[] = {foreground, getSystemFont(13, UIFontWeightRegular)};
    CFDictionaryRef titleDict = CFDictionaryCreate(NULL, keys, titleVals, 2,
                                                   &kCFCopyStringDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = CFDictionaryCreate(NULL, keys, msgVals, 2,
                                                 &kCFCopyStringDictionaryKeyCallBacks,
                                                 &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    updateKVPair(vc, CFSTR("attributedTitle"), (id) titleString);
    updateKVPair(vc, CFSTR("attributedMessage"), (id) msgString);

    id view = getView(vc);
    for (int i = 0; i < 3; ++i) {
        view = (id) CFArrayGetValueAtIndex(getSubviews(view), 0);
    }
    setBackground(view, createColor(ColorTertiaryBG));

    CFRelease(titleDict);
    CFRelease(msgDict);
    CFRelease(titleString);
    CFRelease(msgString);
    return vc;
}

void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler) {
    id action = clsF3(id, CFStringRef, long, void(^)(id), objc_getClass("UIAlertAction"),
                      sel_getUid("actionWithTitle:style:handler:"), title, style, ^(id hdlr _U_) {
        if (handler)
            handler();
        setTintColor(appDel_getWindow(), createColor(ColorRed));
    });
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}
