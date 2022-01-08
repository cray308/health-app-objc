#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaHelpers.h"
#include "Views.h"

#define getViewControllers(_tabVC) getArray(_tabVC, sel_getUid("viewControllers"))

#define getView(_vc) getObject(_vc, sel_getUid("view"))

#define isViewLoaded(_vc) getBool(_vc, sel_getUid("isViewLoaded"))

#define getNavItem(_vc) getObject(_vc, sel_getUid("navigationItem"))

#define getNavBar(_navVC) getObject(_navVC, sel_getUid("navigationBar"))

#define allocNavVC() allocClass(objc_getClass("DMNavVC"))

#define getNavVC(_vc) getObject(_vc, sel_getUid("navigationController"))

#define createVC(_cls) (((id(*)(id,SEL,id,id))objc_msgSend)\
(allocClass((_cls)), sel_getUid("initWithNibName:bundle:"), nil, nil))

#define setBarTint(_v) \
setObject(_v, sel_getUid("setBarTintColor:"), createColor(ColorTertiaryBGGrouped))

extern Class DMTabVC;
extern Class DMNavVC;

typedef void (^Callback)(void);

void setNavButton(id vc, bool left, id button, int totalWidth);
void setVCTitle(id vc, CFStringRef title);
int dmNavVC_getStatusBarStyle(id self, SEL _cmd);
void dmTabVC_updateColors(id self, SEL _cmd);

void setupNavVC(id navVC, id firstVC);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter, Callback handler);

id createAlertController(CFStringRef title, CFStringRef message);
void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler);

#endif /* ViewControllerHelpers_h */
