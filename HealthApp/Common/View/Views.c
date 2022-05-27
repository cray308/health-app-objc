#include "Views.h"

#define setFont(v, f) msg1(void, id, (v), pvc.lc.sf, (f))
#define adjustFontCategory(v, a) msg1(void, bool, (v), pvc.lc.sajfc, (a))

extern CGFloat UIFontWeightMedium;

struct VCache Vtbl;
Class VC;
Class View;
size_t VCSize;
size_t ViewSize;

enum {
    UIViewAutoresizingFlexibleHeight = 16
};

enum {
    UIButtonTypeSystem = 1
};

enum {
    UIStackViewAlignmentCenter = 3
};

enum {
    NSLineBreakByWordWrapping
};

enum {
    UITextBorderStyleRoundedRect = 3
};

static struct PrivVData {
    const struct FontData {
        Class cls;
        SEL pf;
        id (*pref)(Class,SEL,CFStringRef);
    } fc;
    const struct ConstraintData {
        SEL aar;
        void (*activateArr)(Class,SEL,CFArrayRef);
    } cc;
    const struct StackData {
        Class cls;
        SEL si, sa, saln, slmra;
        id (*init)(id,SEL,CFArrayRef);
        void (*setAxis)(id,SEL,long);
        void (*setAlign)(id,SEL,long);
        void (*setRelative)(id,SEL,bool);
    } sc;
    const struct LabelData {
        Class cls;
        SEL sf, sajfc, snl, slbm, sajfs, smsf;
        void (*setLines)(id,SEL,long);
        void (*setMode)(id,SEL,long);
        void (*adjFontWidth)(id,SEL,bool);
        void (*setScale)(id,SEL,CGFloat);
    } lc;
    const struct ButtonData {
        Class cls;
        SEL si, glb;
        id (*init)(Class,SEL,long);
        id (*getLabel)(id,SEL);
    } bc;
    const struct FieldData {
        Class cls;
        SEL sbs, siac;
        void (*setBorder)(id,SEL,long);
        void (*setInput)(id,SEL,id);
    } tfc;
} pvc;

