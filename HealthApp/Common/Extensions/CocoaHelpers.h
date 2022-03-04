#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CFBundle.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/message.h>

#if DEBUG
#include <assert.h>
#define customAssert(x) assert(x);
#else
#define customAssert(x)
#endif

#define _U_ __attribute__((__unused__))

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

#define releaseObj(obj) msg0(void,obj,sel_getUid("release"))
#define createNew(cls) clsF0(id,cls,sel_getUid("new"))
#define allocClass(cls) clsF0(id,cls,sel_getUid("alloc"))

#define createColor(t) clsF1(id,int,ColorClass,sel_getUid("getColorWithType:"),t)
#define getBarColor(t) clsF1(id,int,ColorClass,sel_getUid("getBarColorWithType:"),t)

#define createImage(n) clsF1(id,CFStringRef,objc_getClass("UIImage"),sel_getUid("imageNamed:"),n)

#define getUserNotificationCenter()\
 clsF0(id,objc_getClass("UNUserNotificationCenter"),sel_getUid("currentNotificationCenter"))

extern const CFArrayCallBacks retainedArrCallbacks;
extern Class ColorClass;

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
    ColorBarModal = ColorLabel
};
enum {
    RectFrame, RectBounds
};

void getRect(id view, CGRect *result, int type);
void getScreenBounds(CGRect *result);
void fillStringArray(CFBundleRef bundle, CFStringRef *arr, CFStringRef format, int count);
CFArrayRef createSortDescriptors(CFStringRef key, bool ascending);

#endif /* CocoaBridging_h */
