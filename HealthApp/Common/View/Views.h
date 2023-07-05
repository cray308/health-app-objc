#ifndef Views_h
#define Views_h

#include "CocoaHelpers.h"

#define GroupSpacing 20
#define ViewSpacing 4
#define ViewHeightDefault 44
#define LayoutPriorityRequired 999
#define FontSizeSmall 13

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
    ControlStateSelected = 4,
    ControlStateDisabled = 2
};

enum {
    ControlEventTouchUpInside = 64,
    ControlEventValueChanged = 4096
};

enum {
    TextAlignmentCenter = 1
};

enum {
    ActionStyleDefault,
    ActionStyleCancel,
    ActionStyleDestructive
};

extern CFStringRef NSForegroundColorAttributeName;
extern CFStringRef NSFontAttributeName;

extern CFStringRef UIFontTextStyleTitle3;
extern CFStringRef UIFontTextStyleHeadline;
extern CFStringRef UIFontTextStyleSubheadline;
extern CFStringRef UIFontTextStyleBody;

extern CGFloat UIFontWeightSemibold;
extern CGFloat UIFontWeightRegular;

extern uint64_t UIAccessibilityTraitButton;
extern uint64_t UIAccessibilityTraitHeader;
extern uint64_t UIAccessibilityTraitStaticText;

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
        SEL pf;
        id (*preferred)(Class, SEL, CFStringRef);
    } font;
    const struct {
        Class cls;
        SEL cwi, ac, sa, sp;
        id (*withItem)(Class, SEL, id, long, long, id, long, CGFloat, CGFloat);
        void (*activate)(Class, SEL, CFArrayRef);
        void (*sActive)(id, SEL, bool);
        void (*setPriority)(id, SEL, float);
    } con;
    const struct {
        SEL sbtc;
        SEL stic;
        SEL stamic, sbc, ste, gt, stec, sta, sd, sti;
    } common;
    const struct {
        size_t classSize;
        SEL gv, nc;
        id (*gView)(id, SEL);
        id (*navVC)(id, SEL);
        SEL sv;
        void (*setVal)(id, SEL, id, CFStringRef);
    } vc;
    const struct {
        size_t classSize;
        SEL as, gl, scr, st, gt, sh, sa, schp, sccrp, slm, suie;
        SEL sal, sah, sav, sat, siae, vwt, gb, cr;
        void (*setTranslates)(id, SEL, bool);
        void (*setBG)(id, SEL, id);
        void (*add)(id, SEL, id);
        id (*layer)(id, SEL);
        void (*setCorner)(id, SEL, CGFloat);
        void (*sTag)(id, SEL, long);
        long (*gTag)(id, SEL);
        void (*sHidden)(id, SEL, bool);
        void (*sAlpha)(id, SEL, CGFloat);
        void (*setHugging)(id, SEL, float, long);
        void (*setResistance)(id, SEL, float, long);
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
        SEL gs;
        CFArrayRef (*subviews)(id, SEL);
    } view;
    const struct {
        SEL aas, ss, scs, slmra;
        void (*setTranslates)(id, SEL, bool);
        void (*add)(id, SEL, id);
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
        SEL at, se, ge, st, stc, snl, ct;
        void (*add)(id, SEL, id, SEL, u_long);
        void (*sEnabled)(id, SEL, bool);
        bool (*getEnabled)(id, SEL);
        void (*sTitle)(id, SEL, CFStringRef, u_long);
        void (*setColor)(id, SEL, id, u_long);
        void (*needsLayout)(id, SEL);
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
        SEL ska;
        void (*setAppearance)(id, SEL, long);
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
extern Class Button;
extern Class BarButtonItem;

#define getIVV(t, x) ((t *)((char *)(x) + ViewTable.view.classSize))
#define getIVVC(t, x) ((t *)((char *)(x) + ViewTable.vc.classSize))
#define getIVVCS(t, s) ((t *)((char *)(s) + sizeof(typeof(*s))))
#define getIVVCC(t, s, x) ((t *)((char *)(x) + ViewTable.vc.classSize + sizeof(s)))

#define SetBackgroundSel ViewTable.common.sbc
#define SetTitleSel ViewTable.common.sti

#define getTabBarAppearanceClass() objc_getClass("UITabBarAppearance")
#define getViewInitSel() sel_getUid("initWithFrame:")
#define getTapSel() sel_getUid("buttonTapped:")
#define getValueChangedSel() sel_getUid("valueDidChange")

#define getView(c) ViewTable.vc.gView((c), ViewTable.vc.gv)
#define getNavVC(c) ViewTable.vc.navVC((c), ViewTable.vc.nc)
#define setValue(c, v, k) ViewTable.vc.setVal((c), ViewTable.vc.sv, v, k)
#define getNavItem(c) msgV(objSig(id), (c), sel_getUid("navigationItem"))
#define isViewLoaded(c) msgV(objSig(bool), (c), sel_getUid("isViewLoaded"))
#define presentVC(p, c)                                                          \
 msgV(objSig(void, id, bool, Callback), (p),                                     \
      sel_getUid("presentViewController:animated:completion:"), (c), true, NULL)

#define getPreferredFont(s) ViewTable.font.preferred(ViewTable.font.cls, ViewTable.font.pf, s)

id getSystemFont(int size, CGFloat weight);

#define makeConstraint(v, d1, t, o, d2, c)\
 ViewTable.con.withItem(ViewTable.con.cls, ViewTable.con.cwi, (v), (d1), (t), (o), (d2), 1, (c))
#define activateConstraints(a) ViewTable.con.activate(ViewTable.con.cls, ViewTable.con.ac, (a))
#define lowerPriority(c) ViewTable.con.setPriority((c), ViewTable.con.sp, LayoutPriorityRequired)
#define setActive(c) ViewTable.con.sActive((c), ViewTable.con.sa, true)

#define getBounds(r, v) callRectMethod(ViewTable.view.gBounds, r, v, ViewTable.view.gb)
#define convertRect(r, d, b, s) callRectMethod(ViewTable.view.convert, r, d, ViewTable.view.cr, b, s)

#define setBarTintColor(o, c) msgV(objSig(void, id), (o), ViewTable.common.sbtc, (c))
#define setTintColor(o, c) msgV(objSig(void, id), (o), ViewTable.common.stic, (c))
#define setTextAlignment(o, a) msgV(objSig(void, long), (o), ViewTable.common.sta, (a))
#define setDelegate(o, d) msgV(objSig(void, id), (o), ViewTable.common.sd, (d))
#define setDataSource(o, d) msgV(objSig(void, id), (o), sel_getUid("setDataSource:"), (d))
#define useConstraints(v) ViewTable.view.setTranslates((v), ViewTable.common.stamic, false)
#define setBackgroundColor(v, c) ViewTable.view.setBG((v), SetBackgroundSel, (c))
#define addSubview(v, s) ViewTable.view.add((v), ViewTable.view.as, (s))
#define getSubviews(v) ViewTable.view.subviews((v), ViewTable.view.gs)
#define getLayer(v) ViewTable.view.layer((v), ViewTable.view.gl)
#define addCornerRadius(v) ViewTable.view.setCorner(getLayer(v), ViewTable.view.scr, 5)
#define setTag(v, t) ViewTable.view.sTag((v), ViewTable.view.st, (t))
#define getTag(v) ViewTable.view.gTag((v), ViewTable.view.gt)
#define setHidden(v, h) ViewTable.view.sHidden((v), ViewTable.view.sh, (h))
#define setAlpha(v, a) ViewTable.view.sAlpha((v), ViewTable.view.sa, a)
#define setContentHuggingPriority(v, p, a) ViewTable.view.setHugging((v), ViewTable.view.schp, p, a)
#define setContentCompressionResistancePriority(v, p, a)\
 ViewTable.view.setResistance((v), ViewTable.view.sccrp, p, a)
#define setUserInteractionEnabled(v, e) ViewTable.view.setInteract((v), ViewTable.view.suie, (e))
#define setAccessibilityLabel(v, l) ViewTable.view.setLabel((v), ViewTable.view.sal, (l))
#define setAccessibilityHint(v, h) ViewTable.view.setHint((v), ViewTable.view.sah, (h))
#define setAccessibilityValue(v, a) ViewTable.view.setVal((v), ViewTable.view.sav, (a))
#define setAccessibilityTraits(v, t) ViewTable.view.setTraits((v), ViewTable.view.sat, (t))
#define setIsAccessibilityElement(v, a) ViewTable.view.setAccessible((v), ViewTable.view.siae, (a))
#define viewWithTag(v, t) ViewTable.view.withTag((v), ViewTable.view.vwt, (t))

#define useStackConstraints(s) ViewTable.stack.setTranslates((s), ViewTable.common.stamic, false)
#define addArrangedSubview(s, o) ViewTable.stack.add((s), ViewTable.stack.aas, (o))
#define setSpacing(s, n) ViewTable.stack.sSpacing((s), ViewTable.stack.ss, (n))
#define setCustomSpacing(s, n, v) ViewTable.stack.setCustom((s), ViewTable.stack.scs, (n), (v))

#define setText(l, t) ViewTable.label.sText((l), ViewTable.common.ste, (t))
#define getText(l) ViewTable.label.gText((l), ViewTable.common.gt)
#define setTextColor(l, c) ViewTable.label.setColor((l), ViewTable.common.stec, (c))

#define addTarget(v, t, a, e) ViewTable.button.add((v), ViewTable.button.at, (t), (a), (e))
#define setEnabled(b, e) ViewTable.button.sEnabled((b), ViewTable.button.se, (e))
#define isEnabled(b) ViewTable.button.getEnabled((b), ViewTable.button.ge)
#define setTitle(b, t, s) ViewTable.button.sTitle((b), ViewTable.button.st, (t), (s))
#define setTitleColor(b, c, s) ViewTable.button.setColor((b), ViewTable.button.stc, (c), (s))
#define setNeedsLayout(b) ViewTable.button.needsLayout((b), ViewTable.button.snl)
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
#define setKeyboardAppearance(f, a) ViewTable.field.setAppearance((f), ViewTable.field.ska, (a))

void setFieldBackgroundColor(id field, id color);
void setFieldTextColor(id field, id color);

#define setContentInset(s, i) ViewTable.scroll.setInset((s), ViewTable.scroll.sci, (i))
#define scrollRectToVisible(s, r) ViewTable.scroll.scroll((s), ViewTable.scroll.sr, (r), true)

void initViewData(void (*inits[])(void), int count);

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
id createButton(CFStringRef title CF_CONSUMED, int titleColor,
                bool singleLine, id target, SEL action);
id createSegmentedControl(CFStringRef format, int startIndex);
id createTextField(id delegate, id accessory, CFStringRef hint CF_CONSUMED, int tag);

static inline void updateButtonColors(id button, int titleColor) {
    setTitleColor(button, getColor(titleColor), ControlStateNormal);
    setTitleColor(button, getColor(ColorDisabled), ControlStateDisabled);
    setBackgroundColor(button, getColor(ColorSecondaryBGGrouped));
}

void updateSegmentedControlColors(id control, bool darkMode);

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
