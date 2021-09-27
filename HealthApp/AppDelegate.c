//
//  AppDelegate.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppDelegate.h"
#include "AppCoordinator.h"
#include "PersistenceService.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"
#include "SwiftBridging.h"

static CFStringRef const hasLaunchedKey = CFSTR("hasLaunched");

Class AppDelegateClass;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = createObjectWithFrame("UIWindow", bounds);

    bool hasLaunched = ((bool(*)(id,SEL,CFStringRef))objc_msgSend)
    (getUserDefaults(), sel_getUid("boolForKey:"), hasLaunchedKey);

    persistenceService_init();
    int tzOffset = 0;

    if (!hasLaunched) {
        ((void(*)(id,SEL,bool,CFStringRef))objc_msgSend)
        (getUserDefaults(), sel_getUid("setBool:forKey:"), true, hasLaunchedKey);
        userInfo_create();
        persistenceService_create();
    } else {
        tzOffset = userInfo_initFromStorage();
    }

    id appearance = ((id(*)(id,SEL))objc_msgSend)(allocClass("UITabBarAppearance"),
                                                  sel_getUid("init"));
    setBackground(appearance, createColor("systemBackgroundColor"));
    setTabBarItemColors(((id(*)(id,SEL))objc_msgSend)(appearance,
                                                      sel_getUid("stackedLayoutAppearance")));
    setTabBarItemColors(((id(*)(id,SEL))objc_msgSend)(appearance,
                                                      sel_getUid("inlineLayoutAppearance")));
    setTabBarItemColors(((id(*)(id,SEL))objc_msgSend)(appearance,
                                                      sel_getUid("compactInlineLayoutAppearance")));

    id tabVC = ((id(*)(id,SEL))objc_msgSend)(allocClass("UITabBarController"), sel_getUid("init"));
    id bar = ((id(*)(id,SEL))objc_msgSend)(tabVC, sel_getUid("tabBar"));
    ((void(*)(id,SEL,id))objc_msgSend)(bar, sel_getUid("setStandardAppearance:"), appearance);
    appCoordinator_start(tabVC);
    ((void(*)(id,SEL,id))objc_msgSend)(self->window, sel_getUid("setRootViewController:"), tabVC);
    objc_singleArg(self->window, sel_getUid("makeKeyAndVisible"));

    if (!hasLaunched) {
        ((void(*)(id,SEL,int,void(^)(BOOL,id)))objc_msgSend)
        (getNotificationCenter(), sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
         6, ^(BOOL granted _U_, id error _U_) {});
    }
    releaseObj(appearance);
    releaseObj(tabVC);

    persistenceService_start(tzOffset);
    appCoordinator_fetchHistory();
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_,
                                      id application _U_, id window _U_) {
    id device = objc_staticMethod(objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    int idiom = ((int(*)(id,SEL))objc_msgSend)(device, sel_getUid("userInterfaceIdiom"));
    return idiom == 1 ? 26 : 2;
}
