#include "CocoaHelpers.h"
#include <stdlib.h>
#include "AppUserData.h"

extern int getOSVersion(void);

static const void *cocoaArrRetain(CFAllocatorRef allocator _U_, const void *value) {
    voidFunc((id) value, sel_getUid("retain"));
    return value;
}

static void cocoaArrRelease(CFAllocatorRef allocator _U_, const void *value) {
    releaseObj((id) value);
}

static inline id allocColor(float red, float green, float blue, float alpha) {
    id _obj = allocClass(objc_getClass("UIColor"));
    return ((id(*)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend)
    (_obj, sel_getUid("initWithRed:green:blue:alpha:"), red, green, blue, alpha);
}

CFArrayCallBacks retainedArrCallbacks = {0, cocoaArrRetain, cocoaArrRelease, NULL, NULL};
static id **appColors = NULL;
int osVersion;
static const char *const ColorNames[] = {
    "separatorColor",
    "labelColor",
    "secondaryLabelColor",
    "systemGrayColor",
    "systemRedColor",
    "systemBlueColor",
    "systemGreenColor",
    "systemOrangeColor",
    "systemBackgroundColor",
    "systemGroupedBackgroundColor",
    "secondarySystemBackgroundColor",
    "secondarySystemGroupedBackgroundColor",
    "tertiarySystemBackgroundColor"
};

bool handleIOSVersion(bool *setWindowTint, bool *updateNavbar) {
    bool result = ((osVersion = getOSVersion()) < 13);
    *setWindowTint = osVersion < 14;
    *updateNavbar = osVersion > 14;
    if (result) {
        appColors = malloc(14 * sizeof(id*));
        for (int i = 0; i < 14; ++i) {
            appColors[i] = malloc(sizeof(id) << 1);
        }
        appColors[ColorSeparator][0] = allocColor(0.24, 0.24, 0.26, 0.29);
        appColors[ColorSeparator][1] = allocColor(0.33, 0.33, 0.35, 0.6);
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

        appColors[ColorPrimaryBG][0] = appColors[ColorLabel][1];
        appColors[ColorPrimaryBG][1] = appColors[ColorLabel][0];
        appColors[ColorPrimaryBGGrouped][0] = allocColor(0.95, 0.95, 0.97, 1);
        appColors[ColorPrimaryBGGrouped][1] = appColors[ColorPrimaryBG][1];
        appColors[ColorSecondaryBG][0] = appColors[ColorPrimaryBGGrouped][0];
        appColors[ColorSecondaryBG][1] = allocColor(0.11, 0.11, 0.12, 1);
        appColors[ColorSecondaryBGGrouped][0] = appColors[ColorPrimaryBG][0];
        appColors[ColorSecondaryBGGrouped][1] = appColors[ColorSecondaryBG][1];
        appColors[ColorTertiaryBG][0] = appColors[ColorPrimaryBG][0];
        appColors[ColorTertiaryBG][1] = allocColor(0.17, 0.17, 0.18, 1);
        appColors[ColorTertiaryBGGrouped][0] = appColors[ColorPrimaryBGGrouped][0];
        appColors[ColorTertiaryBGGrouped][1] = appColors[ColorTertiaryBG][1];
    }
    return result;
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
    id screen = staticMethod(objc_getClass("UIScreen"), sel_getUid("mainScreen"));
    getRect(screen, result, 1);
}

id createColor(int type) {
    return osVersion > 12 ? staticMethod(objc_getClass("UIColor"), sel_getUid(ColorNames[type]))
    : appColors[type][userData->darkMode];
}

id createAttribString(CFStringRef text, CFDictionaryRef dict) {
    id _obj = allocClass(objc_getClass("NSAttributedString"));
    return ((id(*)(id,SEL,CFStringRef,CFDictionaryRef))objc_msgSend)
    (_obj, sel_getUid("initWithString:attributes:"), text, dict);
}

CFStringRef localize(CFStringRef key) {
    id bundle = getBundle();
    return ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef,CFStringRef))objc_msgSend)
    (bundle, sel_getUid("localizedStringForKey:value:table:"), key, NULL, NULL);
}

void fillStringArray(CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, format, i);
        arr[i] = localize(key);
        CFRelease(key);
    }
}
