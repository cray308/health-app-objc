//
//  CocoaHelpers.h
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

extern CFArrayCallBacks kCocoaArrCallbacks;
typedef unsigned char uchar;
typedef signed char schar;
typedef unsigned int uint;

typedef void (^CallbackBlock)(void);

id objc_staticMethod(Class _self, SEL _cmd);
void objc_singleArg(id obj, SEL _cmd);
id allocClass(const char *name);
void releaseObj(id obj);

void getViewBounds(id view, CGRect *bounds);
void getViewFrame(id view, CGRect *frame);
void getScreenBounds(CGRect *result);
id getBundle(void);
id getUserDefaults(void);
id getNotificationCenter(void);
id createColor(const char *name);
id createImage(CFStringRef name);
CFStringRef localize(CFStringRef key);

#endif /* CocoaBridging_h */
