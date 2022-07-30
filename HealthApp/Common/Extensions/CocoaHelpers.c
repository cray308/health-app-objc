#include "CocoaHelpers.h"

static const void *cocoaRetain(CFAllocatorRef alo _U_, const void *value) {
    return msg0(id, (id)value, sel_getUid("retain"));
}

static void cocoaRel(CFAllocatorRef alo _U_, const void *value) { releaseO((id)value); }

const CFArrayCallBacks retainedArrCallbacks = {0, cocoaRetain, cocoaRel, NULL, NULL};
struct SelCache Sels;
Class UIColor;
Class UIImage;

static SEL scArr[13];
static id (*modernImps[13])(Class,SEL);

void initNSData(Class **clsRefs, size_t **sizeRefs) {
    SEL alo = sel_getUid("alloc"), nw = sel_getUid("new"), rel = sel_getUid("release");
    Class Object = objc_getClass("NSObject"), View = objc_getClass("UIView");
    Class VC = objc_getClass("UIViewController");
    memcpy(&Sels, &(struct SelCache){alo, nw, rel, (id(*)(Class,SEL))getImpC(Object, alo),
        (id(*)(Class,SEL))getImpC(Object, nw), (void(*)(id,SEL))getImpO(Object, rel),
        (void(*)(id,SEL))getImpO(View, rel), (void(*)(id,SEL))getImpO(VC, rel)
    }, sizeof(struct SelCache));
    *clsRefs[0] = View;
    *clsRefs[1] = VC;
    *sizeRefs[0] = class_getInstanceSize(View);
    *sizeRefs[1] = class_getInstanceSize(VC);

    UIImage = objc_getClass("UIImage");
    UIColor = objc_getClass("UIColor");
    scArr[ColorDiv] = sel_getUid("separatorColor");
    scArr[ColorLabel] = sel_getUid("labelColor");
    scArr[ColorDisabled] = sel_getUid("secondaryLabelColor");
    scArr[ColorGray] = sel_getUid("systemGrayColor");
    scArr[ColorRed] = sel_getUid("systemRedColor");
    scArr[ColorBlue] = sel_getUid("systemBlueColor");
    scArr[ColorGreen] = sel_getUid("systemGreenColor");
    scArr[ColorOrange] = sel_getUid("systemOrangeColor");
    scArr[ColorPrimaryBG] = sel_getUid("systemBackgroundColor");
    scArr[ColorPrimaryBGGrouped] = sel_getUid("systemGroupedBackgroundColor");
    scArr[ColorSecondaryBG] = sel_getUid("secondarySystemBackgroundColor");
    scArr[ColorSecondaryBGGrouped] = sel_getUid("secondarySystemGroupedBackgroundColor");
    scArr[ColorTertiaryBG] = sel_getUid("tertiarySystemBackgroundColor");
    for (int i = 0; i < 13; ++i) {
        modernImps[i] = (id(*)(Class,SEL))getImpC(UIColor, scArr[i]);
    }
}

id getColor(int type) { return modernImps[type](UIColor, scArr[type]); }

CFArrayRef createSortDescriptors(CFStringRef key, bool ascending) {
    id _d = alloc(objc_getClass("NSSortDescriptor"));
    id desc = msg2(id, CFStringRef, bool, _d, sel_getUid("initWithKey:ascending:"), key, ascending);
    CFArrayRef arr = CFArrayCreate(NULL, (const void *[]){desc}, 1, &retainedArrCallbacks);
    releaseO(desc);
    return arr;
}
