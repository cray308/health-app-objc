//
//  CocoaHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#include "CocoaHelpers.h"
#include "AppUserData.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include <stdlib.h>

extern int getOSVersion(void);

static const void *cocoaArrRetain(CFAllocatorRef allocator _U_, const void *value) {
    voidFunc((id) value, sel_getUid("retain"));
    return value;
}

static void cocoaArrRelease(CFAllocatorRef allocator _U_, const void *value) {
    releaseObj((id) value);
}

static inline id allocColor(float red, float green, float blue, float alpha) {
    return ((id(*)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend)
    (allocClass("UIColor"), sel_getUid("initWithRed:green:blue:alpha:"), red, green, blue, alpha);
}

CFArrayCallBacks retainedArrCallbacks = {0, cocoaArrRetain, cocoaArrRelease, NULL, NULL};
static id **appColors = NULL;
int osVersion;
static const char *const ColorNames[] = {
    "labelColor",
    "secondaryLabelColor",
    "systemGrayColor",
    "systemRedColor",
    "systemBlueColor",
    "systemGreenColor",
    "systemOrangeColor",
};

void handleIOSVersion(void) {
    osVersion = getOSVersion();
    if (osVersion == Version12) {
        appColors = malloc(7 * sizeof(id*));
        for (int i = 0; i < 7; ++i)
            appColors[i] = malloc(2 * sizeof(id));
        appColors[ColorLabel][0] = allocColor(0, 0, 0, 1);
        appColors[ColorLabel][1] = allocColor(1, 1, 1, 1);
        appColors[ColorSecondaryLabel][0] = allocColor(0.24, 0.24, 0.26, 0.6);
        appColors[ColorSecondaryLabel][1] = allocColor(0.92, 0.92, 0.96, 0.6);
        appColors[ColorGray][0] = allocColor(0.56, 0.56, 0.58, 1);
        appColors[ColorGray][1] = allocColor(0.56, 0.56, 0.58, 1);
        appColors[ColorRed][0] = allocColor(1, 0.23, 0.19, 1);
        appColors[ColorRed][1] = allocColor(1, 0.27, 0.23, 1);
        appColors[ColorBlue][0] = allocColor(0, 0.48, 1, 1);
        appColors[ColorBlue][1] = allocColor(0.04, 0.52, 1, 1);
        appColors[ColorGreen][0] = allocColor(0.2, 0.78, 0.35, 1);
        appColors[ColorGreen][1] = allocColor(0.19, 0.82, 0.35, 1);
        appColors[ColorOrange][0] = allocColor(1, 0.58, 0, 1);
        appColors[ColorOrange][1] = allocColor(1, 0.62, 0.04, 1);
    }
}

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

int getInt(id obj, SEL _cmd) {
    return ((int(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

void setCGFloat(id obj, SEL _cmd, CGFloat arg) {
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(obj, _cmd, arg);
}

void setArray(id obj, SEL _cmd, CFArrayRef arg) {
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(obj, _cmd, arg);
}

CFDictionaryRef getDict(id obj, SEL _cmd, CFStringRef arg) {
    return ((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)(obj, _cmd, arg);
}

CFArrayRef getArray(id obj, SEL _cmd) {
    return ((CFArrayRef(*)(id,SEL))objc_msgSend)(obj, _cmd);
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

id getColorRef(float red, float green, float blue, float alpha) {
    return ((id(*)(Class,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend)
    (objc_getClass("UIColor"), sel_getUid("colorWithRed:green:blue:alpha:"), red, green, blue, alpha);
}

id getBackground(int type, bool grouped) {
    if (osVersion >= 13) {
        switch (type) {
            case PrimaryBG: return getSystemColor(grouped ? "systemGroupedBackgroundColor" : "systemBackgroundColor");
            case SecondaryBG: return getSystemColor(grouped ? "secondarySystemGroupedBackgroundColor" : "secondarySystemBackgroundColor");
            default: return getSystemColor("tertiarySystemBackgroundColor");
        }
    }
    switch (type) {
        case PrimaryBG:
            if (userData->darkMode) return getSystemColor("blackColor");
            return grouped ? getColorRef(0.95, 0.95, 0.97, 1) : getSystemColor("whiteColor");
        case SecondaryBG:
            if (userData->darkMode) return getColorRef(0.11, 0.11, 0.12, 1);
            return grouped ? getSystemColor("whiteColor") : getColorRef(0.95, 0.95, 0.97, 1);
        default:
            if (userData->darkMode) return getColorRef(0.17, 0.17, 0.18, 1);
            return grouped ? getColorRef(0.95, 0.95, 0.97, 1) : getSystemColor("whiteColor");
    }
    return nil;
}

id getSystemColor(char const *name) {
    return staticMethod(objc_getClass("UIColor"), sel_getUid(name));
}

id createColor(int type) {
    if (osVersion == Version12) return appColors[type][userData->darkMode];
    return getSystemColor(ColorNames[type]);
}

id createImage(CFStringRef name) {
    return staticMethodWithString(objc_getClass("UIImage"), sel_getUid("imageNamed:"), name);
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
