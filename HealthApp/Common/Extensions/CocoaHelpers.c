//
//  CocoaHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#include "CocoaHelpers.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>

extern id UIFontTextStyleTitle1;
extern id UIFontTextStyleTitle2;
extern id UIFontTextStyleTitle3;
extern id UIFontTextStyleHeadline;
extern id UIFontTextStyleSubheadline;
extern id UIFontTextStyleBody;
extern id UIFontTextStyleFootnote;

CFArrayCallBacks kCocoaArrCallbacks = {0};

id staticMethod(Class _self, SEL _cmd) {
    return ((id(*)(Class,SEL))objc_msgSend)(_self, _cmd);
}

id staticMethodWithString(Class _self, SEL _cmd, CFStringRef arg) {
    return ((id(*)(Class,SEL,CFStringRef))objc_msgSend)(_self, _cmd, arg);
}

void voidFunc(id obj, SEL _cmd) {
    ((void(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

id getObject(id obj, SEL _cmd) {
    return ((id(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

void setObject(id obj, SEL _cmd, id arg) {
    ((void(*)(id,SEL,id))objc_msgSend)(obj, _cmd, arg);
}

bool getBool(id obj, SEL _cmd) {
    return ((bool(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

void setBool(id obj, SEL _cmd, bool arg) {
    ((void(*)(id,SEL,bool))objc_msgSend)(obj, _cmd, arg);
}

void setString(id obj, SEL _cmd, CFStringRef arg) {
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(obj, _cmd, arg);
}

void setInt(id obj, SEL _cmd, int arg) {
    ((void(*)(id,SEL,int))objc_msgSend)(obj, _cmd, arg);
}

int getInt(id obj, SEL _cmd) {
    return ((int(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

void setInt16(id obj, SEL _cmd, int16_t arg) {
    ((void(*)(id,SEL,int16_t))objc_msgSend)(obj, _cmd, arg);
}

int16_t getInt16(id obj, SEL _cmd) {
    return ((int16_t(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

void setInt64(id obj, SEL _cmd, int64_t arg) {
    ((void(*)(id,SEL,int64_t))objc_msgSend)(obj, _cmd, arg);
}

int64_t getInt64(id obj, SEL _cmd) {
    return ((int64_t(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

void setFloat(id obj, SEL _cmd, float arg) {
    ((void(*)(id,SEL,float))objc_msgSend)(obj, _cmd, arg);
}

void setCGFloat(id obj, SEL _cmd, CGFloat arg) {
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(obj, _cmd, arg);
}

void setArray(id obj, SEL _cmd, CFArrayRef arg) {
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(obj, _cmd, arg);
}

void setDict(id obj, SEL _cmd, CFDictionaryRef arg) {
    ((void(*)(id,SEL,CFDictionaryRef))objc_msgSend)(obj, _cmd, arg);
}

CFDictionaryRef getDict(id obj, SEL _cmd, CFStringRef arg) {
    return ((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)(obj, _cmd, arg);
}

id allocClass(const char *name) {
    return staticMethod(objc_getClass(name), sel_getUid("alloc"));
}

id getObjectWithFloat(id obj, SEL _cmd, CGFloat arg) {
    return ((id(*)(id,SEL,CGFloat))objc_msgSend)(obj, _cmd, arg);
}

id getObjectWithObject(id obj, SEL _cmd, id arg) {
    return ((id(*)(id,SEL,id))objc_msgSend)(obj, _cmd, arg);
}

id getObjectWithArr(id obj, SEL _cmd, CFArrayRef arg) {
    return ((id(*)(id,SEL,CFArrayRef))objc_msgSend)(obj, _cmd, arg);
}

void releaseObj(id obj) {
    voidFunc(obj, sel_getUid("release"));
}

void getRect(id view, CGRect *result, char type) {
    SEL func = sel_getUid(!type ? "frame" : "bounds");
#if defined(__arm64__)
    *result = ((CGRect(*)(id,SEL))objc_msgSend)(view, func);
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(result, view, func);
#endif
}

void getScreenBounds(CGRect *result) {
    getRect(staticMethod(objc_getClass("UIScreen"), sel_getUid("mainScreen")), result, 1);
}

id getBundle(void) {
    return staticMethod(objc_getClass("NSBundle"), sel_getUid("mainBundle"));
}

id getUserDefaults(void) {
    return staticMethod(objc_getClass("NSUserDefaults"), sel_getUid("standardUserDefaults"));
}

id getDeviceNotificationCenter(void) {
    return staticMethod(objc_getClass("NSNotificationCenter"), sel_getUid("defaultCenter"));
}

id getNotificationCenter(void) {
    return staticMethod(objc_getClass("UNUserNotificationCenter"),
                        sel_getUid("currentNotificationCenter"));
}

id createColor(const char *name) {
    return staticMethod(objc_getClass("UIColor"), sel_getUid(name));
}

id createFont(int style) {
    id fStyle;
    switch (style) {
        case TextFootnote:
            fStyle = UIFontTextStyleFootnote;
            break;
        case TextSubhead:
            fStyle = UIFontTextStyleSubheadline;
            break;
        case TextBody:
            fStyle = UIFontTextStyleBody;
            break;
        case TextHead:
            fStyle = UIFontTextStyleHeadline;
            break;
        case TextTitle1:
            fStyle = UIFontTextStyleTitle1;
            break;
        default:
            fStyle = UIFontTextStyleTitle3;
    }
    return staticMethodWithString(objc_getClass("UIFont"),
                                  sel_getUid("preferredFontForTextStyle:"), (CFStringRef)fStyle);
}

id createImage(CFStringRef name, bool system) {
    SEL method = system ? sel_getUid("systemImageNamed:") : sel_getUid("imageNamed:");
    return staticMethodWithString(objc_getClass("UIImage"), method, name);
}

CFStringRef localize(CFStringRef key) {
    return ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef,CFStringRef))objc_msgSend)
    (getBundle(), sel_getUid("localizedStringForKey:value:table:"), key, NULL, NULL);
}

void fillStringArray(CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, format, i);
        arr[i] = localize(key);
        CFRelease(key);
    }
}

CFStringRef createNumberString(int value) {
    return CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), value);
}
