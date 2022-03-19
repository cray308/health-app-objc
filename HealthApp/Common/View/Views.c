#include "Views.h"
#include <string.h>

extern CGFloat UIFontWeightMedium;

struct FontData {
    Class cls;
    SEL pf;
    id (*pref)(Class,SEL,CFStringRef);
};
struct StackData {
    Class cls;
    SEL si, sa, saln, ssp, slmra;
    id (*init)(id,SEL,CFArrayRef);
    void (*setAxis)(id,SEL,long);
    void (*setAlign)(id,SEL,long);
    void (*setSpace)(id,SEL,CGFloat);
    void (*setRelative)(id,SEL,bool);
};
struct LabelData {
    Class cls;
    SEL sajfs, sajfc, smsf, sace, sf;
    void (*adjFontWidth)(id,SEL,bool);
    void (*adjFontCat)(id,SEL,bool);
    void (*setScale)(id,SEL,CGFloat);
    void (*setAcc)(id,SEL,bool);
    void (*setFont)(id,SEL,id);
};
struct ButtonData {
    Class cls;
    SEL si, glb;
    id (*init)(Class,SEL,long);
    id (*getLabel)(id,SEL);
};
struct FieldData {
    Class cls;
    SEL salg, sbs, skbt;
    void (*adjFontCat)(id,SEL,bool);
    void (*setFont)(id,SEL,id);
    void (*setAlign)(id,SEL,long);
    void (*setBorder)(id,SEL,long);
    void (*setDel)(id,SEL,id);
};
struct PrivVData {
    const struct FontData fc;
    const struct StackData stack;
    const struct LabelData label;
    const struct ButtonData button;
    const struct FieldData field;
};

Class VC;
Class View;
size_t VCSize;
size_t ViewSize;
static struct PrivVData cache;

