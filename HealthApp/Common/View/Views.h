#ifndef Views_h
#define Views_h

#include "CocoaHelpers.h"

#define GroupSpacing 20
#define ViewSpacing 4
#define ViewHeightDefault 44
#define LayoutPriorityRequired 999

enum {
    LayoutAttributeTop = 3,
    LayoutAttributeBottom,
    LayoutAttributeLeading,
    LayoutAttributeTrailing,
    LayoutAttributeWidth,
    LayoutAttributeHeight
};

enum {
    AutoresizingFlexibleHeight = 16
};

enum {
    ConstraintAxisHorizontal,
    ConstraintAxisVertical
};

enum {
    ControlStateNormal,
    ControlStateDisabled = 2
};

enum {
    ControlEventTouchUpInside = 64,
    ControlEventValueChanged = 4096
};

enum {
    ActionStyleDefault,
    ActionStyleCancel,
    ActionStyleDestructive
};

extern CFStringRef UIFontTextStyleSubheadline;
extern CFStringRef UIFontTextStyleBody;

extern uint32_t UIAccessibilityAnnouncementNotification;
extern void UIAccessibilityPostNotification(uint32_t, id);
extern BOOL UIAccessibilityIsVoiceOverRunning(void);

typedef struct {
    CGFloat top, left, bottom, right;
} HAInsets;

#define ViewMargins ((HAInsets){4, 0, 4, 0})

struct VCache {
    const struct {
        Class cls;
        SEL cwi, sa, sp;
        id (*withItem)(Class, SEL, id, long, long, id, long, CGFloat, CGFloat);
        void (*sActive)(id, SEL, bool);
        void (*setPriority)(id, SEL, float);
    } con;
    const struct {
        SEL stic;
        SEL stamic, sbc, ste, gt, stec, sd, sti;
    } common;
    const struct {
        size_t classSize;
        SEL gv, nc;
        id (*gView)(id, SEL);
        id (*navVC)(id, SEL);
    } vc;
    const struct {
        size_t classSize;
        SEL as, rfs, gl, scr, st, gt, sh, slm, suie, sal, sah, sav, sat, siae, vwt, gb, cr;
        void (*setTranslates)(id, SEL, bool);
        void (*setBG)(id, SEL, id);
        void (*add)(id, SEL, id);
        void (*remove)(id, SEL);
        id (*layer)(id, SEL);
        void (*setCorner)(id, SEL, CGFloat);
        void (*sTag)(id, SEL, long);
        long (*gTag)(id, SEL);
        void (*sHidden)(id, SEL, bool);
        void (*setLayoutMargins)(id, SEL, HAInsets);
        void (*setInteract)(id, SEL, bool);
        void (*setLabel)(id, SEL, CFStringRef);
        void (*setHint)(id, SEL, CFStringRef);
        void (*setVal)(id, SEL, CFStringRef);
        void (*setTraits)(id, SEL, uint64_t);
        void (*setAccessible)(id, SEL, bool);
        id (*withTag)(id, SEL, long);
        generateRectFunctionSignature(gBounds);
        generateRectFunctionSignature(convert, CGRect, id);
    } view;
    const struct {
        SEL aas, as, ss, scs, slmra;
        void (*setTranslates)(id, SEL, bool);
        void (*add)(id, SEL, id);
        CFArrayRef (*arranged)(id, SEL);
        void (*sSpacing)(id, SEL, CGFloat);
        void (*setCustom)(id, SEL, CGFloat, id);
        void (*setLayoutMarginsRelativeArrangement)(id, SEL, bool);
    } stack;
    const struct {
        void (*sText)(id, SEL, CFStringRef);
        CFStringRef (*gText)(id, SEL);
        void (*setColor)(id, SEL, id);
    } label;
    const struct {
        SEL at, se, st, stc, ct;
        void (*add)(id, SEL, id, SEL, u_long);
        void (*sEnabled)(id, SEL, bool);
        void (*sTitle)(id, SEL, CFStringRef, u_long);
        void (*setColor)(id, SEL, id, u_long);
        CFStringRef (*gTitle)(id, SEL);
    } button;
    const struct {
        SEL sii, sic;
        id (*iInit)(id, SEL, id, long, id, SEL);
        id (*cInit)(id, SEL, id);
    } bbi;
    const struct {
        SEL skt, bfr;
        void (*sText)(id, SEL, CFStringRef);
        CFStringRef (*gText)(id, SEL);
        bool (*becomeFirst)(id, SEL);
    } field;
    const struct {
        Class cls;
        SEL sci, sr;
        void (*setInset)(id, SEL, HAInsets);
        void (*scroll)(id, SEL, CGRect, bool);
    } scroll;
};

