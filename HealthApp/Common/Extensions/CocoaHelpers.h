#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>
#include "StringUtils.h"

struct SelCache {
    const SEL alo, nw, rel;
    id (*allocate)(Class,SEL);
    id (*newObj)(Class,SEL);
    void (*objRel)(id,SEL);
    void (*viewRel)(id,SEL);
    void (*vcRel)(id,SEL);
};

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

#define alloc(C) Sels.allocate((C), Sels.alo)
#define new(C) Sels.newObj((C), Sels.nw)
#define releaseV(o) Sels.viewRel(o, Sels.rel)
#define releaseO(o) Sels.objRel(o, Sels.rel)
#define releaseVC(o) Sels.vcRel(o, Sels.rel)

#define getImpC(cls, cmd) method_getImplementation(class_getClassMethod((cls), (cmd)))
#define getImpO(cls, cmd) method_getImplementation(class_getInstanceMethod((cls), (cmd)))

#define clsF0(rv, cls, cmd) (((rv(*)(Class,SEL))objc_msgSend)((cls),(cmd)))
#define clsF1(rv, t, cls, cmd, a) (((rv(*)(Class,SEL,t))objc_msgSend)((cls),(cmd),(a)))

#define msg0(rv, o, cmd) (((rv(*)(id,SEL))objc_msgSend)((o),(cmd)))
#define msg1(rv, t, o, cmd, a) (((rv(*)(id,SEL,t))objc_msgSend)((o),(cmd),(a)))
#define msg2(rv, t1, t2, o, cmd, a1, a2) (((rv(*)(id,SEL,t1,t2))objc_msgSend)((o),(cmd),(a1),(a2)))
#define msg3(rv, t1, t2, t3, o, cmd, a1, a2, a3)\
 (((rv(*)(id,SEL,t1,t2,t3))objc_msgSend)((o),(cmd),(a1),(a2),(a3)))

#define msgSup0(rv, s, cmd) (((rv(*)(struct objc_super*,SEL))objc_msgSendSuper)(s, cmd))

typedef void (^Callback)(void);

extern const CFArrayCallBacks retainedArrCallbacks;
extern struct SelCache Sels;
extern Class UIColor;
extern Class UIImage;

#define getImg(name) clsF1(id, CFStringRef, UIImage, sel_getUid("imageNamed:"), (name))
#define sysImg(name) clsF1(id, CFStringRef, UIImage, sel_getUid("systemImageNamed:"), (name))

void initNSData(Class **clsRefs, size_t **sizeRefs);
id getColor(int type);

CFArrayRef createSortDescriptors(CFStringRef key, bool ascending);

#endif /* CocoaBridging_h */