void initViewData(VCache *cacheRef) {
    Class Font = objc_getClass("UIFont"); SEL pf = sel_getUid("preferredFontForTextStyle:");
    struct FontData fc = {Font, pf, (id(*)(Class,SEL,CFStringRef))getImpC(Font, pf)};

    Class Constraint = objc_getClass("NSLayoutConstraint"); SEL lp = sel_getUid("setPriority:");
    SEL cr = sel_getUid("constraintWithItem:attribute:relatedBy:toItem:attribute:multiplier:constant:");
    SEL ac = sel_getUid("setActive:"), aar = sel_getUid("activateConstraints:");
    ConstraintCache cc = {Constraint, cr, ac, aar, lp,
        (id(*)(Class,SEL,id,long,long,id,long,CGFloat,CGFloat))getImpC(Constraint, cr),
        (void(*)(id,SEL,bool))getImpO(Constraint, ac),
        (void(*)(Class,SEL,CFArrayRef))getImpC(Constraint, aar),
        (void(*)(id,SEL,float))getImpO(Constraint, lp)
    };

    SEL trans = sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), glyr = sel_getUid("layer");
    SEL asv = sel_getUid("addSubview:"), rsv = sel_getUid("removeFromSuperview"), gtg = sel_getUid("tag");
    SEL stg = sel_getUid("setTag:"), shd = sel_getUid("setHidden:"), sbg = sel_getUid("setBackgroundColor:");
    SEL sacl = sel_getUid("setAccessibilityLabel:");
    ViewCache view = {trans, asv, rsv, glyr, stg, gtg, shd, sbg, sacl,
        (void(*)(id,SEL,bool))getImpO(View, trans), (void(*)(id,SEL,id))getImpO(View, asv),
        (void(*)(id,SEL))getImpO(View, rsv), (id(*)(id,SEL))getImpO(View, glyr),
        (void(*)(id,SEL,long))getImpO(View, stg), (long(*)(id,SEL))getImpO(View, gtg),
        (void(*)(id,SEL,bool))getImpO(View, shd), (void(*)(id,SEL,id))getImpO(View, sbg),
        (void(*)(id,SEL,CFStringRef))getImpO(View, sacl)
    };

    Class Stack = objc_getClass("UIStackView"); SEL istack = sel_getUid("initWithArrangedSubviews:");
    SEL aasv = sel_getUid("addArrangedSubview:"), gsv = sel_getUid("arrangedSubviews");
    SEL smr = sel_getUid("setLayoutMargins:"), saln = sel_getUid("setAlignment:");
    SEL ssp = sel_getUid("setSpacing:"), slmra = sel_getUid("setLayoutMarginsRelativeArrangement:");
    SEL sa = sel_getUid("setAxis:");
    struct StackData _stData = {Stack, istack, sa, saln, ssp, slmra,
        (id(*)(id,SEL,CFArrayRef))getImpO(Stack, istack), (void(*)(id,SEL,long))getImpO(Stack, sa),
        (void(*)(id,SEL,long))getImpO(Stack, saln), (void(*)(id,SEL,CGFloat))getImpO(Stack, ssp),
        (void(*)(id,SEL,bool))getImpO(Stack, slmra)
    };
    StackCache stack = {aasv, gsv, smr, (void(*)(id,SEL,id))getImpO(Stack, aasv),
        (CFArrayRef(*)(id,SEL))getImpO(Stack, gsv), (void(*)(id,SEL,HAInsets))getImpO(Stack, smr)
    };

    Class Label = objc_getClass("UILabel");
    SEL smsf = sel_getUid("setMinimumScaleFactor:"), sace = sel_getUid("setIsAccessibilityElement:");
    SEL sajfs = sel_getUid("setAdjustsFontSizeToFitWidth:"), sf = sel_getUid("setFont:");
    SEL sajfc = sel_getUid("setAdjustsFontForContentSizeCategory:"), gtxt = sel_getUid("text");
    SEL stxt = sel_getUid("setText:"), stc = sel_getUid("setTextColor:");
    struct LabelData _lData = {Label, sajfs, sajfc, smsf, sace, sf,
        (void(*)(id,SEL,bool))getImpO(Label, sajfs), (void(*)(id,SEL,bool))getImpO(Label, sajfc),
        (void(*)(id,SEL,CGFloat))getImpO(Label, smsf), (void(*)(id,SEL,bool))getImpO(Label, sace),
        (void(*)(id,SEL,id))getImpO(Label, sf)
    };
    LabelCache label = {stxt, gtxt, stc, (void(*)(id,SEL,CFStringRef))getImpO(Label, stxt),
        (CFStringRef(*)(id,SEL))getImpO(Label, gtxt), (void(*)(id,SEL,id))getImpO(Label, stc)
    };

    Class Button = objc_getClass("UIButton"); SEL ibtn = sel_getUid("buttonWithType:");
    SEL atgt = sel_getUid("addTarget:action:forControlEvents:"), en = sel_getUid("setEnabled:");
    SEL sbtxt = sel_getUid("setTitle:forState:"), sbc = sel_getUid("setTitleColor:forState:");
    SEL glb = sel_getUid("titleLabel");
    struct ButtonData _bData = {Button, ibtn, glb, (id(*)(Class,SEL,long))getImpC(Button, ibtn),
        (id(*)(id,SEL))getImpO(Button, glb)
    };
    ButtonCache button = {atgt, en, sbtxt, sbc, (void(*)(id,SEL,id,SEL,u_long))getImpO(Button, atgt),
        (void(*)(id,SEL,bool))getImpO(Button,en), (void(*)(id,SEL,CFStringRef,u_long))getImpO(Button, sbtxt),
        (void(*)(id,SEL,id,u_long))getImpO(Button, sbc)
    };

    Class Field = objc_getClass("UITextField"); SEL salg = sel_getUid("setTextAlignment:");
    SEL sbs = sel_getUid("setBorderStyle:"), skbt = sel_getUid("setKeyboardType:");
    SEL sdg = sel_getUid("setDelegate:"), siac = sel_getUid("setInputAccessoryView:");
    struct FieldData _fData = {Field, salg, sbs, skbt, (void(*)(id,SEL,bool))getImpO(Field, sajfc),
        (void(*)(id,SEL,id))getImpO(Field, sf), (void(*)(id,SEL,long))getImpO(Field, salg),
        (void(*)(id,SEL,long))getImpO(Field, sbs), (void(*)(id,SEL,id))getImpO(Field, sdg)
    };
    FieldCache field = {sdg, siac, (void(*)(id,SEL,CFStringRef))getImpO(Field, stxt),
        (void(*)(id,SEL,id))getImpO(Field, stc), (void(*)(id,SEL,id))getImpO(Field, siac)
    };

    VCache lcache = {cc, view, stack, label, button, field};
    memcpy(cacheRef, &lcache, sizeof(VCache));
    struct PrivVData localData = {fc, _stData, _lData, _bData, _fData};
    memcpy(&cache, &localData, sizeof(struct PrivVData));
}