extern struct VCache ViewTable;
extern Class VC;
extern Class View;
extern Class BarButtonItem;

#define getIVV(t, x) ((t *)((char *)(x) + ViewTable.view.classSize))
#define getIVVC(t, x) ((t *)((char *)(x) + ViewTable.vc.classSize))
#define getIVVCS(t, s) ((t *)((char *)(s) + sizeof(typeof(*s))))
#define getIVVCC(t, s, x) ((t *)((char *)(x) + ViewTable.vc.classSize + sizeof(s)))

#define SetBackgroundSel ViewTable.common.sbc
#define SetTitleSel ViewTable.common.sti

#define getTapSel() sel_getUid("buttonTapped:")
#define getCustomButtonSel() sel_getUid("customButtonTapped:")
#define getValueChangedSel() sel_getUid("valueDidChange")

#define getView(c) ViewTable.vc.gView((c), ViewTable.vc.gv)
#define getNavVC(c) ViewTable.vc.navVC((c), ViewTable.vc.nc)
#define getNavItem(c) msgV(objSig(id), (c), sel_getUid("navigationItem"))
#define isViewLoaded(c) msgV(objSig(bool), (c), sel_getUid("isViewLoaded"))
#define presentVC(p, c)                                                          \
 msgV(objSig(void, id, bool, Callback), (p),                                     \
      sel_getUid("presentViewController:animated:completion:"), (c), true, NULL)

#define makeConstraint(v, d1, t, o, d2, c)\
 ViewTable.con.withItem(ViewTable.con.cls, ViewTable.con.cwi, (v), (d1), (t), (o), (d2), 1, (c))
#define lowerPriority(c) ViewTable.con.setPriority((c), ViewTable.con.sp, LayoutPriorityRequired)
#define setActive(c) ViewTable.con.sActive((c), ViewTable.con.sa, true)

#define getBounds(r, v) callRectMethod(ViewTable.view.gBounds, r, v, ViewTable.view.gb)
#define convertRect(r, d, b, s) callRectMethod(ViewTable.view.convert, r, d, ViewTable.view.cr, b, s)

#define setTintColor(o, c) msgV(objSig(void, id), (o), ViewTable.common.stic, (c))
#define setDelegate(o, d) msgV(objSig(void, id), (o), ViewTable.common.sd, (d))
#define useConstraints(v) ViewTable.view.setTranslates((v), ViewTable.common.stamic, false)
#define setBackgroundColor(v, c) ViewTable.view.setBG((v), SetBackgroundSel, (c))
#define addSubview(v, s) ViewTable.view.add((v), ViewTable.view.as, (s))
#define removeFromSuperview(v) ViewTable.view.remove((v), ViewTable.view.rfs)
#define getLayer(v) ViewTable.view.layer((v), ViewTable.view.gl)
#define addCornerRadius(v) ViewTable.view.setCorner(getLayer(v), ViewTable.view.scr, 5)
#define setTag(v, t) ViewTable.view.sTag((v), ViewTable.view.st, (t))
#define getTag(v) ViewTable.view.gTag((v), ViewTable.view.gt)
#define setHidden(v, h) ViewTable.view.sHidden((v), ViewTable.view.sh, (h))
#define setUserInteractionEnabled(v, e) ViewTable.view.setInteract((v), ViewTable.view.suie, (e))
#define setAccessibilityLabel(v, l) ViewTable.view.setLabel((v), ViewTable.view.sal, (l))
#define setAccessibilityHint(v, h) ViewTable.view.setHint((v), ViewTable.view.sah, (h))
#define setAccessibilityValue(v, a) ViewTable.view.setVal((v), ViewTable.view.sav, (a))
#define setAccessibilityTraits(v, t) ViewTable.view.setTraits((v), ViewTable.view.sat, (t))
#define setIsAccessibilityElement(v, a) ViewTable.view.setAccessible((v), ViewTable.view.siae, (a))
#define viewWithTag(v, t) ViewTable.view.withTag((v), ViewTable.view.vwt, (t))

#define useStackConstraints(s) ViewTable.stack.setTranslates((s), ViewTable.common.stamic, false)
#define addArrangedSubview(s, o) ViewTable.stack.add((s), ViewTable.stack.aas, (o))
#define getArrangedSubviews(s) ViewTable.stack.arranged((s), ViewTable.stack.as)
#define setSpacing(s, n) ViewTable.stack.sSpacing((s), ViewTable.stack.ss, (n))
#define setCustomSpacing(s, n, v) ViewTable.stack.setCustom((s), ViewTable.stack.scs, (n), (v))

