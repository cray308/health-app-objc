//
//  ViewControllerHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "ViewControllerHelpers.h"

extern id NSForegroundColorAttributeName;

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

void setupNavVC(id navVC, id firstVC) {
    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){firstVC}, 1, &kCocoaArrCallbacks);
    setArray(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
    CFRelease(array);
}

id getFirstVC(id navVC) {
    CFArrayRef ctrls = ((CFArrayRef(*)(id,SEL))objc_msgSend)(navVC, sel_getUid("viewControllers"));
    return (id) CFArrayGetValueAtIndex(ctrls, 0);
}

id allocNavVC(void) {
    return allocClass("UINavigationController");
}

void presentVC(id presenter, id child) {
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id presenter, id modal) {
    id container = getObjectWithObject(allocNavVC(),
                                       sel_getUid("initWithRootViewController:"), modal);
    presentVC(presenter, container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter) {
    ((void(*)(id,SEL,bool,id))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, nil);
}

id createAlertController(CFStringRef title, CFStringRef message) {
    return ((id(*)(Class,SEL,CFStringRef,CFStringRef,int))objc_msgSend)
    (objc_getClass("UIAlertController"),
     sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler) {
    id action = ((id(*)(Class,SEL,CFStringRef,int,ObjectBlock))objc_msgSend)
    (objc_getClass("UIAlertAction"), sel_getUid("actionWithTitle:style:handler:"), title, style,
     ^(id action _U_) {
        if (handler)
            handler();
    });
    setObject(ctrl, sel_getUid("addAction:"), action);
}

#pragma mark - View Functions

id createTabController(void) {
    id appearance = getObject(allocClass("UITabBarAppearance"), sel_getUid("init"));
    setBackground(appearance, createColor("systemBackgroundColor"));
    char const *items[] = {
        "stackedLayoutAppearance", "inlineLayoutAppearance", "compactInlineLayoutAppearance"
    };
    CFDictionaryValueCallBacks valueCallbacks = {0};
    const void *keys[] = {(CFStringRef) NSForegroundColorAttributeName};
    id normalColor = createColor("systemGrayColor"), selectedColor = createColor("systemRedColor");
    const void *normalVals[] = {normalColor}, *selectedVals[] = {selectedColor};
    CFDictionaryRef normalDict = CFDictionaryCreate(NULL, keys, normalVals, 1,
                                                    &kCFCopyStringDictionaryKeyCallBacks,
                                                    &valueCallbacks);
    CFDictionaryRef selectedDict = CFDictionaryCreate(NULL, keys, selectedVals, 1,
                                                      &kCFCopyStringDictionaryKeyCallBacks,
                                                      &valueCallbacks);

    for (int i = 0; i < 3; ++i) {
        id item = getObject(appearance, sel_getUid(items[i]));
        id normal = getObject(item, sel_getUid("normal"));
        setObject(normal, sel_getUid("setIconColor:"), normalColor);
        setDict(normal, sel_getUid("setTitleTextAttributes:"), normalDict);
        id selected = getObject(item, sel_getUid("selected"));
        setObject(selected, sel_getUid("setIconColor:"), selectedColor);
        setDict(selected, sel_getUid("setTitleTextAttributes:"), selectedDict);
    }
    id tabVC = getObject(allocClass("UITabBarController"), sel_getUid("init"));
    id bar = getObject(tabVC, sel_getUid("tabBar"));
    setObject(bar, sel_getUid("setStandardAppearance:"), appearance);
    CFRelease(normalDict);
    CFRelease(selectedDict);
    releaseObj(appearance);
    return tabVC;
}
