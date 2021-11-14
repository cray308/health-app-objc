//
//  CocoaHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#ifndef CocoaBridging_h
#define CocoaBridging_h

#define _U_ __attribute__((__unused__))

#include <CoreFoundation/CFArray.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/objc.h>

enum {
    Version12 = 12,
    Version13,
    Version14
};

extern CFArrayCallBacks retainedArrCallbacks;
extern int osVersion;

enum {
    PrimaryBG,
    SecondaryBG,
    TertiaryBG
};

enum {
    ColorLabel,
    ColorSecondaryLabel,
    ColorGray,
    ColorRed,
    ColorBlue,
    ColorGreen,
    ColorOrange
};

id staticMethod(Class _self, SEL _cmd);
id staticMethodWithString(Class _self, SEL _cmd, CFStringRef arg);
void voidFunc(id obj, SEL _cmd);
id getObject(id obj, SEL _cmd);
void setObject(id obj, SEL _cmd, id arg);
bool getBool(id obj, SEL _cmd);
void setBool(id obj, SEL _cmd, bool arg);
void setString(id obj, SEL _cmd, CFStringRef arg);
int getInt(id obj, SEL _cmd);
void setCGFloat(id obj, SEL _cmd, CGFloat arg);
void setArray(id obj, SEL _cmd, CFArrayRef arg);
CFDictionaryRef getDict(id obj, SEL _cmd, CFStringRef arg);
CFArrayRef getArray(id obj, SEL _cmd);

id allocClass(const char *name);
id getObjectWithObject(id obj, SEL _cmd, id arg);
id getObjectWithFloat(id obj, SEL _cmd, CGFloat arg);
void releaseObj(id obj);

void getRect(id view, CGRect *result, char type);
void getScreenBounds(CGRect *result);
id getBundle(void);
id getUserDefaults(void);
id getDeviceNotificationCenter(void);
id getNotificationCenter(void);
id getBackground(int type, bool grouped);
id getColorRef(float red, float green, float blue, float alpha);
id getSystemColor(char const *name);
id createColor(int type);
id createImage(CFStringRef name);
CFStringRef localize(CFStringRef key);
void fillStringArray(CFStringRef *arr, CFStringRef format, int count);
CFStringRef createNumberString(int value);

#endif /* CocoaBridging_h */