static void setDynamicFont(id view) {
    cache.label.adjFontWidth(view, cache.label.sajfs, true);
    cache.label.adjFontCat(view, cache.label.sajfc, true);
    cache.label.setScale(view, cache.label.smsf, 0.85);
}

void pin(ConstraintCache const *tbl, id v, id container) {
    const void *constraints[] = {
        tbl->init(tbl->cls, tbl->cr, v, 3, 0, container, 3, 1, 0),
        tbl->init(tbl->cls, tbl->cr, v, 4, 0, container, 4, 1, 0),
        tbl->init(tbl->cls, tbl->cr, v, 5, 0, container, 5, 1, 0),
        tbl->init(tbl->cls, tbl->cr, v, 6, 0, container, 6, 1, 0)
    };
    CFArrayRef array = CFArrayCreate(NULL, constraints, 4, NULL);
    tbl->activateArr(tbl->cls, tbl->aar, array);
    CFRelease(array);
}

#pragma mark - View initializers

id createStackView(VCacheRef tbl, id *subviews, int count,
                   int axis, int align, int spacing, Padding margins) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)subviews, count, NULL);
    id view = cache.stack.init(Sels.alloc(cache.stack.cls, Sels.alo), cache.stack.si, arr);
    CFRelease(arr);
    cache.stack.setAxis(view, cache.stack.sa, axis);
    cache.stack.setAlign(view, cache.stack.saln, align);
    cache.stack.setSpace(view, cache.stack.ssp, spacing);
    cache.stack.setRelative(view, cache.stack.slmra, true);
    HAInsets insets = {margins.top, margins.left, margins.bottom, margins.right};
    tbl->stack.setMargins(view, tbl->stack.smr, insets);
    return view;
}

id createScrollView(void) {
    id view = Sels.new(objc_getClass("UIScrollView"), Sels.nw);
    msg1(void, long, view, sel_getUid("setAutoresizingMask:"), 16);
    return view;
}

id createLabel(VCacheRef tbl, CCacheRef clr, CFStringRef text, CFStringRef style, bool accessible) {
    LabelCache const *c = &tbl->label;
    id view = Sels.new(cache.label.cls, Sels.nw);
    c->setText(view, c->stxt, text);
    if (text)
        CFRelease(text);
    cache.label.setFont(view, cache.label.sf, cache.fc.pref(cache.fc.cls, cache.fc.pf, style));
    setDynamicFont(view);
    c->setColor(view, c->stc, clr->getColor(clr->cls, clr->sc, ColorLabel));
    cache.label.setAcc(view, cache.label.sace, accessible);
    return view;
}

id createButton(VCacheRef tbl, CCacheRef clr, CFStringRef title,
                int color, CFStringRef style, int tag, id target, SEL action) {
    ButtonCache const *bt = &tbl->button;
    id view = cache.button.init(cache.button.cls, cache.button.si, 1);
    tbl->view.setTrans(view, tbl->view.trans, false);
    bt->setTitle(view, bt->sbtxt, title, 0);
    if (title)
        CFRelease(title);
    bt->setColor(view, bt->sbc, clr->getColor(clr->cls, clr->sc, color), 0);
    bt->setColor(view, bt->sbc, clr->getColor(clr->cls, clr->sc, ColorSecondaryLabel), 2);
    id label = cache.button.getLabel(view, cache.button.glb);
    cache.label.setFont(label, cache.label.sf, cache.fc.pref(cache.fc.cls, cache.fc.pf, style));
    setDynamicFont(label);
    tbl->view.setTag(view, tbl->view.stg, tag);
    bt->addTarget(view, bt->atgt, target, action, 64);
    return view;
}

id createSegmentedControl(CFBundleRef bundle, CFStringRef format, int startIndex) {
    CFStringRef segments[3];
    fillStringArray(bundle, segments, format, 3);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, 3, NULL);
    id view = msg1(id, CFArrayRef, Sels.alloc(objc_getClass("UISegmentedControl"), Sels.alo),
                   sel_getUid("initWithItems:"), array);
    msg1(void, long, view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    CFRelease(array);
    CFRelease(segments[0]);
    CFRelease(segments[1]);
    CFRelease(segments[2]);
    return view;
}

