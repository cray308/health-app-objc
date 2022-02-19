#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFString.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/message.h>

#if DEBUG
#include <assert.h>
#define customAssert(x) assert(x);
#else
#define customAssert(x)
#endif

#define max(a,b) ((a) >= (b) ? (a) : (b))

#define _U_ __attribute__((__unused__))

#define staticMethod(_cls, _cmd) (((id(*)(Class,SEL))objc_msgSend)((_cls), (_cmd)))

#define staticMethodWithString(_cls, _cmd, arg) \
(((id(*)(Class,SEL,CFStringRef))objc_msgSend)((_cls), (_cmd), (arg)))

#define voidFunc(obj, _cmd) (((void(*)(id,SEL))objc_msgSend)((obj), (_cmd)))

#define releaseObj(_obj) voidFunc(_obj, sel_getUid("release"))

#define createNew(_cls) staticMethod(_cls, sel_getUid("new"))

#define getObject(_obj, _cmd) (((id(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define setObject(_obj, _cmd, _arg) (((void(*)(id,SEL,id))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getBool(_obj, _cmd) (((bool(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define setBool(_obj, _cmd, _arg) (((void(*)(id,SEL,bool))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setString(_obj, _cmd, _arg) (((void(*)(id,SEL,CFStringRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getInt(_obj, _cmd) (((long(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define setInt(_obj, _cmd, _arg) (((void(*)(id,SEL,long))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setCGFloat(_obj, _cmd, _arg) (((void(*)(id,SEL,CGFloat))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setFloat(_obj, _cmd, _arg) (((void(*)(id,SEL,float))objc_msgSend)((_obj), (_cmd), (_arg)))

#define setArray(_obj, _cmd, _arg) (((void(*)(id,SEL,CFArrayRef))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getArray(_obj, _cmd) (((CFArrayRef(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define getObjectWithFloat(_obj, _cmd, _arg) (((id(*)(id,SEL,CGFloat))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getObjectWithObject(_obj, _cmd, _arg) (((id(*)(id,SEL,id))objc_msgSend)((_obj), (_cmd), (_arg)))

#define allocClass(_cls) staticMethod(_cls, sel_getUid("alloc"))

#define getColorRef(red, green, blue, alpha) \
(((id(*)(Class,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend) \
(ColorClass, sel_getUid("colorWithRed:green:blue:alpha:"), (red), (green), (blue), (alpha)))

#define createColor(_t) \
(((id(*)(Class,SEL,int))objc_msgSend)(ColorClass, sel_getUid("getColorWithType:"), (_t)))

#define getBarColor(_t) \
(((id(*)(Class,SEL,int))objc_msgSend)(ColorClass, sel_getUid("getBarColorWithType:"), (_t)))

#define createImage(_name) staticMethodWithString(objc_getClass("UIImage"), sel_getUid("imageNamed:"), (_name))

#define getBundle() staticMethod(objc_getClass("NSBundle"), sel_getUid("mainBundle"))

#define getUserDefaults() staticMethod(objc_getClass("NSUserDefaults"), sel_getUid("standardUserDefaults"))

#define getDeviceNotificationCenter() staticMethod(objc_getClass("NSNotificationCenter"), sel_getUid("defaultCenter"))

#define getNotificationCenter() staticMethod(objc_getClass("UNUserNotificationCenter"), sel_getUid("currentNotificationCenter"))

extern const CFArrayCallBacks retainedArrCallbacks;
extern Class ColorClass;

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
    ColorBarModal = ColorLabel
};

id colorCreateLegacy(id self, SEL _cmd, int type);
id colorCreate(id self, SEL _cmd, int type);
id barColorCreateLegacy(id self, SEL _cmd, int type);
id barColorCreate(id self, SEL _cmd, int type);

void setupAppColors(unsigned char darkMode, bool deleteOld);
void getRect(id view, CGRect *result, char type);
void getScreenBounds(CGRect *result);
id createAttribString(CFStringRef text, CFDictionaryRef dict);
CFStringRef localize(CFStringRef key);
void fillStringArray(CFStringRef *arr, CFStringRef format, int count);
CFArrayRef createSortDescriptors(CFStringRef key, bool ascending);

#endif /* CocoaBridging_h */
