#include "Views_VCExt.h"

struct VCCache VCTable;
Class NavVC;

enum {
    AlertControllerStyleAlert = 1
};

static long barStyle;
static struct PrivVCData {
    const struct {
        SEL snsbau;
        void (*setNeedsStatusBarAppearanceUpdate)(id, SEL);
    } nav;
    const struct {
        SEL setTranslucent, stta;
        void (*setTitleTextAttributes)(id, SEL, CFDictionaryRef);
    } bar;
    const struct {
        SEL setScrollEdge, configureWithOpaqueBackground, setStandard;
    } appear;
    const struct {
        Class cls;
        SEL aa;
        void (*addAction)(id, SEL, id);
        id (*createFunc)(CFStringRef, CFStringRef);
        SEL sv;
        void (*setValue)(id, SEL, id, CFStringRef);
    } alert;
    const struct {
        Class cls;
        SEL cr;
        id (*create)(Class, SEL, CFStringRef, long, void(^)(id));
    } action;
} pvcc;

static void setupCharts(bool);
static long getPreferredStatusBarStyle(id, SEL);
static id alertCreate(CFStringRef, CFStringRef);
static id alertCreateLegacy(CFStringRef, CFStringRef);

void initVCData(uint8_t darkMode) {
    Class AlertVC = objc_getClass("UIAlertController");
    SEL acaa = sel_getUid("addAction:");
    SEL acsv = sel_getUid("setValue:forKey:");
    id (*alertCreateFunc)(CFStringRef, CFStringRef) = alertCreate;

    if (isCharValid(darkMode)) {
        barStyle = darkMode;
        alertCreateFunc = alertCreateLegacy;
        setupAppColors(darkMode);
    }

    NavVC = objc_getClass("UINavigationController");
    NavVC = objc_allocateClassPair(NavVC, "DMNavVC", 0);
    class_addMethod(NavVC, sel_getUid("preferredStatusBarStyle"),
                    (IMP)getPreferredStatusBarStyle, "q@:");
    objc_registerClassPair(NavVC);
    VCTable.si = sel_getUid("initWithRootViewController:");
    VCTable.nb = sel_getUid("navigationBar");
    VCTable.init = (id(*)(id, SEL, id))class_getMethodImplementation(NavVC, VCTable.si);
    VCTable.navBar = (id(*)(id, SEL))class_getMethodImplementation(NavVC, VCTable.nb);
    SEL ncsnsbau = sel_getUid("setNeedsStatusBarAppearanceUpdate");

    Class NavBar = objc_getClass("UINavigationBar");
    SEL nbstta = sel_getUid("setTitleTextAttributes:");

    Class Action = objc_getClass("UIAlertAction");
    SEL cAct = sel_getUid("actionWithTitle:style:handler:");

    memcpy(&pvcc, &(struct PrivVCData){
        {ncsnsbau, (void(*)(id, SEL))class_getMethodImplementation(NavVC, ncsnsbau)},
        {
            sel_getUid("setTranslucent:"), nbstta,
            (void(*)(id, SEL, CFDictionaryRef))class_getMethodImplementation(NavBar, nbstta)
        },
        {
            sel_getUid("setScrollEdgeAppearance:"),
            sel_getUid("configureWithOpaqueBackground"), sel_getUid("setStandardAppearance:")
        },
        {
            .createFunc = alertCreateFunc, .sv = acsv, .setValue =
            (void(*)(id, SEL, id, CFStringRef))class_getMethodImplementation(AlertVC, acsv),
            .cls =
            AlertVC, acaa, (void(*)(id, SEL, id))class_getMethodImplementation(AlertVC, acaa)
        },
        {
            Action, cAct,
            (id(*)(Class, SEL, CFStringRef, long, void(^)(id)))getClassMethodImp(Action, cAct)
        }
    }, sizeof(struct PrivVCData));
    setupCharts(darkMode);
}

#pragma mark - VC Setup

void setupNavItem(id vc, CFStringRef titleKey, id const *buttons) {
    id navItem = getNavItem(vc);
    CFStringRef title = localize(titleKey);
    msgV(objSig(void, CFStringRef), navItem, SetTitleSel, title);
    CFRelease(title);
    if (!buttons) return;

    char *setters[] = {"setLeftBarButtonItem:", "setRightBarButtonItem:"};
    for (int i = 0; i < 2; ++i) {
        id btn = buttons[i];
        if (btn) {
            useConstraints(btn);
            id barItem = createBarButtonItemWithView(btn);
            msgV(objSig(void, id), navItem, sel_getUid(setters[i]), barItem);
            releaseObject(barItem);
        }
    }
}

void setupHierarchy(id vc, id vStack, id scrollView, int backgroundColor) {
    id view = getView(vc);
    setBackgroundColor(view, getColor(backgroundColor));
    useConstraints(scrollView);
    useStackConstraints(vStack);
    setLayoutMargins(vStack, (HAInsets){16, 8, 16, 8});
    addSubview(view, scrollView);
    addSubview(scrollView, vStack);
    pinToMainView(scrollView, view);
    pin(vStack, scrollView);
    setActive(makeConstraint(vStack, LayoutAttributeWidth, 0, scrollView, LayoutAttributeWidth, 0));
    releaseView(scrollView);
    releaseView(vStack);
}

#pragma mark - VC Appearance