void initViewData(void (*inits[])(void)) {
    Class Font = objc_getClass("UIFont");
    SEL pf = sel_getUid("preferredFontForTextStyle:");

    Class Con = objc_getClass("NSLayoutConstraint");
    SEL lp = sel_getUid("setPriority:");
    SEL cr = sel_getUid("constraintWithItem:attribute:relatedBy:toItem:attribute:multiplier:constant:");
    SEL ac = sel_getUid("setActive:"), aar = sel_getUid("activateConstraints:");

    SEL glyr = sel_getUid("layer"), gtg = sel_getUid("tag"), stg = sel_getUid("setTag:");
    SEL asv = sel_getUid("addSubview:"), rsv = sel_getUid("removeFromSuperview");
    SEL shd = sel_getUid("setHidden:"), lfn = sel_getUid("layoutIfNeeded");
    SEL sacl = sel_getUid("setAccessibilityLabel:"), shn = sel_getUid("setAccessibilityHint:");
    SEL satrs = sel_getUid("setAccessibilityTraits:"), sace = sel_getUid("setIsAccessibilityElement:");
    SEL scrad = sel_getUid("setCornerRadius:"), vwt = sel_getUid("viewWithTag:");

    Class Stack = objc_getClass("UIStackView");
    SEL istack = sel_getUid("initWithArrangedSubviews:");
    SEL aasv = sel_getUid("addArrangedSubview:"), gsv = sel_getUid("arrangedSubviews");
    SEL smr = sel_getUid("setLayoutMargins:"), saln = sel_getUid("setAlignment:");
    SEL ssp = sel_getUid("setSpacing:"), slmra = sel_getUid("setLayoutMarginsRelativeArrangement:");
    SEL sa = sel_getUid("setAxis:"), scsp = sel_getUid("setCustomSpacing:afterView:");

    Class Label = objc_getClass("UILabel");
    SEL sajfs = sel_getUid("setAdjustsFontSizeToFitWidth:"), slbm = sel_getUid("setLineBreakMode:");
    SEL smsf = sel_getUid("setMinimumScaleFactor:"), snl = sel_getUid("setNumberOfLines:");

    Class Btn = objc_getClass("UIButton");
    SEL ibtn = sel_getUid("buttonWithType:"), gct = sel_getUid("currentTitle");
    SEL atgt = sel_getUid("addTarget:action:forControlEvents:"), en = sel_getUid("setEnabled:");
    SEL sbtxt = sel_getUid("setTitle:forState:"), sbc = sel_getUid("setTitleColor:forState:");
    SEL glb = sel_getUid("titleLabel");

    Class Field = objc_getClass("UITextField");
    SEL sbs = sel_getUid("setBorderStyle:"), siac = sel_getUid("setInputAccessoryView:");
    SEL bfr = sel_getUid("becomeFirstResponder");

    Class Scroll = objc_getClass("UIScrollView");
    SEL sci = sel_getUid("setContentInset:"), scrvs = sel_getUid("scrollRectToVisible:animated:");

    memcpy(&Vtbl, &(struct VCache){
        {Con, cr, ac, lp,
            (id(*)(Class,SEL,id,long,long,id,long,CGFloat,CGFloat))getImpC(Con, cr),
            (void(*)(id,SEL,bool))getImpO(Con, ac), (void(*)(id,SEL,float))getImpO(Con, lp)
        },
        {sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), sel_getUid("setBackgroundColor:"),
            sel_getUid("bounds"), sel_getUid("convertRect:fromView:"), asv, rsv, glyr, scrad,
            stg, gtg, shd, sacl, shn, satrs, sace, vwt, lfn, (void(*)(id,SEL,id))getImpO(View, asv),
            (void(*)(id,SEL))getImpO(View, rsv), (id(*)(id,SEL))getImpO(View, glyr),
            (void(*)(id,SEL,CGFloat))getImpO(clsF0(Class, View, sel_getUid("layerClass")), scrad),
            (void(*)(id,SEL,long))getImpO(View, stg), (long(*)(id,SEL))getImpO(View, gtg),
            (void(*)(id,SEL,bool))getImpO(View, shd),
            (void(*)(id,SEL,CFStringRef))getImpO(View, sacl),
            (void(*)(id,SEL,CFStringRef))getImpO(View, shn),
            (void(*)(id,SEL,uint64_t))getImpO(View, satrs), (void(*)(id,SEL,bool))getImpO(View, sace),
            (id(*)(id,SEL,long))getImpO(View, vwt), (void(*)(id,SEL))getImpO(View, lfn)
        },
        {aasv, gsv, smr, ssp, scsp, (void(*)(id,SEL,id))getImpO(Stack, aasv),
            (CFArrayRef(*)(id,SEL))getImpO(Stack, gsv), (void(*)(id,SEL,HAInsets))getImpO(Stack, smr),
            (void(*)(id,SEL,CGFloat))getImpO(Stack, ssp),
            (void(*)(id,SEL,CGFloat,id))getImpO(Stack, scsp)
        },
        {sel_getUid("setText:"), sel_getUid("text"), sel_getUid("setTextColor:")},
        {atgt, en, sbtxt, gct, sbc, (void(*)(id,SEL,id,SEL,u_long))getImpO(Btn, atgt),
            (void(*)(id,SEL,bool))getImpO(Btn, en),
            (void(*)(id,SEL,CFStringRef,u_long))getImpO(Btn, sbtxt),
            (CFStringRef(*)(id,SEL))getImpO(Btn, gct), (void(*)(id,SEL,id,u_long))getImpO(Btn, sbc)
        },
        {sel_getUid("setDelegate:"), sel_getUid("setKeyboardType:"), bfr,
            (bool(*)(id,SEL))getImpO(Field, bfr)
        },
        {sci, scrvs, (void(*)(id,SEL,HAInsets))getImpO(Scroll, sci),
            (void(*)(id,SEL,CGRect,bool))getImpO(Scroll, scrvs)
        }
    }, sizeof(struct VCache));
    memcpy(&pvc, &(struct PrivVData){
        {Font, pf, (id(*)(Class,SEL,CFStringRef))getImpC(Font, pf)},
        {aar, (void(*)(Class,SEL,CFArrayRef))getImpC(Con, aar)},
        {Stack, istack, sa, saln, slmra, (id(*)(id,SEL,CFArrayRef))getImpO(Stack, istack),
            (void(*)(id,SEL,long))getImpO(Stack, sa), (void(*)(id,SEL,long))getImpO(Stack, saln),
            (void(*)(id,SEL,bool))getImpO(Stack, slmra)
        },
        {Label, sel_getUid("setFont:"), sel_getUid("setAdjustsFontForContentSizeCategory:"),
            snl, slbm, sajfs, smsf, (void(*)(id,SEL,long))getImpO(Label, snl),
            (void(*)(id,SEL,long))getImpO(Label, slbm), (void(*)(id,SEL,bool))getImpO(Label, sajfs),
            (void(*)(id,SEL,CGFloat))getImpO(Label, smsf)
        },
        {Btn, ibtn, glb, (id(*)(Class,SEL,long))getImpC(Btn, ibtn), (id(*)(id,SEL))getImpO(Btn, glb)},
        {Field, sbs, siac, (void(*)(id,SEL,long))getImpO(Field, sbs),
            (void(*)(id,SEL,id))getImpO(Field, siac)
        }
    }, sizeof(struct PrivVData));
    inits[0]();
    inits[1]();
}

