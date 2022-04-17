#include "CocoaHelpers.h"
#include <string.h>

static const void *cocoaRetain(CFAllocatorRef alo _U_, const void *value) {
    msg0(id, (id)value, sel_getUid("retain"));
    return value;
}

static void cocoaRel(CFAllocatorRef alo _U_, const void *value) { Sels.objRel((id)value, Sels.rel); }

const CFArrayCallBacks retainedArrCallbacks = {0, cocoaRetain, cocoaRel, NULL, NULL};
struct SelCache Sels;

static id appColors[13];
static id barColors[2];
static SEL scArr[13];
static id (*modernImps[13])(Class,SEL);
static id (*modernBarImp)(Class,SEL,CFStringRef);
static SEL sbcInt;

id colorCreateLegacy(id self _U_, SEL _cmd _U_, int type) { return appColors[type]; }
id barColorCreateLegacy(id self _U_, SEL _cmd _U_, int type) { return barColors[type]; }

id colorCreate(id self, SEL _cmd _U_, int type) { return modernImps[type]((Class)self, scArr[type]); }

id barColorCreate(id self, SEL _cmd _U_, int type) {
    static CFStringRef const names[] = {CFSTR("navBarColor"), CFSTR("modalColor")};
    return modernBarImp((Class)self, sbcInt, names[type]);
}

void initNSData(bool modern, ColorCache *r, Class **clsRefs, size_t **sizeRefs) {
    SEL alo = sel_getUid("alloc"), nw = sel_getUid("new"), rel = sel_getUid("release");
    Class Object = objc_getClass("NSObject"), View = objc_getClass("UIView");
    Class VC = objc_getClass("UIViewController");
    memcpy(&Sels, &(struct SelCache){alo, nw, rel, (id(*)(Class,SEL))getImpC(Object, alo),
        (id(*)(Class,SEL))getImpC(Object, nw), (void(*)(id,SEL))getImpO(Object, rel),
        (void(*)(id,SEL))getImpO(View, rel), (void(*)(id,SEL))getImpO(VC, rel)
    }, sizeof(struct SelCache));
    *clsRefs[0] = View; *clsRefs[1] = VC;
    *sizeRefs[0] = class_getInstanceSize(View); *sizeRefs[1] = class_getInstanceSize(VC);

    Class Color = objc_getClass("UIColor"), colorMeta = objc_getMetaClass("UIColor");
    SEL sc = sel_registerName("getColorWithType:");
    IMP colorImp = (IMP)colorCreate, barImp = (IMP)barColorCreate;
    sbcInt = sel_getUid("colorNamed:");
    scArr[0] = sel_getUid("separatorColor");
    scArr[1] = sel_getUid("labelColor");
    scArr[2] = sel_getUid("secondaryLabelColor");
    scArr[3] = sel_getUid("systemGrayColor");
    scArr[4] = sel_getUid("systemRedColor");
    scArr[5] = sel_getUid("systemBlueColor");
    scArr[6] = sel_getUid("systemGreenColor");
    scArr[7] = sel_getUid("systemOrangeColor");
    scArr[8] = sel_getUid("systemBackgroundColor");
    scArr[9] = sel_getUid("systemGroupedBackgroundColor");
    scArr[10] = sel_getUid("secondarySystemBackgroundColor");
    scArr[11] = sel_getUid("secondarySystemGroupedBackgroundColor");
    scArr[12] = sel_getUid("tertiarySystemBackgroundColor");
    if (modern) {
        modernBarImp = (id(*)(Class,SEL,CFStringRef))getImpC(Color, sbcInt);
        for (int i = 0; i < 13; ++i) {
            modernImps[i] = (id(*)(Class,SEL))getImpC(Color, scArr[i]);
        }
    } else {
        colorImp = (IMP)colorCreateLegacy; barImp = (IMP)barColorCreateLegacy;
    }
    class_addMethod(colorMeta, sc, colorImp, "@@:i");
    class_addMethod(colorMeta, sel_registerName("getBarColorWithType:"), barImp, "@@:i");
    memcpy(r, &(ColorCache){Color, sc, (id(*)(Class,SEL,int))getImpC(Color, sc)}, sizeof(ColorCache));
}

