//
//  CocoaBridging.h
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#ifndef CocoaBridging_h
#define CocoaBridging_h

#define _U_ __attribute__((__unused__))

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <objc/runtime.h>
#include <objc/message.h>

extern CFArrayCallBacks *kCocoaArrCallbacks;

typedef struct {
    CGFloat top, left, bottom, right;
} HAEdgeInsets;

id objc_staticMethod(Class _self, SEL _cmd);
void objc_singleArg(id obj, SEL _cmd);
id createChartEntry(int x, int y);

void getScreenBounds(CGRect *result);
id getUserDefaults(void);
id createColor(const char *name);
id createObjectWithFrame(const char *name, CGRect rect);

id allocClass(const char *name);
void releaseObj(id obj);

id createVCWithDelegate(const char *name, void *delegate);
id getFirstVC(id navVC);
void setupNavVC(id navVC, id firstVC);
id allocNavVC(void);
void presentVC(id presenter, id modal);
void dismissPresentedVC(id presenter);

id createView(id color, bool rounded);
id createStackView(id *subviews, int count, int axis, CGFloat spacing,
                   int distribution, HAEdgeInsets margins);
id createScrollView(void);
id createLabel(CFStringRef text, id style, bool dynamicSize, int alignment);
id createTextfield(id delegate, CFStringRef placeholder, int alignment, int keyboard);
id createButton(CFStringRef title, id color, id disabledColor, id style,
                id background, bool rounded, bool dynamicSize, bool enabled, int tag);
id createSegmentedControl(CFStringRef *items, int count, int startIndex);
void enableButton(id view, bool enabled);
void activateConstraints(id *constraints, int count);

#endif /* CocoaBridging_h */