long getPreferredStatusBarStyle(id self _U_, SEL _cmd _U_) { return barStyle; }

static void setupNavBarColor(id bar) {
    const void *keys[] = {NSForegroundColorAttributeName};
    CFDictionaryRef attrs = CFDictionaryCreate(NULL, keys, (const void *[]){
        getColor(ColorLabel)
    }, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    pvcc.bar.setTitleTextAttributes(bar, pvcc.bar.stta, attrs);
    CFRelease(attrs);
}

void setupBarGeneric(id bar, Class BarAppearance, id color) {
    if (!BarAppearance) {
        msgV(objSig(void, bool), bar, pvcc.bar.setTranslucent, false);
        setBarTintColor(bar, color);
        return;
    }

    id appearance = new(BarAppearance);
    msgV(objSig(void), appearance, pvcc.appear.configureWithOpaqueBackground);
    msgV(objSig(void, id), appearance, SetBackgroundSel, color);
    msgV(objSig(void, id), bar, pvcc.appear.setStandard, appearance);
    if (respondsToSelector(bar, pvcc.appear.setScrollEdge))
        msgV(objSig(void, id), bar, pvcc.appear.setScrollEdge, appearance);
    releaseObject(appearance);
}

void setupTabVC(id vc, Class TabBarAppearance) {
    id tabBar = msgV(objSig(id), vc, sel_getUid("tabBar"));
    id color = getBarColor(BarColorNav);
    setupBarGeneric(tabBar, TabBarAppearance, color);
    if (!TabBarAppearance) {
        id tabBarColor = getColor(ColorGray);
        msgV(objSig(void, id), tabBar, sel_getUid("setUnselectedItemTintColor:"), tabBarColor);
    }

    Class NavBarAppearance = getNavBarAppearanceClass();
    CFArrayRef controllers = msgV(objSig(CFArrayRef), vc, sel_getUid("viewControllers"));
    for (int i = 0; i < 3; ++i) {
        id navVC = (id)CFArrayGetValueAtIndex(controllers, i);
        id navBar = getNavBar(navVC);
        setupBarGeneric(navBar, NavBarAppearance, color);
        if (!TabBarAppearance) {
            setupNavBarColor(navBar);
            pvcc.nav.setNeedsStatusBarAppearanceUpdate(navVC, pvcc.nav.snsbau);
        }
    }
}

void handleTintChange(id window, bool darkMode) {
    barStyle = darkMode;
    setupAppColors(darkMode);
    setTintColor(window, getColor(ColorRed));
    setupTabVC(msgV(objSig(id), window, sel_getUid("rootViewController")), Nil);
    setupCharts(darkMode);
}

void setupCharts(bool enabled) {
    msgV(clsSig(void, bool), objc_getClass("Charts.ChartUtility"), sel_getUid("setup:"), enabled);
}

#pragma mark - VC Presentation

void presentModalVC(id vc, id modal) {
    id navVC = createNavVC(modal);
    id navBar = getNavBar(navVC);
    Class NavBarAppearance = getNavBarAppearanceClass();
    setupBarGeneric(navBar, NavBarAppearance, getBarColor(BarColorModal));
    if (!NavBarAppearance) setupNavBarColor(navBar);
    presentVC(vc, navVC);
    releaseVC(navVC);
    releaseVC(modal);
}

#pragma mark - Alerts

id createAlert(CFStringRef titleKey, CFStringRef messageKey) {
    CFStringRef title = localize(titleKey), message = localize(messageKey);
    id alert = pvcc.alert.createFunc(title, message);
    CFRelease(title);
    CFRelease(message);
    return alert;
}

id alertCreate(CFStringRef title, CFStringRef message) {
    return msgV(clsSig(id, CFStringRef, CFStringRef, long), pvcc.alert.cls,
                sel_getUid("alertControllerWithTitle:message:preferredStyle:"),
                title, message, AlertControllerStyleAlert);
}

id alertCreateLegacy(CFStringRef title, CFStringRef message) {
    id alert = alertCreate(title, message);
    id view = getView(alert);
    for (int i = 0; i < 3; ++i) {
        view = (id)CFArrayGetValueAtIndex(getSubviews(view), 0);
    }
    setBackgroundColor(view, getColor(ColorTertiaryBG));

    id foreground = getColor(ColorLabel);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    CFDictionaryRef titleDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        foreground, getSystemFont(17, UIFontWeightSemibold)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        foreground, getSystemFont(FontSizeSmall, UIFontWeightRegular)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    pvcc.alert.setValue(alert, pvcc.alert.sv, (id)titleString, CFSTR("attributedTitle"));
    pvcc.alert.setValue(alert, pvcc.alert.sv, (id)msgString, CFSTR("attributedMessage"));
    CFRelease(titleDict);
    CFRelease(msgDict);
    CFRelease(titleString);
    CFRelease(msgString);
    return alert;
}

void addAlertAction(id alert, CFStringRef titleKey, int style, Callback handler) {
    CFStringRef title = localize(titleKey);
    id action = pvcc.action.create(pvcc.action.cls, pvcc.action.cr, title, style, ^(id act _U_) {
        if (handler) handler();
        setTintColor(getAppWindow(), getColor(ColorRed));
    });
    pvcc.alert.addAction(alert, pvcc.alert.aa, action);
    CFRelease(title);
}

void disableWindowTint(void) { setTintColor(getAppWindow(), nil); }
