//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaHelpers.h"
#include "ContainerView.h"
#include "TextValidator.h"
#include "Views.h"

#define _cfstr(x) ((__bridge CFStringRef) x)
#define _nsstr(x) ((__bridge NSString*) x)
#define _nsarr(x) ((__bridge NSArray*) x)

extern Class DMTabVC;

typedef void (^Callback)(void);

CFArrayRef getViewControllers(id tabVC);
id getFirstVC(id navVC);
id getView(id vc);

void setNavButton(id navItem, bool left, id button, CGFloat totalWidth);
void dmTabVC_updateColors(id self, SEL _cmd);

void setupNavVC(id navVC, id firstVC);
id allocNavVC(void);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter, Callback handler);

id createAlertController(CFStringRef title, CFStringRef message);
void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler);

#endif /* ViewControllerHelpers_h */