#define setText(l, t) ViewTable.label.sText((l), ViewTable.common.ste, (t))
#define getText(l) ViewTable.label.gText((l), ViewTable.common.gt)
#define setTextColor(l, c) ViewTable.label.setColor((l), ViewTable.common.stec, (c))

#define addTarget(v, t, a, e) ViewTable.button.add((v), ViewTable.button.at, (t), (a), (e))
#define setEnabled(b, e) ViewTable.button.sEnabled((b), ViewTable.button.se, (e))
#define setTitle(b, t, s) ViewTable.button.sTitle((b), ViewTable.button.st, (t), (s))
#define setTitleColor(b, c, s) ViewTable.button.setColor((b), ViewTable.button.stc, (c), (s))
#define getCurrentTitle(b) ViewTable.button.gTitle((b), ViewTable.button.ct)

#define getSelectedSegmentIndex(s) msgV(objSig(long), (s), sel_getUid("selectedSegmentIndex"))
#define setSelectedSegmentIndex(s, i)\
 msgV(objSig(void, long), (s), sel_getUid("setSelectedSegmentIndex:"), (i))

#define createBarButtonItemWithImage(i, t, a)\
 ViewTable.bbi.iInit(alloc(BarButtonItem), ViewTable.bbi.sii, getImage(i), 0, (t), (a))
#define createBarButtonItemWithView(v) ViewTable.bbi.cInit(alloc(BarButtonItem), ViewTable.bbi.sic, v)

#define setFieldText(f, t) ViewTable.field.sText((f), ViewTable.common.ste, (t))
#define getFieldText(f) ViewTable.field.gText((f), ViewTable.common.gt)
#define becomeFirstResponder(f) ViewTable.field.becomeFirst((f), ViewTable.field.bfr)
#define setKeyboardType(f, t) msgV(objSig(void, long), (f), ViewTable.field.skt, (t))

#define setContentInset(s, i) ViewTable.scroll.setInset((s), ViewTable.scroll.sci, (i))
#define scrollRectToVisible(s, r) ViewTable.scroll.scroll((s), ViewTable.scroll.sr, (r), true)

void initViewData(void (*inits[])(void));

void pin(id view, id container);

#define pinToSafeArea(c, v) pin((c), msgV(objSig(id), (v), sel_getUid("safeAreaLayoutGuide")))

static inline void setHeight(id view, int height, bool greaterOrEqual, bool optional) {
    id constraint = makeConstraint(view, LayoutAttributeHeight, greaterOrEqual, nil, 0, height);
    if (optional) lowerPriority(constraint);
    setActive(constraint);
}

static inline id createScrollView(void) {
    id scroll = new(ViewTable.scroll.cls);
    msgV(objSig(void, long), scroll, sel_getUid("setAutoresizingMask:"), AutoresizingFlexibleHeight);
    return scroll;
}

id createHStack(id const *subviews);
id createVStack(id const *subviews, int count);

static inline void setLayoutMargins(id stack, HAInsets insets) {
    ViewTable.view.setLayoutMargins(stack, ViewTable.view.slm, insets);
    ViewTable.stack.setLayoutMarginsRelativeArrangement(stack, ViewTable.stack.slmra, true);
}

id createLabel(CFStringRef text CF_CONSUMED, CFStringRef style, int textColor);
id createButton(CFStringRef title CF_CONSUMED, int titleColor, id target, SEL action);
id createSegmentedControl(CFStringRef format, int startIndex);
id createTextField(id delegate, id accessory, CFStringRef hint CF_CONSUMED, int tag);

extern void setupNavItem(id vc, CFStringRef titleKey, id const *buttons);
extern void setupHierarchy(id vc, id vStack, id scrollView, int backgroundColor);

extern void presentModalVC(id vc, id modal);

static inline void dismissPresentedVC(id vc, Callback handler) {
    id presenter = msgV(objSig(id), getNavVC(vc), sel_getUid("presentingViewController"));
    msgV(objSig(void, bool, Callback), presenter,
         sel_getUid("dismissViewControllerAnimated:completion:"), true, handler);
}

extern id createAlert(CFStringRef titleKey, CFStringRef messageKey);
extern void addAlertAction(id alert, CFStringRef titleKey, int style, Callback handler);
extern void disableWindowTint(void);

#endif /* Views_h */