void setupAppColors(Class Color, unsigned char darkMode, bool deleteOld) {
    if (deleteOld) {
        for (int i = 0; i < 13; ++i) {
            Sels.objRel(appColors[i], Sels.rel);
        }
        Sels.objRel(barColors[0], Sels.rel);
        Sels.objRel(barColors[1], Sels.rel);
    }

    SEL sel = sel_getUid("initWithRed:green:blue:alpha:"), retain = sel_getUid("retain");
    id (*init)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat) =
      (id(*)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat))getImpO(Color, sel);
    if (!darkMode) {
        barColors[0] = init(Sels.alloc(Color, Sels.alo), sel, 0.984f, 0.984f, 0.992f, 1);
        barColors[1] = init(Sels.alloc(Color, Sels.alo), sel, 0.988f, 0.988f, 0.988f, 1);

        appColors[ColorDiv] = init(Sels.alloc(Color, Sels.alo), sel, 0.24f, 0.24f, 0.26f, 0.29f);
        appColors[ColorLabel] = init(Sels.alloc(Color, Sels.alo), sel, 0, 0, 0, 1);
        appColors[ColorDisabled] = init(Sels.alloc(Color, Sels.alo), sel, 0.24f, 0.24f, 0.26f, 0.6f);
        appColors[ColorGray] = init(Sels.alloc(Color, Sels.alo), sel, 0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed] = init(Sels.alloc(Color, Sels.alo), sel, 1, 0.23f, 0.19f, 1);
        appColors[ColorBlue] = init(Sels.alloc(Color, Sels.alo), sel, 0, 0.48f, 1, 1);
        appColors[ColorGreen] = init(Sels.alloc(Color, Sels.alo), sel, 0.2f, 0.78f, 0.35f, 1);
        appColors[ColorOrange] = init(Sels.alloc(Color, Sels.alo), sel, 1, 0.58f, 0, 1);

        id primary = init(Sels.alloc(Color, Sels.alo), sel, 1, 1, 1, 1);
        appColors[ColorPrimaryBG] = primary;
        id grouped = init(Sels.alloc(Color, Sels.alo), sel, 0.95f, 0.95f, 0.97f, 1);
        appColors[ColorPrimaryBGGrouped] = grouped;
        appColors[ColorSecondaryBG] = msg0(id, grouped, retain);
        appColors[ColorSecondaryBGGrouped] = msg0(id, primary, retain);
        appColors[ColorTertiaryBG] = msg0(id, primary, retain);
    } else {
        barColors[0] = init(Sels.alloc(Color, Sels.alo), sel, 0.075f, 0.075f, 0.075f, 1);
        barColors[1] = init(Sels.alloc(Color, Sels.alo), sel, 0.196f, 0.196f, 0.196f, 1);

        appColors[ColorDiv] = init(Sels.alloc(Color, Sels.alo), sel, 0.33f, 0.33f, 0.35f, 0.6f);
        appColors[ColorLabel] = init(Sels.alloc(Color, Sels.alo), sel, 1, 1, 1, 1);
        appColors[ColorDisabled] = init(Sels.alloc(Color, Sels.alo), sel, 0.92f, 0.92f, 0.96f, 0.6f);
        appColors[ColorGray] = init(Sels.alloc(Color, Sels.alo), sel, 0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed] = init(Sels.alloc(Color, Sels.alo), sel, 1, 0.27f, 0.23f, 1);
        appColors[ColorBlue] = init(Sels.alloc(Color, Sels.alo), sel, 0.04f, 0.52f, 1, 1);
        appColors[ColorGreen] = init(Sels.alloc(Color, Sels.alo), sel, 0.19f, 0.82f, 0.35f, 1);
        appColors[ColorOrange] = init(Sels.alloc(Color, Sels.alo), sel, 1, 0.62f, 0.04f, 1);

        id primary = init(Sels.alloc(Color, Sels.alo), sel, 0, 0, 0, 1);
        appColors[ColorPrimaryBG] = primary;
        appColors[ColorPrimaryBGGrouped] = msg0(id, primary, retain);
        id secondary = init(Sels.alloc(Color, Sels.alo), sel, 0.11f, 0.11f, 0.12f, 1);
        appColors[ColorSecondaryBG] = secondary;
        appColors[ColorSecondaryBGGrouped] = msg0(id, secondary, retain);
        appColors[ColorTertiaryBG] = init(Sels.alloc(Color, Sels.alo), sel, 0.17f, 0.17f, 0.18f, 1);
    }
}

void fillStringArray(CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = formatStr(NULL, format, i);
        arr[i] = localize(key);
        CFRelease(key);
    }
}

CFArrayRef createSortDescriptors(CFStringRef key, bool ascending) {
    id desc = msg2(id, CFStringRef, bool, Sels.alloc(objc_getClass("NSSortDescriptor"), Sels.alo),
                   sel_getUid("initWithKey:ascending:"), key, ascending);
    CFArrayRef arr = CFArrayCreate(NULL, (const void *[]){desc}, 1, &retainedArrCallbacks);
    Sels.objRel(desc, Sels.rel);
    return arr;
}
