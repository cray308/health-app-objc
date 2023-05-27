#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>
#include "BaseMacros.h"

#define objcArgs(t) t self _U_, SEL _cmd _U_
#define objSig(r, ...) (r(*)(id, SEL, ##__VA_ARGS__))
#define clsSig(r, ...) (r(*)(Class, SEL, ##__VA_ARGS__))
#define msgV(s, o, ...) ((s objc_msgSend) (o, ##__VA_ARGS__))

#define supSig(...) ((void(*)(struct objc_super *, SEL, ##__VA_ARGS__)) objc_msgSendSuper)
#define msgSupV(s, o, C, cmd, ...) (s (&(struct objc_super){o, C}, cmd, ##__VA_ARGS__))

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

#define respondsToSelector(o, s) msgV(objSig(bool, SEL), (o), AppTable.sels.rts, (s))
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
