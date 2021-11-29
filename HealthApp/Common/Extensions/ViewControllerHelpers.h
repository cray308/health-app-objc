#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaHelpers.h"
#include "Views.h"

#define _cfstr(x) ((__bridge CFStringRef) x)
#define _nsstr(x) ((__bridge NSString*) x)
#define _nsarr(x) ((__bridge NSArray*) x)

#define getViewControllers(_tabVC) getArray(_tabVC, sel_getUid("viewControllers"))

#define getView(_vc) getObject(_vc, sel_getUid("view"))

#define getNavItem(_vc) getObject(_vc, sel_getUid("navigationItem"))

#define allocNavVC() allocClass(objc_getClass("UINavigationController"))

#define createVC(_cls) (((id(*)(id,SEL,id,id))objc_msgSend)\
(allocClass((_cls)), sel_getUid("initWithNibName:bundle:"), nil, nil))

extern Class DMTabVC;

typedef void (^Callback)(void);

id getFirstVC(id navVC);

void setNavButton(id vc, bool left, id button, int totalWidth);
void setVCTitle(id vc, CFStringRef title);
void dmTabVC_updateColors(id self, SEL _cmd);

void setupNavVC(id navVC, id firstVC);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter, Callback handler);

id createAlertController(CFStringRef title, CFStringRef message);
void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler);

#endif /* ViewControllerHelpers_h */
