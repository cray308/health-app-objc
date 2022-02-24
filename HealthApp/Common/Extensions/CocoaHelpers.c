#include "CocoaHelpers.h"
#include <stdlib.h>
#include <string.h>

static const void *cocoaArrRetain(CFAllocatorRef allocator _U_, const void *value) {
    voidFunc((id) value, sel_getUid("retain"));
    return value;
}

static void cocoaArrRelease(CFAllocatorRef allocator _U_, const void *value) {
    releaseObj((id) value);
}

static inline id allocColor(float red, float green, float blue, float alpha) {
    id _obj = allocClass(ColorClass);
    return (((id(*)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend)
            (_obj, sel_getUid("initWithRed:green:blue:alpha:"), red, green, blue, alpha));
}

const CFArrayCallBacks retainedArrCallbacks = {0, cocoaArrRetain, cocoaArrRelease, NULL, NULL};
Class ColorClass;
static id appColors[13];
static id barColors[2];

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

void setupAppColors(unsigned char darkMode, bool deleteOld) {
    if (deleteOld) {
        for (int i = 0; i < 13; ++i) {
            releaseObj(appColors[i]);
        }
        releaseObj(barColors[0]);
        releaseObj(barColors[1]);
    }

    if (!darkMode) {
        barColors[0] = allocColor(0.984f, 0.984f, 0.992f, 1);
        barColors[1] = allocColor(0.988f, 0.988f, 0.988f, 1);

        appColors[ColorSeparator] = allocColor(0.24f, 0.24f, 0.26f, 0.29f);
        appColors[ColorLabel] = allocColor(0, 0, 0, 1);
        appColors[ColorSecondaryLabel] = allocColor(0.24f, 0.24f, 0.26f, 0.6f);
        appColors[ColorGray] = allocColor(0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed] = allocColor(1, 0.23f, 0.19f, 1);
        appColors[ColorBlue] = allocColor(0, 0.48f, 1, 1);
        appColors[ColorGreen] = allocColor(0.2f, 0.78f, 0.35f, 1);
        appColors[ColorOrange] = allocColor(1, 0.58f, 0, 1);

        appColors[ColorPrimaryBG] = allocColor(1, 1, 1, 1);
        appColors[ColorPrimaryBGGrouped] = allocColor(0.95f, 0.95f, 0.97f, 1);
        appColors[ColorSecondaryBG] = allocColor(0.95f, 0.95f, 0.97f, 1);
        appColors[ColorSecondaryBGGrouped] = allocColor(1, 1, 1, 1);
        appColors[ColorTertiaryBG] = allocColor(1, 1, 1, 1);
    } else {
        barColors[0] = allocColor(0.075f, 0.075f, 0.075f, 1);
        barColors[1] = allocColor(0.196f, 0.196f, 0.196f, 1);

        appColors[ColorSeparator] = allocColor(0.33f, 0.33f, 0.35f, 0.6f);
        appColors[ColorLabel] = allocColor(1, 1, 1, 1);
        appColors[ColorSecondaryLabel] = allocColor(0.92f, 0.92f, 0.96f, 0.6f);
        appColors[ColorGray] = allocColor(0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed] = allocColor(1, 0.27f, 0.23f, 1);
        appColors[ColorBlue] = allocColor(0.04f, 0.52f, 1, 1);
        appColors[ColorGreen] = allocColor(0.19f, 0.82f, 0.35f, 1);
        appColors[ColorOrange] = allocColor(1, 0.62f, 0.04f, 1);

        appColors[ColorPrimaryBG] = allocColor(0, 0, 0, 1);
        appColors[ColorPrimaryBGGrouped] = allocColor(0, 0, 0, 1);
        appColors[ColorSecondaryBG] = allocColor(0.11f, 0.11f, 0.12f, 1);
        appColors[ColorSecondaryBGGrouped] = allocColor(0.11f, 0.11f, 0.12f, 1);
        appColors[ColorTertiaryBG] = allocColor(0.17f, 0.17f, 0.18f, 1);
    }
}

id colorCreateLegacy(id self _U_, SEL _cmd _U_, int type) { return appColors[type]; }
id barColorCreateLegacy(id self _U_, SEL _cmd _U_, int type) { return barColors[type]; }

id colorCreate(id self _U_, SEL _cmd _U_, int type) {
    return staticMethod(ColorClass, sel_getUid(ColorNames[type]));
}

id barColorCreate(id self _U_, SEL _cmd _U_, int type) {
    return staticMethodWithString(ColorClass, sel_getUid("colorNamed:"), BarColorNames[type]);
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

void fillStringArray(CFBundleRef bundle, CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, format, i);
        arr[i] = CFBundleCopyLocalizedString(bundle, key, NULL, NULL);
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
