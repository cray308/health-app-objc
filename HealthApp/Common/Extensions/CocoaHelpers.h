#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CFBundle.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/message.h>
#include "ColorCache.h"

#if DEBUG
#include <assert.h>
#define customAssert(x) assert(x);
extern void NSLog(id format, ...);
#else
#define customAssert(x)
#endif

struct SelCache {
    const SEL alo, nw, rel;
    id (*alloc)(Class,SEL);
    id (*new)(Class,SEL);
    void (*objRel)(id,SEL);
    void (*viewRel)(id,SEL);
    void (*vcRel)(id,SEL);
};

#define _U_ __attribute__((__unused__))

#define getImpC(cls, cmd) method_getImplementation(class_getClassMethod((cls), (cmd)))
#define getImpO(cls, cmd) method_getImplementation(class_getInstanceMethod((cls), (cmd)))

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

#define formatStr(l, fmt, ...) CFStringCreateWithFormat(NULL, (CFDictionaryRef)(l), fmt, ##__VA_ARGS__)
#define localize(b, s) CFBundleCopyLocalizedString(b, s, NULL, NULL)
#define createDict(k, v, s, cb)\
 CFDictionaryCreate(NULL, k, v, s, &kCFCopyStringDictionaryKeyCallBacks, cb)

extern const CFArrayCallBacks retainedArrCallbacks;
extern struct SelCache Sels;

enum {
    ColorSeparator,
    ColorLabel,
    ColorSecondaryLabel,
    ColorGray,
    ColorRed,
    ColorBlue,
    ColorGreen,
    ColorOrange,
    ColorPrimaryBG,
    ColorPrimaryBGGrouped,
    ColorSecondaryBG,
    ColorSecondaryBGGrouped,
    ColorTertiaryBG,
};

void fillStringArray(CFBundleRef bundle, CFStringRef *arr, CFStringRef format, int count);
CFArrayRef createSortDescriptors(CFStringRef key, bool ascending);

#endif /* CocoaBridging_h */
