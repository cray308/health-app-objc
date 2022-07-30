#ifndef Views_h
#define Views_h

#include <CoreGraphics/CoreGraphics.h>
#include "CocoaHelpers.h"

#define getIVV(x) ((char *)(x) + ViewSize)
#define getIVVC(x) ((char *)(x) + VCSize)

typedef struct {
    CGFloat top, left, bottom, right;
} HAInsets;

struct VCache {
    const struct ConstraintCache {
        Class cls;
        SEL cr, ac, lp;
        id (*init)(Class,SEL,id,long,long,id,long,CGFloat,CGFloat);
        void (*activateC)(id,SEL,bool);
        void (*lowerPri)(id,SEL,float);
    } cc;
    const struct ViewCache {
        SEL trans, sbg, bnd, conv, asv, rsv, glyr, scrad, stg, gtg, shd, sacl, shn, satrs, sace;
        SEL vwt, lfn;
        void (*addSub)(id,SEL,id);
        void (*rmSub)(id,SEL);
        id (*gLayer)(id,SEL);
        void (*setCorner)(id,SEL,CGFloat);
        void (*sTag)(id,SEL,long);
        long (*gTag)(id,SEL);
        void (*hide)(id,SEL,bool);
        void (*setAcc)(id,SEL,CFStringRef);
        void (*setHint)(id,SEL,CFStringRef);
        void (*setTraits)(id,SEL,uint64_t);
        void (*setIsAcc)(id,SEL,bool);
        id (*gvWithTag)(id,SEL,long);
        void (*layout)(id,SEL);
    } vc;
    const struct StackCache {
        SEL asv, gsv, smr, ssp, scsp;
        void (*addSub)(id,SEL,id);
        CFArrayRef (*getSub)(id,SEL);
        void (*setMargins)(id,SEL,HAInsets);
        void (*setSpace)(id,SEL,CGFloat);
        void (*setSpaceAfter)(id,SEL,CGFloat,id);
    } sc;
    const struct LabelCache {
        SEL stxt, gtxt, stc;
    } lc;
    const struct ButtonCache {
        SEL atgt, en, sbtxt, gct, sbc;
        void (*addTgt)(id,SEL,id,SEL,unsigned long);
        void (*sEnabled)(id,SEL,bool);
        void (*setTitle)(id,SEL,CFStringRef,unsigned long);
        CFStringRef (*gTitle)(id,SEL);
        void (*setColor)(id,SEL,id,unsigned long);
    } bc;
    const struct FieldCache {
        SEL sdg, skbt, bfr;
        bool (*becomeResponder)(id,SEL);
    } field;
    const struct ScrollCache {
        SEL sci, scrvs;
        void (*setInset)(id,SEL,HAInsets);
        void (*scrollR)(id,SEL,CGRect,bool);
    } scr;
};

enum {
    NSLayoutAttributeTop = 3,
    NSLayoutAttributeBottom,
    NSLayoutAttributeLeading,
    NSLayoutAttributeTrailing,
    NSLayoutAttributeWidth,
    NSLayoutAttributeHeight
};

enum {
    UILayoutConstraintAxisHorizontal,
    UILayoutConstraintAxisVertical
};

enum {
    UIControlStateNormal,
    UIControlStateDisabled = 2
};

enum {
    UIControlEventTouchUpInside = 64,
    UIControlEventValueChanged = 4096
};

enum {
    UIAlertActionStyleDefault,
    UIAlertActionStyleCancel,
    UIAlertActionStyleDestructive
};

#define GroupSpacing 20
#define ViewSpacing 4
#define ViewHeightDefault 44
#define LayoutPriorityRequired 999
#define VCMargins ((HAInsets){16, 8, 16, 8})
#define ViewMargins ((HAInsets){4, 0, 4, 0})

#define makeConstraint(v, d1, t, o, d2, c)\
 Vtbl.cc.init(Vtbl.cc.cls, Vtbl.cc.cr, (v), (d1), (t), (o), (d2), 1, (c))
#define lowerPriority(c) Vtbl.cc.lowerPri((c), Vtbl.cc.lp, LayoutPriorityRequired)
#define setActive(c) Vtbl.cc.activateC((c), Vtbl.cc.ac, true)

#if defined(__arm64__)
#define getRect(rect, o, cmd) rect = msg0(CGRect, (o), (cmd))
#define convertRect(rect, bounds, dst, src)\
 rect = msg2(CGRect, CGRect, id, (dst), Vtbl.vc.conv, (bounds), (src))
#else
#define getRect(rect, o, cmd) ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&rect, (o), (cmd))
#define convertRect(rect, bounds, dst, src)\
 ((void(*)(CGRect*,id,SEL,CGRect,id))objc_msgSend_stret)(&rect, (dst), Vtbl.vc.conv, (bounds), (src))
#endif

