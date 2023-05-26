#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>
#include "BaseMacros.h"

#define clsF0(rv, cls, cmd) (((rv(*)(Class,SEL))objc_msgSend)((cls),(cmd)))
#define clsF1(rv, t, cls, cmd, a) (((rv(*)(Class,SEL,t))objc_msgSend)((cls),(cmd),(a)))
#define clsF2(rv, t1, t2, cls, cmd, a1, a2)\
 (((rv(*)(Class,SEL,t1,t2))objc_msgSend)((cls),(cmd),(a1),(a2)))
#define clsF3(rv, t1, t2, t3, cls, cmd, a1, a2, a3)\
 (((rv(*)(Class,SEL,t1,t2,t3))objc_msgSend)((cls),(cmd),(a1),(a2),(a3)))

#define msg0(rv, o, cmd) (((rv(*)(id,SEL))objc_msgSend)((o),(cmd)))
#define msg1(rv, t, o, cmd, a) (((rv(*)(id,SEL,t))objc_msgSend)((o),(cmd),(a)))
#define msg2(rv, t1, t2, o, cmd, a1, a2) (((rv(*)(id,SEL,t1,t2))objc_msgSend)((o),(cmd),(a1),(a2)))
#define msg3(rv, t1, t2, t3, o, cmd, a1, a2, a3)\
 (((rv(*)(id,SEL,t1,t2,t3))objc_msgSend)((o),(cmd),(a1),(a2),(a3)))
#define msg4(rv, t1, t2, t3, t4, o, cmd, a1, a2, a3, a4)\
 (((rv(*)(id,SEL,t1,t2,t3,t4))objc_msgSend)((o),(cmd),(a1),(a2),(a3),(a4)))

#define msgSup0(rv, s, cmd) (((rv(*)(struct objc_super*,SEL))objc_msgSendSuper)(s, cmd))
#define msgSup1(rv, t, s, cmd, a) (((rv(*)(struct objc_super*,SEL,t))objc_msgSendSuper)(s, cmd, a))

#define getClassMethodImp(C, s) method_getImplementation(class_getClassMethod((C), (s)))

enum {
    ColorDiv,
    ColorLabel,
    ColorDisabled,
    ColorGray,
    ColorRed,
    ColorBlue,
    ColorGreen,
    ColorOrange,
    ColorPrimaryBG,
    ColorPrimaryBGGrouped,
    ColorSecondaryBG,
    ColorSecondaryBGGrouped,
    ColorTertiaryBG
};

enum {
    BarColorNav,
    BarColorModal
};

struct AppCache {
    const struct {
        SEL rts;
        SEL sa, sn, ret, rel;
        id (*objAlloc)(Class, SEL);
        id (*objNew)(Class, SEL);
        id (*viewRet)(id, SEL);
        void (*objRel)(id, SEL);
        void (*viewRel)(id, SEL);
        void (*vcRel)(id, SEL);
    } sels;
    const struct ColorCache {
        id (*colorFunc)(int);
        id (*barFunc)(int);
        SEL si;
        id (*init)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat);
        SEL sels[13];
        id (*imps[13])(Class, SEL);
    } color;
    const struct {
        SEL imn;
        id (*named)(Class, SEL, CFStringRef);
    } image;
    const struct {
        Class cls;
        SEL cns, anr;
        id (*current)(Class, SEL);
        void (*add)(id, SEL, id, void(^)(id));
    } unc;
};

typedef void (^Callback)(void);

extern const CFArrayCallBacks RetainedArrCallbacks;
extern struct AppCache AppTable;
extern Class Color;
extern Class Image;

#define ReleaseSel AppTable.sels.rel

#define respondsToSelector(o, s) msg1(bool, SEL, (o), AppTable.sels.rts, (s))
#define alloc(C) AppTable.sels.objAlloc((C), AppTable.sels.sa)
#define new(C) AppTable.sels.objNew((C), AppTable.sels.sn)
#define retainView(v) AppTable.sels.viewRet((v), AppTable.sels.ret)
#define releaseView(v) AppTable.sels.viewRel(v, ReleaseSel)
#define releaseObject(o) AppTable.sels.objRel(o, ReleaseSel)
#define releaseVC(c) AppTable.sels.vcRel(c, ReleaseSel)

#define createColor(r, g, b, a)\
 AppTable.color.init(alloc(Color), AppTable.color.si, (r), (g), (b), (a))
#define getColor(t) AppTable.color.colorFunc((t))
#define getBarColor(t) AppTable.color.barFunc((t))

#define getImage(n) AppTable.image.named(Image, AppTable.image.imn, (n))

#define getNotificationCenter() AppTable.unc.current(AppTable.unc.cls, AppTable.unc.cns)
#define addNotificationRequest(c, r) AppTable.unc.add((c), AppTable.unc.anr, (r), ^(id err _U_){})

void initAppData(bool modern, Class **clsRefs, size_t **sizeRefs);

void setupAppColors(bool darkMode);

void fillStringArray(CFStringRef *arr, CFStringRef format, int count);
CFArrayRef createSortDescriptors(CFStringRef key, bool ascending);

#endif /* CocoaBridging_h */