static void setDynamicFont(id view, bool accessibilitySize) {
    pvc.lc.adjFontWidth(view, pvc.lc.sajfs, true);
    adjustFontCategory(view, accessibilitySize);
    pvc.lc.setScale(view, pvc.lc.smsf, 0.35);
}

#pragma mark - Constraints

void pin(id view, id container) {
    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){
        makeConstraint(view, NSLayoutAttributeTop, 0, container, NSLayoutAttributeTop, 0),
        makeConstraint(view, NSLayoutAttributeBottom, 0, container, NSLayoutAttributeBottom, 0),
        makeConstraint(view, NSLayoutAttributeLeading, 0, container, NSLayoutAttributeLeading, 0),
        makeConstraint(view, NSLayoutAttributeTrailing, 0, container, NSLayoutAttributeTrailing, 0)
    }, 4, NULL);
    pvc.cc.activateArr(Vtbl.cc.cls, pvc.cc.aar, array);
    CFRelease(array);
}

void setHeight(id view, int height, bool geq, bool optional) {
    id constraint = makeConstraint(view, NSLayoutAttributeHeight, geq, nil, 0, height);
    if (optional) lowerPriority(constraint);
    setActive(constraint);
}

#pragma mark - View Initializers

static id createStackCommon(id *subviews, int count) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)subviews, count, NULL);
    id view = pvc.sc.init(alloc(pvc.sc.cls), pvc.sc.si, arr);
    CFRelease(arr);
    pvc.sc.setRelative(view, pvc.sc.slmra, true);
    return view;
}

id createHStack(id *subviews) {
    id view = createStackCommon(subviews, 2);
    pvc.sc.setAlign(view, pvc.sc.saln, UIStackViewAlignmentCenter);
    setSpacing(view, 8);
    return view;
}

id createVStack(id *subviews, int count) {
    id view = createStackCommon(subviews, count);
    pvc.sc.setAxis(view, pvc.sc.sa, UILayoutConstraintAxisVertical);
    return view;
}

id createScrollView(void) {
    id view = new(objc_getClass("UIScrollView"));
    msg1(void, long, view, sel_getUid("setAutoresizingMask:"), UIViewAutoresizingFlexibleHeight);
    return view;
}

id createLabel(CFStringRef text, CFStringRef style, int color) {
    id view = new(pvc.lc.cls);
    setText(view, text);
    if (text) CFRelease(text);
    setFont(view, pvc.fc.pref(pvc.fc.cls, pvc.fc.pf, style));
    setDynamicFont(view, true);
    setTextColor(view, getColor(color));
    pvc.lc.setLines(view, pvc.lc.snl, 0);
    pvc.lc.setMode(view, pvc.lc.slbm, NSLineBreakByWordWrapping);
    return view;
}

