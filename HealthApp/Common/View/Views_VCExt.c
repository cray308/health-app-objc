#include "Views_VCExt.h"

struct VCCache VCTable;
Class NavVC;

enum {
    AlertControllerStyleAlert = 1
};

static struct PrivVCData {
    const struct {
        SEL setScrollEdge, configureWithOpaqueBackground, setStandard;
    } appear;
    const struct {
        Class cls;
        SEL aa;
        void (*addAction)(id, SEL, id);
    } alert;
    const struct {
        Class cls;
        SEL cr;
        id (*create)(Class, SEL, CFStringRef, long, ObjectBlock);
    } action;
} pvcc;

void initVCData(void) {
    Class AlertVC = objc_getClass("UIAlertController");
    SEL acaa = sel_getUid("addAction:");

    NavVC = objc_getClass("UINavigationController");
    VCTable.si = sel_getUid("initWithRootViewController:");
    VCTable.nb = sel_getUid("navigationBar");
    VCTable.init = (id(*)(id, SEL, id))class_getMethodImplementation(NavVC, VCTable.si);
    VCTable.navBar = (id(*)(id, SEL))class_getMethodImplementation(NavVC, VCTable.nb);

    Class Action = objc_getClass("UIAlertAction");
    SEL cAct = sel_getUid("actionWithTitle:style:handler:");

    memcpy(&pvcc, &(struct PrivVCData){
        {
            sel_getUid("setScrollEdgeAppearance:"),
            sel_getUid("configureWithOpaqueBackground"), sel_getUid("setStandardAppearance:")
        },
        {
            AlertVC, acaa, (void(*)(id, SEL, id))class_getMethodImplementation(AlertVC, acaa)
        },
        {
            Action, cAct,
            (id(*)(Class, SEL, CFStringRef, long, ObjectBlock))getClassMethodImp(Action, cAct)
        }
    }, sizeof(struct PrivVCData));
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
    setIDFormatted(scrollView, CFSTR("scroll_%@"), CFAutorelease(
      CFStringCreateWithCString(NULL, object_getClassName(vc), kCFStringEncodingUTF8)))

    id view = getView(vc);
    setBackgroundColor(view, getColor(backgroundColor));
    useConstraints(scrollView);
    useStackConstraints(vStack);
    setLayoutMargins(vStack, (HAInsets){16, 8, 16, 8});
    addSubview(view, scrollView);
    addSubview(scrollView, vStack);
    pinToSafeArea(scrollView, view);
    pin(vStack, scrollView);
    setActive(makeConstraint(vStack, LayoutAttributeWidth, 0, scrollView, LayoutAttributeWidth, 0));
    releaseView(scrollView);
    releaseView(vStack);
}

#pragma mark - VC Appearance

void setupBar(id bar, Class BarAppearance, id color) {
    id appearance = new(BarAppearance);
    msgV(objSig(void), appearance, pvcc.appear.configureWithOpaqueBackground);
    msgV(objSig(void, id), appearance, SetBackgroundSel, color);
    msgV(objSig(void, id), bar, pvcc.appear.setStandard, appearance);
    if (respondsToSelector(bar, pvcc.appear.setScrollEdge))
        msgV(objSig(void, id), bar, pvcc.appear.setScrollEdge, appearance);
    releaseObject(appearance);
}

#pragma mark - VC Presentation

void presentModalVC(id vc, id modal) {
    id navVC = createNavVC(modal);
    setupBar(getNavBar(navVC), getNavBarAppearanceClass(), getBarColor(CFSTR("modalColor")));
    presentVC(vc, navVC);
    releaseVC(navVC);
    releaseVC(modal);
}

#pragma mark - Alerts

id createAlert(CFStringRef titleKey, CFStringRef messageKey) {
    CFStringRef title = localize(titleKey), message = localize(messageKey);
    id alert = msgV(clsSig(id, CFStringRef, CFStringRef, long), pvcc.alert.cls,
                    sel_getUid("alertControllerWithTitle:message:preferredStyle:"),
                    title, message, AlertControllerStyleAlert);
    CFRelease(title);
    CFRelease(message);
    return alert;
}

void addAlertAction(id alert, CFStringRef titleKey, int style, Callback handler) {
    CFStringRef title = localize(titleKey);
    id action = pvcc.action.create(pvcc.action.cls, pvcc.action.cr, title, style, ^(id act _U_) {
        if (handler) handler();
    });
    pvcc.alert.addAction(alert, pvcc.alert.aa, action);
    CFRelease(title);
}