id createTextfield(VCacheRef tbl, CCacheRef clr, id delegate,
                   CFStringRef text, CFStringRef hint, int alignment, int keyboard, int tag) {
    FieldCache const *c = &tbl->field;
    id view = Sels.new(cache.field.cls, Sels.nw);
    tbl->view.setBG(view, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorTertiaryBG));
    c->setText(view, tbl->label.stxt, text);
    cache.field.setFont(view, cache.label.sf, cache.fc.pref(cache.fc.cls, cache.fc.pf, UIFontTextStyleBody));
    c->setColor(view, tbl->label.stc, clr->getColor(clr->cls, clr->sc, ColorLabel));
    cache.field.adjFontCat(view, cache.label.sajfc, true);
    cache.field.setAlign(view, cache.field.salg, alignment);
    tbl->view.setTag(view, tbl->view.stg, tag);
    cache.field.setBorder(view, cache.field.sbs, 3);
    msg1(void, long, view, cache.field.skbt, keyboard);
    cache.field.setDel(view, c->sdg, delegate);
    ConstraintCache const *cc = &tbl->cc;
    cc->activateC(cc->init(cc->cls, cc->cr, view, 8, 1, nil, 0, 1, 44), cc->ac, true);
    tbl->view.setAcc(view, tbl->view.sacl, hint);
    CFRelease(hint);
    return view;
}

void addVStackToScrollView(VCacheRef tbl, id view, id vStack, id scrollView) {
    tbl->view.setTrans(scrollView, tbl->view.trans, false);
    tbl->view.setTrans(vStack, tbl->view.trans, false);
    tbl->view.addSub(view, tbl->view.asv, scrollView);
    tbl->view.addSub(scrollView, tbl->view.asv, vStack);
    pin(&tbl->cc, scrollView, msg0(id, view, sel_getUid("safeAreaLayoutGuide")));
    pin(&tbl->cc, vStack, scrollView);
    tbl->cc.activateC(tbl->cc.init(tbl->cc.cls, tbl->cc.cr, vStack, 7, 0, scrollView, 7, 1, 0),
                      tbl->cc.ac, true);
}

void setNavButtons(id vc, id *buttons) {
    static const char *const setters[] = {"setLeftBarButtonItem:", "setRightBarButtonItem:"};
    Class BarItem = objc_getClass("UIBarButtonItem");
    SEL itemInit = sel_getUid("initWithCustomView:");
    id navItem = msg0(id, vc, sel_getUid("navigationItem"));
    for (int i = 0; i < 2; ++i) {
        id btn = buttons[i];
        if (btn) {
            id item = msg1(id, id, Sels.alloc(BarItem, Sels.alo), itemInit, btn);
            msg1(void, id, navItem, sel_getUid(setters[i]), item);
            Sels.objRel(item, Sels.rel);
        }
    }
}

void setVCTitle(id vc, CFStringRef title) {
    msg1(void, CFStringRef, msg0(id, vc, sel_getUid("navigationItem")), sel_getUid("setTitle:"), title);
    CFRelease(title);
}

void updateSegmentedControl(CCacheRef clr, id view, unsigned char darkMode) {
    id fg = clr->getColor(clr->cls, clr->sc, ColorLabel);
    float redGreen = 0.78f, blue = 0.8f;
    if (darkMode) {
        redGreen = 0.28f;
        blue = 0.29f;
    }
    id tint = msg4(id, CGFloat, CGFloat, CGFloat, CGFloat, Sels.alloc(clr->cls, Sels.alo),
                   sel_getUid("initWithRed:green:blue:alpha:"), redGreen, redGreen, blue, 1);
    msg1(void, id, view, sel_getUid("setTintColor:"), tint);
    Sels.objRel(tint, Sels.rel);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    SEL getFont = sel_getUid("systemFontOfSize:weight:");
    const void *normalVals[] = {fg, clsF2(id, CGFloat, CGFloat,
                                          cache.fc.cls, getFont, 13, UIFontWeightRegular)};
    const void *selectedVals[] = {fg, clsF2(id, CGFloat, CGFloat,
                                            cache.fc.cls, getFont, 13, UIFontWeightMedium)};
    CFDictionaryRef normalDict = createDict(keys, normalVals, 2, NULL);
    CFDictionaryRef selectedDict = createDict(keys, selectedVals, 2, NULL);
    SEL satr = sel_getUid("setTitleTextAttributes:forState:");
    msg2(void, CFDictionaryRef, unsigned long, view, satr, normalDict, 0);
    msg2(void, CFDictionaryRef, unsigned long, view, satr, selectedDict, 4);
    CFRelease(normalDict);
    CFRelease(selectedDict);
}
