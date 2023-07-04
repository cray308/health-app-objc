#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <objc/message.h>
#include "StringUtils.h"

#define objSig(r, ...) (r(*)(id, SEL, ##__VA_ARGS__))
#define clsSig(r, ...) (r(*)(Class, SEL, ##__VA_ARGS__))
#define msgV(s, o, ...) ((s objc_msgSend) (o, ##__VA_ARGS__))

#define supSig(...) ((void(*)(struct objc_super *, SEL, ##__VA_ARGS__)) objc_msgSendSuper)
#define msgSupV(s, o, C, cmd, ...) (s (&(struct objc_super){o, C}, cmd, ##__VA_ARGS__))

#define getClassMethodImp(C, s) method_getImplementation(class_getClassMethod((C), (s)))

#if defined(__arm64__)
#define generateRectFunctionSignature(name, ...) CGRect (*name)(id, SEL, ##__VA_ARGS__)
#define getRectMethodImplementation(Cls, cmd, ...)\
 (CGRect(*)(id, SEL, ##__VA_ARGS__))class_getMethodImplementation(Cls, cmd)
#define callRectMethod(func, rv, o, cmd, ...) rv = func((o), cmd, ##__VA_ARGS__)
#else
#define generateRectFunctionSignature(name, ...) void (*name)(CGRect *, id, SEL, ##__VA_ARGS__)
#define getRectMethodImplementation(Cls, cmd, ...)\
 (void(*)(CGRect *, id, SEL, ##__VA_ARGS__))class_getMethodImplementation_stret(Cls, cmd)
#define callRectMethod(func, rv, o, cmd, ...) func(&rv, (o), cmd, ##__VA_ARGS__)
#endif

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

typedef void (^Callback)(void);
typedef void (^ObjectBlock)(id);

struct AppCache {
    const struct {
        SEL sa, sn, ret, rel;
        id (*objAlloc)(Class, SEL);
        id (*objNew)(Class, SEL);
        id (*viewRet)(id, SEL);
        void (*objRel)(id, SEL);
        void (*viewRel)(id, SEL);
        void (*vcRel)(id, SEL);
    } sels;
    const struct ColorCache {
        Class cls;
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
        void (*add)(id, SEL, id, ObjectBlock);
    } unc;
};

extern struct AppCache AppTable;
extern Class Image;

#define ReleaseSel AppTable.sels.rel

#define alloc(C) AppTable.sels.objAlloc((C), AppTable.sels.sa)
#define new(C) AppTable.sels.objNew((C), AppTable.sels.sn)
#define retainView(v) AppTable.sels.viewRet((v), AppTable.sels.ret)
#define releaseView(v) AppTable.sels.viewRel(v, ReleaseSel)
#define releaseObject(o) AppTable.sels.objRel(o, ReleaseSel)
#define releaseVC(c) AppTable.sels.vcRel(c, ReleaseSel)

#define getColor(t) AppTable.color.imps[(t)](AppTable.color.cls, AppTable.color.sels[(t)])
#define getBarColor(n)\
 msgV(clsSig(id, CFStringRef), AppTable.color.cls, sel_getUid("colorNamed:"), (n))

#define getImage(n) AppTable.image.named(Image, AppTable.image.imn, (n))

#define getNotificationCenter() AppTable.unc.current(AppTable.unc.cls, AppTable.unc.cns)
#define addNotificationRequest(c, r) AppTable.unc.add((c), AppTable.unc.anr, (r), ^(id err _U_){})

void initAppData(Class **clsRefs);

#endif /* CocoaBridging_h */
