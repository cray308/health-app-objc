#include "Views_VCExt.h"

enum {
    UIAlertControllerStyleAlert = 1
};

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

void setupBarGeneric(id bar, Class appearanceClass, id color) {
    SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
    id appearance = new(appearanceClass);
    msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
    setBackgroundColor(appearance, color);
    msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
    if (msg1(bool, SEL, bar, sel_getUid("respondsToSelector:"), scrollEdge))
        msg1(void, id, bar, scrollEdge, appearance);
    releaseO(appearance);
}

#pragma mark - VC Presentation

void presentVC(id presenter, id child) {
    msg3(void, id, bool, Callback, presenter,
         sel_getUid("presentViewController:animated:completion:"), child, true, NULL);
}

void presentModalVC(id presenter, id modal) {
    id _c = alloc(objc_getClass("UINavigationController"));
    id nav = msg1(id, id, _c, sel_getUid("initWithRootViewController:"), modal);
    id color = clsF1(id, CFStringRef, UIColor, sel_getUid("colorNamed:"), CFSTR("modalColor"));
    id bar = msg0(id, nav, sel_getUid("navigationBar"));
    setupBarGeneric(bar, objc_getClass("UINavigationBarAppearance"), color);
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
    id ctrl = (((id(*)(Class,SEL,CFStringRef,CFStringRef,long))objc_msgSend)
               (objc_getClass("UIAlertController"),
                sel_getUid("alertControllerWithTitle:message:preferredStyle:"),
                title, message, UIAlertControllerStyleAlert));
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler) {
    CFStringRef title = localize(titleKey);
    id action = (((id(*)(Class,SEL,CFStringRef,long,void(^)(id)))objc_msgSend)
                 (objc_getClass("UIAlertAction"),
                  sel_getUid("actionWithTitle:style:handler:"), title, style, ^(id act _U_) {
        if (handler) handler();
    }));
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}