#define setDelegate(o, d) msg1(void, id, (o), Vtbl.field.sdg, (d))
#define setTrans(v) msg1(void, bool, (v), Vtbl.vc.trans, false)
#define setBackgroundColor(v, c) msg1(void, id, (v), Vtbl.vc.sbg, (c))
#define addSubview(v, s) Vtbl.vc.addSub((v), Vtbl.vc.asv, (s))
#define removeFromSuperview(v) Vtbl.vc.rmSub((v), Vtbl.vc.rsv)
#define getLayer(v) Vtbl.vc.gLayer((v), Vtbl.vc.glyr)
#define setCornerRadius(v) Vtbl.vc.setCorner(getLayer(v), Vtbl.vc.scrad, 5)
#define setTag(v, t) Vtbl.vc.sTag((v), Vtbl.vc.stg, (t))
#define getTag(v) Vtbl.vc.gTag((v), Vtbl.vc.gtg)
#define setHidden(v, h) Vtbl.vc.hide((v), Vtbl.vc.shd, (h))
#define setAccessibilityLabel(v, l) Vtbl.vc.setAcc((v), Vtbl.vc.sacl, (l))
#define setAccessibilityHint(v, h) Vtbl.vc.setHint((v), Vtbl.vc.shn, (h))
#define setAccessibilityTraits(v, t) Vtbl.vc.setTraits((v), Vtbl.vc.satrs, (t))
#define setIsAccessibilityElement(v, a) Vtbl.vc.setIsAcc((v), Vtbl.vc.sace, (a))
#define viewWithTag(v, t) Vtbl.vc.gvWithTag((v), Vtbl.vc.vwt, (t))
#define layoutIfNeeded(v) Vtbl.vc.layout((v), Vtbl.vc.lfn)

#define addArrangedSubview(s, o) Vtbl.sc.addSub((s), Vtbl.sc.asv, (o))
#define getArrangedSubviews(s) Vtbl.sc.getSub((s), Vtbl.sc.gsv)
#define setLayoutMargins(s, m) Vtbl.sc.setMargins((s), Vtbl.sc.smr, (m))
#define setSpacing(s, n) Vtbl.sc.setSpace((s), Vtbl.sc.ssp, (n))
#define setCustomSpacing(s, n, v) Vtbl.sc.setSpaceAfter((s), Vtbl.sc.scsp, (n), (v))

#define setText(l, t) msg1(void, CFStringRef, (l), Vtbl.lc.stxt, (t))
#define getText(l) msg0(CFStringRef, (l), Vtbl.lc.gtxt)
#define setTextColor(l, c) msg1(void, id, (l), Vtbl.lc.stc, (c))

#define addTarget(v, t, a, e) Vtbl.bc.addTgt((v), Vtbl.bc.atgt, (t), (a), (e))
#define setEnabled(b, e) Vtbl.bc.sEnabled((b), Vtbl.bc.en, (e))
#define setBtnTitle(b, t) Vtbl.bc.setTitle((b), Vtbl.bc.sbtxt, (t), UIControlStateNormal)
#define getBtnTitle(b) Vtbl.bc.gTitle((b), Vtbl.bc.gct)
#define setTitleColor(b, c, s) Vtbl.bc.setColor((b), Vtbl.bc.sbc, (c), (s))

#define setKeyboardType(f, t) msg1(void, long, (f), Vtbl.field.skbt, (t))
#define becomeFirstResponder(f) Vtbl.field.becomeResponder((f), Vtbl.field.bfr)

#define setContentInset(s, i) Vtbl.scr.setInset((s), Vtbl.scr.sci, (i))
#define scrollRect(s, r) Vtbl.scr.scrollR((s), Vtbl.scr.scrvs, (r), true)

extern CFStringRef UIFontTextStyleSubheadline;
extern CFStringRef UIFontTextStyleBody;

extern uint32_t UIAccessibilityAnnouncementNotification;
extern void UIAccessibilityPostNotification(uint32_t, id);
extern BOOL UIAccessibilityIsVoiceOverRunning(void);

extern struct VCache Vtbl;
extern Class VC;
extern Class View;
extern size_t VCSize;
extern size_t ViewSize;

void initViewData(void (*inits[])(void));

void pin(id view, id container);
void setHeight(id view, int height, bool geq, bool optional);

id createHStack(id *subviews);
id createVStack(id *subviews, int count);
id createScrollView(void);
id createLabel(CFStringRef text CF_CONSUMED, CFStringRef style, int color);
id createButton(CFStringRef title CF_CONSUMED, int color, CFStringRef style, id target, SEL action);
id createSegmentedControl(CFStringRef format, int startIndex);
id createTextfield(id delegate, id accessory, CFStringRef hint CF_CONSUMED, int tag);

void setupNavItem(id vc, CFStringRef titleKey, id *buttons);
void setupHierarchy(id vc, id vStack, id scrollView, int color);

void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id vc, Callback handler);

id createAlertController(CFStringRef titleKey, CFStringRef msgKey);
void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler);

#endif /* Views_h */
