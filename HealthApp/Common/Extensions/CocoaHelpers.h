#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFString.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/message.h>

#define _U_ __attribute__((__unused__))

#define staticMethod(_cls, _cmd) (((id(*)(Class,SEL))objc_msgSend)((_cls), (_cmd)))

#define staticMethodWithString(_cls, _cmd, arg) \
(((id(*)(Class,SEL,CFStringRef))objc_msgSend)((_cls), (_cmd), (arg)))

#define voidFunc(obj, _cmd) (((void(*)(id,SEL))objc_msgSend)((obj), (_cmd)))

#define releaseObj(_obj) voidFunc(_obj, sel_getUid("release"))

#define getObject(_obj, _cmd) (((id(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define setObject(_obj, _cmd, _arg) (((void(*)(id,SEL,id))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getBool(_obj, _cmd) (((bool(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define setBool(_obj, _cmd, _arg) (((void(*)(id,SEL,bool))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setString(_obj, _cmd, _arg) (((void(*)(id,SEL,CFStringRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getInt(_obj, _cmd) (((int(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define setInt(_obj, _cmd, _arg) (((void(*)(id,SEL,int))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setCGFloat(_obj, _cmd, _arg) (((void(*)(id,SEL,CGFloat))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setArray(_obj, _cmd, _arg) (((void(*)(id,SEL,CFArrayRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getDict(_obj, _cmd, _arg) (((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getArray(_obj, _cmd) (((CFArrayRef(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define getObjectWithFloat(_obj, _cmd, _arg) (((id(*)(id,SEL,CGFloat))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getObjectWithObject(_obj, _cmd, _arg) (((id(*)(id,SEL,id))objc_msgSend)((_obj), (_cmd), (_arg)))

#define allocClass(_cls) staticMethod(_cls, sel_getUid("alloc"))

#define getColorRef(red, green, blue, alpha) \
(((id(*)(Class,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend) \
(objc_getClass("UIColor"), sel_getUid("colorWithRed:green:blue:alpha:"), (red), (green), (blue), (alpha)))

#define getSystemColor(_name) staticMethod(objc_getClass("UIColor"), sel_getUid((_name)))

#define createImage(_name) staticMethodWithString(objc_getClass("UIImage"), sel_getUid("imageNamed:"), (_name))

#define getBundle() staticMethod(objc_getClass("NSBundle"), sel_getUid("mainBundle"))

#define getUserDefaults() staticMethod(objc_getClass("NSUserDefaults"), sel_getUid("standardUserDefaults"))

#define getDeviceNotificationCenter() staticMethod(objc_getClass("NSNotificationCenter"), sel_getUid("defaultCenter"))

#define getNotificationCenter() staticMethod(objc_getClass("UNUserNotificationCenter"), sel_getUid("currentNotificationCenter"))

extern CFArrayCallBacks retainedArrCallbacks;
extern int osVersion;

enum {
    PrimaryBG,
    SecondaryBG,
    TertiaryBG
};

enum {
    ColorSeparator,
    ColorLabel,
    ColorSecondaryLabel,
    ColorGray,
    ColorRed,
    ColorBlue,
    ColorGreen,
    ColorOrange,
    ColorPrimaryBG,
    ColorPrimaryBGGrouped,
    ColorSecondaryBG,
    ColorSecondaryBGGrouped,
    ColorTertiaryBG,
    ColorTertiaryBGGrouped
};

void getRect(id view, CGRect *result, char type);
void getScreenBounds(CGRect *result);
id createColor(int type);
CFStringRef localize(CFStringRef key);
void fillStringArray(CFStringRef *arr, CFStringRef format, int count);

#endif /* CocoaBridging_h */
