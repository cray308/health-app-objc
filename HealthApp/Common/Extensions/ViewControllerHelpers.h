#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaHelpers.h"
#include "Views.h"

#define getRootVC(w) msg0(id,w,sel_getUid("rootViewController"))
#define getViewControllers(v) msg0(CFArrayRef,v,sel_getUid("viewControllers"))

#define createNavVC(c) msg1(id,id,allocClass(DMNavVC),sel_getUid("initWithRootViewController:"),c)
#define getView(v) msg0(id,v,sel_getUid("view"))
#define getNavItem(v) msg0(id,v,sel_getUid("navigationItem"))
#define getNavVC(v) msg0(id,v,sel_getUid("navigationController"))
#define isViewLoaded(v) msg0(bool,v,sel_getUid("isViewLoaded"))

#define setBarTint(v, c) msg1(void,id,v,sel_getUid("setBarTintColor:"),c)

extern Class DMNavVC;
extern Class VCClass;
extern size_t VCSize;

typedef void (^Callback)(void);

void setNavButtons(id vc, id *buttons);
void setupTabVC(id vc);
void setVCTitle(id vc, CFStringRef title CF_CONSUMED);

void presentVC(id child);
void presentModalVC(id modal);
void dismissPresentedVC(Callback handler);

id createAlertController(CFStringRef title CF_CONSUMED, CFStringRef message CF_CONSUMED);
void addAlertAction(id ctrl, CFStringRef title CF_CONSUMED, int style, Callback handler);

#endif /* ViewControllerHelpers_h */
