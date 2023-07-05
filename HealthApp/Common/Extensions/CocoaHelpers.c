#include "CocoaHelpers.h"

struct AppCache AppTable;
Class Image;

void initAppData(Class **clsRefs) {
    SEL sa = sel_getUid("alloc"), sn = sel_getUid("new"), ret = sel_getUid("retain");
    SEL rel = sel_getUid("release");
    SEL rts = sel_getUid("respondsToSelector:");
    Class Object = objc_getClass("NSObject"), View = objc_getClass("UIView");
    Class VC = objc_getClass("UIViewController");

    Class Color = objc_getClass("UIColor");
    struct ColorCache colorCache = {
        .cls = Color,
        .cwac = sel_getUid("colorWithAlphaComponent:"),
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

    Image = objc_getClass("UIImage");
    SEL iimn = sel_getUid("systemImageNamed:");

    Class Center = objc_getClass("UNUserNotificationCenter");
    SEL ccns = sel_getUid("currentNotificationCenter");
    SEL canr = sel_getUid("addNotificationRequest:withCompletionHandler:");

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