id createButton(CFStringRef title, int color, CFStringRef style, id target, SEL action) {
    id view = pvc.bc.init(pvc.bc.cls, pvc.bc.si, UIButtonTypeSystem);
    setBtnTitle(view, title);
    if (title) CFRelease(title);
    setTitleColor(view, getColor(color), UIControlStateNormal);
    setTitleColor(view, getColor(ColorDisabled), UIControlStateDisabled);
    id label = pvc.bc.getLabel(view, pvc.bc.glb);
    setFont(label, pvc.fc.pref(pvc.fc.cls, pvc.fc.pf, style));
    setDynamicFont(label, false);
    addTarget(view, target, action, UIControlEventTouchUpInside);
    return view;
}

id createSegmentedControl(CFStringRef format, int startIndex) {
    CFStringRef segments[3];
    fillStringArray(segments, format, 3);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, 3, NULL);
    id _v = alloc(objc_getClass("UISegmentedControl"));
    id view = msg1(id, CFArrayRef, _v, sel_getUid("initWithItems:"), array);
    msg1(void, long, view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    CFRelease(array);
    CFRelease(segments[0]);
    CFRelease(segments[1]);
    CFRelease(segments[2]);
    return view;
}

id createTextfield(id delegate, id accessory, CFStringRef hint, int tag) {
    id view = new(pvc.tfc.cls);
    setTag(view, tag);
    setBackgroundColor(view, getColor(ColorTertiaryBG));
    setFont(view, pvc.fc.pref(pvc.fc.cls, pvc.fc.pf, UIFontTextStyleBody));
    setTextColor(view, getColor(ColorLabel));
    adjustFontCategory(view, true);
    pvc.tfc.setBorder(view, pvc.tfc.sbs, UITextBorderStyleRoundedRect);
    setDelegate(view, delegate);
    pvc.tfc.setInput(view, pvc.tfc.siac, accessory);
    setHeight(view, ViewHeightDefault, true, false);
    setAccessibilityLabel(view, hint);
    CFRelease(hint);
    return view;
}

#pragma mark - Color Updates

void updateButtonColors(id view, int color) {
    setTitleColor(view, getColor(color), UIControlStateNormal);
    setTitleColor(view, getColor(ColorDisabled), UIControlStateDisabled);
    setBackgroundColor(view, getColor(ColorSecondaryBGGrouped));
}

void updateSegmentedControl(id view, bool darkMode) {
    id fg = getColor(ColorLabel);
    float rg = 0.78f, blue = 0.8f;
    if (darkMode) {
        rg = 0.28f;
        blue = 0.29f;
    }
    id tint = (((id(*)(id,SEL,CGFloat,CGFloat,CGFloat,CGFloat))objc_msgSend)
               (alloc(UIColor), sel_getUid("initWithRed:green:blue:alpha:"), rg, rg, blue, 1));
    msg1(void, id, view, sel_getUid("setTintColor:"), tint);
    releaseO(tint);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    SEL gf = sel_getUid("systemFontOfSize:weight:");
    const void *nv[] = {
        fg, ((id(*)(Class,SEL,CGFloat,CGFloat))objc_msgSend)(pvc.fc.cls, gf,
                                                             FontSizeReg, UIFontWeightRegular)
    };
    const void *sv[] = {
        fg, ((id(*)(Class,SEL,CGFloat,CGFloat))objc_msgSend)(pvc.fc.cls, gf,
                                                             FontSizeReg, UIFontWeightMedium)
    };
    CFDictionaryRef normalDict = CFDictionaryCreate(
      NULL, keys, nv, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef selectedDict = CFDictionaryCreate(
      NULL, keys, sv, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    SEL satr = sel_getUid("setTitleTextAttributes:forState:");
    msg2(void, CFDictionaryRef, u_long, view, satr, normalDict, UIControlStateNormal);
    msg2(void, CFDictionaryRef, u_long, view, satr, selectedDict, UIControlStateSelected);
    CFRelease(normalDict);
    CFRelease(selectedDict);
}
