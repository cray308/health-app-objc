#include "CocoaHelpers.h"
#include <stdlib.h>
#include <string.h>
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
    return (((id(*)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend)
            (_obj, sel_getUid("initWithRed:green:blue:alpha:"), red, green, blue, alpha));
}

CFArrayCallBacks retainedArrCallbacks = {0, cocoaArrRetain, cocoaArrRelease, NULL, NULL};
static id **appColors = NULL;
static id **barColors = NULL;
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
static CFStringRef const BarColorNames[] = {CFSTR("navBarColor"), CFSTR("modalColor")};

bool handleIOSVersion(void) {
    bool result = ((osVersion = getOSVersion()) < 13);
    if (result) {
        barColors = malloc(sizeof(id*) << 1);
        barColors[0] = malloc(sizeof(id) << 1);
        barColors[1] = malloc(sizeof(id) << 1);
        barColors[0][0] = allocColor(0.984f, 0.984f, 0.992f, 1);
        barColors[0][1] = allocColor(0.075f, 0.075f, 0.075f, 1);
        barColors[1][0] = allocColor(0.988f, 0.988f, 0.988f, 1);
        barColors[1][1] = allocColor(0.196f, 0.196f, 0.196f, 1);

        appColors = malloc(13 * sizeof(id*));
        for (int i = 0; i < 13; ++i) {
            appColors[i] = malloc(sizeof(id) << 1);
        }
        appColors[ColorSeparator][0] = allocColor(0.24f, 0.24f, 0.26f, 0.29f);
        appColors[ColorSeparator][1] = allocColor(0.33f, 0.33f, 0.35f, 0.6f);
        appColors[ColorLabel][0] = allocColor(0, 0, 0, 1);
        appColors[ColorLabel][1] = allocColor(1, 1, 1, 1);
        appColors[ColorSecondaryLabel][0] = allocColor(0.24f, 0.24f, 0.26f, 0.6f);
        appColors[ColorSecondaryLabel][1] = allocColor(0.92f, 0.92f, 0.96f, 0.6f);
        appColors[ColorGray][0] = allocColor(0.56f, 0.56f, 0.58f, 1);
        appColors[ColorGray][1] = allocColor(0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed][0] = allocColor(1, 0.23f, 0.19f, 1);
        appColors[ColorRed][1] = allocColor(1, 0.27f, 0.23f, 1);
        appColors[ColorBlue][0] = allocColor(0, 0.48f, 1, 1);
        appColors[ColorBlue][1] = allocColor(0.04f, 0.52f, 1, 1);
        appColors[ColorGreen][0] = allocColor(0.2f, 0.78f, 0.35f, 1);
        appColors[ColorGreen][1] = allocColor(0.19f, 0.82f, 0.35f, 1);
        appColors[ColorOrange][0] = allocColor(1, 0.58f, 0, 1);
        appColors[ColorOrange][1] = allocColor(1, 0.62f, 0.04f, 1);

        appColors[ColorPrimaryBG][0] = appColors[ColorLabel][1];
        appColors[ColorPrimaryBG][1] = appColors[ColorLabel][0];
        appColors[ColorPrimaryBGGrouped][0] = allocColor(0.95f, 0.95f, 0.97f, 1);
        appColors[ColorPrimaryBGGrouped][1] = appColors[ColorPrimaryBG][1];
        appColors[ColorSecondaryBG][0] = appColors[ColorPrimaryBGGrouped][0];
        appColors[ColorSecondaryBG][1] = allocColor(0.11f, 0.11f, 0.12f, 1);
        appColors[ColorSecondaryBGGrouped][0] = appColors[ColorPrimaryBG][0];
        appColors[ColorSecondaryBGGrouped][1] = appColors[ColorSecondaryBG][1];
        appColors[ColorTertiaryBG][0] = appColors[ColorPrimaryBG][0];
        appColors[ColorTertiaryBG][1] = allocColor(0.17f, 0.17f, 0.18f, 1);
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
    return (osVersion > 12 ? staticMethod(objc_getClass("UIColor"), sel_getUid(ColorNames[type]))
            : appColors[type][userData->darkMode]);
}

id getBarColor(int type) {
    if (osVersion > 12) {
        return staticMethodWithString(objc_getClass("UIColor"),
                                      sel_getUid("colorNamed:"), BarColorNames[type]);
    }
    return barColors[type][userData->darkMode];
}

id createAttribString(CFStringRef text, CFDictionaryRef dict) {
    id _obj = allocClass(objc_getClass("NSAttributedString"));
    return (((id(*)(id,SEL,CFStringRef,CFDictionaryRef))objc_msgSend)
            (_obj, sel_getUid("initWithString:attributes:"), text, dict));
}

CFStringRef localize(CFStringRef key) {
    id bundle = getBundle();
    return (((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef,CFStringRef))objc_msgSend)
            (bundle, sel_getUid("localizedStringForKey:value:table:"), key, NULL, NULL));
}

void fillStringArray(CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, format, i);
        arr[i] = localize(key);
        CFRelease(key);
    }
}

CFArrayRef createSortDescriptors(CFStringRef key, bool ascending) {
    id _obj = allocClass(objc_getClass("NSSortDescriptor"));
    id descriptor = (((id(*)(id,SEL,CFStringRef,bool))objc_msgSend)
                     (_obj, sel_getUid("initWithKey:ascending:"), key, ascending));
    CFArrayRef arr = CFArrayCreate(NULL, (const void *[]){descriptor}, 1, &retainedArrCallbacks);
    releaseObj(descriptor);
    return arr;
}
