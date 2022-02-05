#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaHelpers.h"
#include "Views.h"

#define getViewControllers(_tabVC) getArray(_tabVC, sel_getUid("viewControllers"))

#define getView(_vc) getObject(_vc, sel_getUid("view"))

#define isViewLoaded(_vc) getBool(_vc, sel_getUid("isViewLoaded"))

#define getNavItem(_vc) getObject(_vc, sel_getUid("navigationItem"))

#define getNavBar(_navVC) getObject(_navVC, sel_getUid("navigationBar"))

#define getTabBar(_tabVC) getObject(_tabVC, sel_getUid("tabBar"))

#define getNavVC(_vc) getObject(_vc, sel_getUid("navigationController"))

#define createVC(_cls) (((id(*)(Class,SEL))objc_msgSend)((_cls), sel_getUid("new")))

#define setBarTint(_v, _c) setObject(_v, sel_getUid("setBarTintColor:"), _c)

extern Class DMNavVC;

typedef void (^Callback)(void);

void setNavButton(id vc, bool left, id button, int totalWidth);
void setupTabVC(id vc);
void setupNavBar(id vc, bool modal);
void setVCTitle(id vc, CFStringRef title);
int dmNavVC_getStatusBarStyle(id self, SEL _cmd);

id createNavVC(id child);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter, Callback handler);

id createAlertController(CFStringRef title, CFStringRef message);
void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler);

#endif /* ViewControllerHelpers_h */
