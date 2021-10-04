//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "TextValidator.h"
#include "ContainerView.h"

void setNavButton(id navItem, bool left, id button, CGFloat totalWidth);

id getFirstVC(id navVC);
void setupNavVC(id navVC, id firstVC);
id allocNavVC(void);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter);

id createAlertController(CFStringRef title, CFStringRef message);
void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler);

id createTabController(void);

#endif /* ViewControllerHelpers_h */
