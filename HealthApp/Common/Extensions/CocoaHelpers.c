#include "CocoaHelpers.h"

struct AppCache AppTable;
Class Image;

static id appColors[13];
static id barColors[2];
static id (*objRetain)(id, SEL);

static id colorCreateLegacy(int type) { return appColors[type]; }

static id barColorCreateLegacy(int type) { return barColors[type]; }

static id colorCreate(int type) {
    return AppTable.color.imps[type](AppTable.color.cls, AppTable.color.sels[type]);
}

static id barColorCreate(int type) {
    CFStringRef names[] = {CFSTR("navBarColor"), CFSTR("modalColor")};
    return msgV(clsSig(id, CFStringRef), AppTable.color.cls, sel_getUid("colorNamed:"), names[type]);
}

void initAppData(bool modern, Class **clsRefs) {
    SEL sa = sel_getUid("alloc"), sn = sel_getUid("new"), ret = sel_getUid("retain");
    SEL rel = sel_getUid("release");
    SEL rts = sel_getUid("respondsToSelector:");
    Class Object = objc_getClass("NSObject"), View = objc_getClass("UIView");
    Class VC = objc_getClass("UIViewController");

    Class Color = objc_getClass("UIColor");
    SEL iColor = sel_getUid("initWithRed:green:blue:alpha:");
    struct ColorCache colorCache = {
        .cls = Color,
        .colorFunc = colorCreate,
        .barFunc = barColorCreate,
        .si = iColor,
        .cwac = sel_getUid("colorWithAlphaComponent:"),
        .init = ((id(*)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat))
                 class_getMethodImplementation(Color, iColor)),
        .sels = {
            sel_getUid("separatorColor"),
            sel_getUid("labelColor"),
            sel_getUid("secondaryLabelColor"),
            sel_getUid("systemGrayColor"),
            sel_getUid("systemRedColor"),
            sel_getUid("systemBlueColor"),
            sel_getUid("systemGreenColor"),
            sel_getUid("systemOrangeColor"),
            sel_getUid("systemBackgroundColor"),
            sel_getUid("systemGroupedBackgroundColor"),
            sel_getUid("secondarySystemBackgroundColor"),
            sel_getUid("secondarySystemGroupedBackgroundColor"),
            sel_getUid("tertiarySystemBackgroundColor")
        }
    };

    for (int i = 0; i < 13; ++i) {
        colorCache.imps[i] = (id(*)(Class, SEL))getClassMethodImp(Color, colorCache.sels[i]);
    }

    if (!modern) {
        colorCache.colorFunc = colorCreateLegacy;
        colorCache.barFunc = barColorCreateLegacy;
    }

    Image = objc_getClass("UIImage");
    SEL iimn = sel_getUid("imageNamed:");

    Class Center = objc_getClass("UNUserNotificationCenter");
    SEL ccns = sel_getUid("currentNotificationCenter");
    SEL canr = sel_getUid("addNotificationRequest:withCompletionHandler:");

    objRetain = (id(*)(id, SEL))class_getMethodImplementation(Object, ret);

    memcpy(&AppTable, &(struct AppCache){
        {
            rts, (bool(*)(id, SEL, SEL))class_getMethodImplementation(Object, rts),
            sa, sn, ret, rel,
            (id(*)(Class, SEL))getClassMethodImp(Object, sa),
            (id(*)(Class, SEL))getClassMethodImp(Object, sn),
            (id(*)(id, SEL))class_getMethodImplementation(View, ret),
            (void(*)(id, SEL))class_getMethodImplementation(Object, rel),
            (void(*)(id, SEL))class_getMethodImplementation(View, rel),
            (void(*)(id, SEL))class_getMethodImplementation(VC, rel)
        },
        colorCache,
        {iimn, (id(*)(Class, SEL, CFStringRef))getClassMethodImp(Image, iimn)},
        {
            Center, ccns, canr,
            (id(*)(Class, SEL))getClassMethodImp(Center, ccns),
            (void(*)(id, SEL, id, ObjectBlock))class_getMethodImplementation(Center, canr)
        }
    }, sizeof(struct AppCache));
    *clsRefs[0] = View;
    *clsRefs[1] = VC;
}

void updateAppColors(bool darkMode) {
    if (appColors[0]) {
        for (int i = 0; i < 13; ++i) {
            releaseObject(appColors[i]);
        }
        releaseObject(barColors[0]);
        releaseObject(barColors[1]);
    }

    if (!darkMode) {
        barColors[BarColorNav] = createColor(0.984f, 0.984f, 0.992f, 1);
        barColors[BarColorModal] = createColor(0.988f, 0.988f, 0.988f, 1);

        appColors[ColorDiv] = createColor(0.24f, 0.24f, 0.26f, 0.29f);
        appColors[ColorLabel] = createColor(0, 0, 0, 1);
        appColors[ColorDisabled] = createColor(0.24f, 0.24f, 0.26f, 0.6f);
        appColors[ColorGray] = createColor(0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed] = createColor(1, 0.23f, 0.19f, 1);
        appColors[ColorBlue] = createColor(0, 0.48f, 1, 1);
        appColors[ColorGreen] = createColor(0.2f, 0.78f, 0.35f, 1);
        appColors[ColorOrange] = createColor(1, 0.58f, 0, 1);

        id primary = createColor(1, 1, 1, 1), grouped = createColor(0.95f, 0.95f, 0.97f, 1);
        appColors[ColorPrimaryBG] = primary;
        appColors[ColorPrimaryBGGrouped] = grouped;
        appColors[ColorSecondaryBG] = objRetain(grouped, AppTable.sels.ret);
        appColors[ColorSecondaryBGGrouped] = objRetain(primary, AppTable.sels.ret);
        appColors[ColorTertiaryBG] = objRetain(primary, AppTable.sels.ret);
    } else {
        barColors[BarColorNav] = createColor(0.075f, 0.075f, 0.075f, 1);
        barColors[BarColorModal] = createColor(0.196f, 0.196f, 0.196f, 1);

        appColors[ColorDiv] = createColor(0.33f, 0.33f, 0.35f, 0.6f);
        appColors[ColorLabel] = createColor(1, 1, 1, 1);
        appColors[ColorDisabled] = createColor(0.92f, 0.92f, 0.96f, 0.6f);
        appColors[ColorGray] = createColor(0.56f, 0.56f, 0.58f, 1);
        appColors[ColorRed] = createColor(1, 0.27f, 0.23f, 1);
        appColors[ColorBlue] = createColor(0.04f, 0.52f, 1, 1);
        appColors[ColorGreen] = createColor(0.19f, 0.82f, 0.35f, 1);
        appColors[ColorOrange] = createColor(1, 0.62f, 0.04f, 1);

        id primary = createColor(0, 0, 0, 1), secondary = createColor(0.11f, 0.11f, 0.12f, 1);
        appColors[ColorPrimaryBG] = primary;
        appColors[ColorPrimaryBGGrouped] = objRetain(primary, AppTable.sels.ret);
        appColors[ColorSecondaryBG] = secondary;
        appColors[ColorSecondaryBGGrouped] = objRetain(secondary, AppTable.sels.ret);
        appColors[ColorTertiaryBG] = createColor(0.17f, 0.17f, 0.18f, 1);
    }
}
