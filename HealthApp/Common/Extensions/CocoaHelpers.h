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

extern CFArrayCallBacks retainedArrCallbacks;

enum {
    TextFootnote = 1,
    TextSubhead,
    TextBody,
    TextHead,
    TextTitle1,
    TextTitle3
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
void setInt(id obj, SEL _cmd, int arg);
void setInt16(id obj, SEL _cmd, int16_t arg);
void setInt64(id obj, SEL _cmd, int64_t arg);
int16_t getInt16(id obj, SEL _cmd);
int64_t getInt64(id obj, SEL _cmd);
void setFloat(id obj, SEL _cmd, float arg);
void setCGFloat(id obj, SEL _cmd, CGFloat arg);
void setArray(id obj, SEL _cmd, CFArrayRef arg);
void setDict(id obj, SEL _cmd, CFDictionaryRef arg);
CFDictionaryRef getDict(id obj, SEL _cmd, CFStringRef arg);

id allocClass(const char *name);
id getObjectWithObject(id obj, SEL _cmd, id arg);
id getObjectWithFloat(id obj, SEL _cmd, CGFloat arg);
id getObjectWithArr(id obj, SEL _cmd, CFArrayRef arg);
void releaseObj(id obj);

void getRect(id view, CGRect *result, char type);
void getScreenBounds(CGRect *result);
id getBundle(void);
id getUserDefaults(void);
id getDeviceNotificationCenter(void);
id getNotificationCenter(void);
id createColor(const char *name);
id createFont(int style);
id createImage(CFStringRef name, bool system);
CFStringRef localize(CFStringRef key);
void fillStringArray(CFStringRef *arr, CFStringRef format, int count);
CFStringRef createNumberString(int value);

#endif /* CocoaBridging_h */
