#include "Views.h"

extern CFStringRef UIFontTextStyleHeadline;
extern CGFloat UIFontWeightMedium;

struct VCache ViewTable;
Class VC;
Class View;
Class BarButtonItem;

enum {
    ButtonTypeSystem = 1
};

enum {
    StackViewAlignmentCenter = 3
};

enum {
    LineBreakByWordWrapping
};

enum {
    BorderStyleRoundedRect = 3
};

static struct PrivVData {
    const struct {
        Class cls;
        SEL si, sf;
        id (*init)(id, SEL, CFStringRef);
        id (*scaledFont)(id, SEL, id, CGFloat);
    } metrics;
    const struct {
        Class cls;
        SEL pf, sf;
        id (*preferred)(Class, SEL, CFStringRef);
        id (*system)(Class, SEL, CGFloat, CGFloat);
    } font;
    const struct {
        SEL ac;
        void (*activateConstraints)(Class, SEL, CFArrayRef);
    } con;
    const struct {
        SEL sf, safstfw, saffcsc;
    } common;
    const struct {
        Class cls;
        SEL si, sax, sal;
        id (*init)(id, SEL, CFArrayRef);
        void (*setAxis)(id, SEL, long);
        void (*setAlignment)(id, SEL, long);
    } stack;
    const struct {
        Class cls;
        SEL snl, slbm, smsf;
        void (*setFont)(id, SEL, id);
        void (*setAdjustsFontSizeToFitWidth)(id, SEL, bool);
        void (*setAdjustsFontForContentSizeCategory)(id, SEL, bool);
        void (*setNumberOfLines)(id, SEL, long);
        void (*setLineBreakMode)(id, SEL, long);
        void (*setMinimumScaleFactor)(id, SEL, CGFloat);
    } label;
    const struct {
        Class cls;
        SEL bwt, tl;
        id (*withType)(Class, SEL, long);
        id (*titleLabel)(id, SEL);
    } button;
    const struct {
        SEL stta;
        void (*setTint)(id, SEL, id);
        void (*setTitleTextAttributes)(id, SEL, CFDictionaryRef, u_long);
    } seg;
    const struct {
        Class cls;
        SEL smfs, sbs, siav;
        void (*setFont)(id, SEL, id);
        void (*setAdjustsFontSizeToFitWidth)(id, SEL, bool);
        void (*setAdjustsFontForContentSizeCategory)(id, SEL, bool);
        void (*setDel)(id, SEL, id);
        void (*setBG)(id, SEL, id);
        void (*sTextColor)(id, SEL, id);
        void (*setMinimumFontSize)(id, SEL, CGFloat);
        void (*setBorderStyle)(id, SEL, long);
        void (*setInputAccessoryView)(id, SEL, id);
    } field;
} pvc;

id getPreferredFont(CFStringRef style) {
    return pvc.font.preferred(pvc.font.cls, pvc.font.pf, style);
}

id getSystemFont(int size, CGFloat weight) {
    return pvc.font.system(pvc.font.cls, pvc.font.sf, size, weight);
}

void setFieldBackgroundColor(id field, id color) { pvc.field.setBG(field, SetBackgroundSel, color); }

void setFieldTextColor(id field, id color) {
    pvc.field.sTextColor(field, ViewTable.common.stec, color);
}

void initViewData(void (*inits[])(void)) {
    Class Font = objc_getClass("UIFont");
    SEL fpf = sel_getUid("preferredFontForTextStyle:"), fsf = sel_getUid("systemFontOfSize:weight:");

    Class Metrics = objc_getClass("UIFontMetrics");
    SEL iMet = sel_getUid("initForTextStyle:");
    SEL msf = sel_getUid("scaledFontForFont:maximumPointSize:");

    Class Constraint = objc_getClass("NSLayoutConstraint");
    SEL cncwi = sel_getUid("constraintWithItem:attribute:relatedBy:"
                           "toItem:attribute:multiplier:constant:");
    SEL cnsa = sel_getUid("setActive:"), cnsp = sel_getUid("setPriority:");
    SEL cnac = sel_getUid("activateConstraints:");

    SEL cmstic = sel_getUid("setTintColor:");
    SEL cmstamic = sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:");
    SEL cmsbc = sel_getUid("setBackgroundColor:"), cmste = sel_getUid("setText:");
    SEL cmgt = sel_getUid("text"), cmstec = sel_getUid("setTextColor:");
    SEL cmsd = sel_getUid("setDelegate:"), cmsf = sel_getUid("setFont:");
    SEL cmsafstfw = sel_getUid("setAdjustsFontSizeToFitWidth:");
    SEL cmsaffcsc = sel_getUid("setAdjustsFontForContentSizeCategory:");

    SEL vcsv = sel_getUid("setValue:forKey:");
    SEL vcgv = sel_getUid("view"), vcnc = sel_getUid("navigationController");

    SEL vgs = sel_getUid("subviews");
    SEL vas = sel_getUid("addSubview:"), vrfs = sel_getUid("removeFromSuperview");
    SEL vgl = sel_getUid("layer"), vscr = sel_getUid("setCornerRadius:"), vst = sel_getUid("setTag:");
    SEL vgt = sel_getUid("tag"), vsh = sel_getUid("setHidden:");
    SEL vslm = sel_getUid("setLayoutMargins:"), vsuie = sel_getUid("setUserInteractionEnabled:");
    SEL vsal = sel_getUid("setAccessibilityLabel:"), vsah = sel_getUid("setAccessibilityHint:");
    SEL vsav = sel_getUid("setAccessibilityValue:"), vsat = sel_getUid("setAccessibilityTraits:");
    SEL vsiae = sel_getUid("setIsAccessibilityElement:"), vvwt = sel_getUid("viewWithTag:");
    SEL vgb = sel_getUid("bounds"), vcr = sel_getUid("convertRect:fromView:");

    Class Stack = objc_getClass("UIStackView");
    SEL iStack = sel_getUid("initWithArrangedSubviews:"), staas = sel_getUid("addArrangedSubview:");
    SEL stas = sel_getUid("arrangedSubviews");
    SEL stss = sel_getUid("setSpacing:"), stscs = sel_getUid("setCustomSpacing:afterView:");
    SEL stslmra = sel_getUid("setLayoutMarginsRelativeArrangement:");
    SEL stsax = sel_getUid("setAxis:"), stsal = sel_getUid("setAlignment:");

    Class Label = objc_getClass("UILabel");
    SEL lsnl = sel_getUid("setNumberOfLines:"), lslbm = sel_getUid("setLineBreakMode:");
    SEL lsmsf = sel_getUid("setMinimumScaleFactor:");

    Class Button = objc_getClass("UIButton");
    SEL bbwt = sel_getUid("buttonWithType:");
    SEL bge = sel_getUid("isEnabled");
    SEL bat = sel_getUid("addTarget:action:forControlEvents:"), bse = sel_getUid("setEnabled:");
    SEL bst = sel_getUid("setTitle:forState:"), bstc = sel_getUid("setTitleColor:forState:");
    SEL bct = sel_getUid("currentTitle"), btl = sel_getUid("titleLabel");

    Class Seg = objc_getClass("UISegmentedControl");
    SEL sgstta = sel_getUid("setTitleTextAttributes:forState:");

    BarButtonItem = objc_getClass("UIBarButtonItem");
    SEL iItemImg = sel_getUid("initWithImage:style:target:action:");
    SEL iItemCustom = sel_getUid("initWithCustomView:");

    Class Field = objc_getClass("UITextField");
    SEL tfska = sel_getUid("setKeyboardAppearance:");
    SEL tfbfr = sel_getUid("becomeFirstResponder"), tfsmfs = sel_getUid("setMinimumFontSize:");
    SEL tfsbs = sel_getUid("setBorderStyle:"), tfsiav = sel_getUid("setInputAccessoryView:");

    Class Scroll = objc_getClass("UIScrollView");
    SEL scsci = sel_getUid("setContentInset:"), scsr = sel_getUid("scrollRectToVisible:animated:");

    memcpy(&ViewTable, &(struct VCache){
        {
            Constraint, cncwi, cnsa, cnsp,
            ((id(*)(Class, SEL, id, long, long, id, long, CGFloat, CGFloat))
             getClassMethodImp(Constraint, cncwi)),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Constraint, cnsa),
            (void(*)(id, SEL, float))class_getMethodImplementation(Constraint, cnsp)
        },
        {
            sel_getUid("setBarTintColor:"),
            cmstic,
            cmstamic, cmsbc, cmste, cmgt, cmstec, cmsd, sel_getUid("setTitle:")
        },
        {
            .sv = vcsv,
            .setVal = (void(*)(id, SEL, id, CFStringRef))class_getMethodImplementation(VC, vcsv),
            .classSize =
            class_getInstanceSize(VC), vcgv, vcnc,
            (id(*)(id, SEL))class_getMethodImplementation(VC, vcgv),
            (id(*)(id, SEL))class_getMethodImplementation(VC, vcnc)
        },
        {
            .gs = vgs, .subviews = (CFArrayRef(*)(id, SEL))class_getMethodImplementation(View, vgs),
            .classSize =
            class_getInstanceSize(View),
            vas, vrfs, vgl, vscr, vst, vgt, vsh, vslm, vsuie,
            vsal, vsah, vsav, vsat, vsiae, vvwt, vgb, vcr,
            (void(*)(id, SEL, bool))class_getMethodImplementation(View, cmstamic),
            (void(*)(id, SEL, id))class_getMethodImplementation(View, cmsbc),
            (void(*)(id, SEL, id))class_getMethodImplementation(View, vas),
            (void(*)(id, SEL))class_getMethodImplementation(View, vrfs),
            (id(*)(id, SEL))class_getMethodImplementation(View, vgl),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(objc_getClass("CALayer"), vscr),
            (void(*)(id, SEL, long))class_getMethodImplementation(View, vst),
            (long(*)(id, SEL))class_getMethodImplementation(View, vgt),
            (void(*)(id, SEL, bool))class_getMethodImplementation(View, vsh),
            (void(*)(id, SEL, HAInsets))class_getMethodImplementation(View, vslm),
            (void(*)(id, SEL, bool))class_getMethodImplementation(View, vsuie),
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(View, vsal),
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(View, vsah),
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(View, vsav),
            (void(*)(id, SEL, uint64_t))class_getMethodImplementation(View, vsat),
            (void(*)(id, SEL, bool))class_getMethodImplementation(View, vsiae),
            (id(*)(id, SEL, long))class_getMethodImplementation(View, vvwt),
            getRectMethodImplementation(View, vgb),
            getRectMethodImplementation(View, vcr, CGRect, id)
        },
        {
            staas, stas, stss, stscs, stslmra,
            (void(*)(id, SEL, bool))class_getMethodImplementation(Stack, cmstamic),
            (void(*)(id, SEL, id))class_getMethodImplementation(Stack, staas),
            (CFArrayRef(*)(id, SEL))class_getMethodImplementation(Stack, stas),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Stack, stss),
            (void(*)(id, SEL, CGFloat, id))class_getMethodImplementation(Stack, stscs),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Stack, stslmra)
        },
        {
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(Label, cmste),
            (CFStringRef(*)(id, SEL))class_getMethodImplementation(Label, cmgt),
            (void(*)(id, SEL, id))class_getMethodImplementation(Label, cmstec)
        },
        {
            .ge = bge, .getEnabled = (bool(*)(id, SEL))class_getMethodImplementation(Button, bge),
            .at =
            bat, bse, bst, bstc, bct,
            (void(*)(id, SEL, id, SEL, u_long))class_getMethodImplementation(Button, bat),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Button, bse),
            (void(*)(id, SEL, CFStringRef, u_long))class_getMethodImplementation(Button, bst),
            (void(*)(id, SEL, id, u_long))class_getMethodImplementation(Button, bstc),
            (CFStringRef(*)(id, SEL))class_getMethodImplementation(Button, bct)
        },
        {
            iItemImg, iItemCustom,
            (id(*)(id, SEL, id, long, id, SEL))class_getMethodImplementation(BarButtonItem, iItemImg),
            (id(*)(id, SEL, id))class_getMethodImplementation(BarButtonItem, iItemCustom)
        },
        {
            .ska = tfska,
            .setAppearance = (void(*)(id, SEL, long))class_getMethodImplementation(Field, tfska),
            .skt =
            sel_getUid("setKeyboardType:"), tfbfr,
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(Field, cmste),
            (CFStringRef(*)(id, SEL))class_getMethodImplementation(Field, cmgt),
            (bool(*)(id, SEL))class_getMethodImplementation(Field, tfbfr)
        },
        {
            Scroll, scsci, scsr,
            (void(*)(id, SEL, HAInsets))class_getMethodImplementation(Scroll, scsci),
            (void(*)(id, SEL, CGRect, bool))class_getMethodImplementation(Scroll, scsr)
        }
    }, sizeof(struct VCache));
    memcpy(&pvc, &(struct PrivVData){
        {
            Metrics, iMet, msf,
            (id(*)(id, SEL, CFStringRef))class_getMethodImplementation(Metrics, iMet),
            (id(*)(id, SEL, id, CGFloat))class_getMethodImplementation(Metrics, msf)
        },
        {
            Font, fpf, fsf,
            (id(*)(Class, SEL, CFStringRef))getClassMethodImp(Font, fpf),
            (id(*)(Class, SEL, CGFloat, CGFloat))getClassMethodImp(Font, fsf)
        },
        {cnac, (void(*)(Class, SEL, CFArrayRef))getClassMethodImp(Constraint, cnac)},
        {cmsf, cmsafstfw, cmsaffcsc},
        {
            Stack, iStack, stsax, stsal,
            (id(*)(id, SEL, CFArrayRef))class_getMethodImplementation(Stack, iStack),
            (void(*)(id, SEL, long))class_getMethodImplementation(Stack, stsax),
            (void(*)(id, SEL, long))class_getMethodImplementation(Stack, stsal)
        },
        {
            Label, lsnl, lslbm, lsmsf,
            (void(*)(id, SEL, id))class_getMethodImplementation(Label, cmsf),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Label, cmsafstfw),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Label, cmsaffcsc),
            (void(*)(id, SEL, long))class_getMethodImplementation(Label, lsnl),
            (void(*)(id, SEL, long))class_getMethodImplementation(Label, lslbm),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Label, lsmsf)
        },
        {
            Button, bbwt, btl,
            (id(*)(Class, SEL, long))getClassMethodImp(Button, bbwt),
            (id(*)(id, SEL))class_getMethodImplementation(Button, btl)
        },
        {
            sgstta,
            (void(*)(id, SEL, id))class_getMethodImplementation(Seg, cmstic),
            (void(*)(id, SEL, CFDictionaryRef, u_long))class_getMethodImplementation(Seg, sgstta)
        },
        {
            Field, tfsmfs, tfsbs, tfsiav,
            (void(*)(id, SEL, id))class_getMethodImplementation(Field, cmsf),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Field, cmsafstfw),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Field, cmsaffcsc),
            (void(*)(id, SEL, id))class_getMethodImplementation(Field, cmsd),
            (void(*)(id, SEL, id))class_getMethodImplementation(Field, cmsbc),
            (void(*)(id, SEL, id))class_getMethodImplementation(Field, cmstec),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Field, tfsmfs),
            (void(*)(id, SEL, long))class_getMethodImplementation(Field, tfsbs),
            (void(*)(id, SEL, id))class_getMethodImplementation(Field, tfsiav)
        }
    }, sizeof(struct PrivVData));
    inits[0]();
    inits[1]();
    inits[2]();
    inits[3]();
}

static void setupLabelFont(id label, id font) {
    pvc.label.setFont(label, pvc.common.sf, font);
    pvc.label.setAdjustsFontSizeToFitWidth(label, pvc.common.safstfw, true);
    pvc.label.setAdjustsFontForContentSizeCategory(label, pvc.common.saffcsc, true);
    pvc.label.setMinimumScaleFactor(label, pvc.label.smsf, 0.35);
}

#pragma mark - Constraints

void pin(id view, id container) {
    CFArrayRef constraints = CFArrayCreate(NULL, (const void *[]){
        makeConstraint(view, LayoutAttributeTop, 0, container, LayoutAttributeTop, 0),
        makeConstraint(view, LayoutAttributeBottom, 0, container, LayoutAttributeBottom, 0),
        makeConstraint(view, LayoutAttributeLeading, 0, container, LayoutAttributeLeading, 0),
        makeConstraint(view, LayoutAttributeTrailing, 0, container, LayoutAttributeTrailing, 0)
    }, 4, NULL);
    pvc.con.activateConstraints(ViewTable.con.cls, pvc.con.ac, constraints);
    CFRelease(constraints);
}

#pragma mark - View Initializers

static id createStackCommon(id const *subviews, int count) {
    CFArrayRef views = CFArrayCreate(NULL, (const void **)subviews, count, NULL);
    id stack = pvc.stack.init(alloc(pvc.stack.cls), pvc.stack.si, views);
    CFRelease(views);
    return stack;
}

id createHStack(id const *subviews) {
    id stack = createStackCommon(subviews, 2);
    pvc.stack.setAlignment(stack, pvc.stack.sal, StackViewAlignmentCenter);
    setSpacing(stack, 8);
    return stack;
}

id createVStack(id const *subviews, int count) {
    id stack = createStackCommon(subviews, count);
    pvc.stack.setAxis(stack, pvc.stack.sax, ConstraintAxisVertical);
    return stack;
}

id createLabel(CFStringRef text, CFStringRef style, int textColor) {
    id label = new(pvc.label.cls);
    if (text) {
        setText(label, text);
        CFRelease(text);
    }
    setupLabelFont(label, pvc.font.preferred(pvc.font.cls, pvc.font.pf, style));
    setTextColor(label, getColor(textColor));
    pvc.label.setNumberOfLines(label, pvc.label.snl, 0);
    pvc.label.setLineBreakMode(label, pvc.label.slbm, LineBreakByWordWrapping);
    return label;
}

id createButton(CFStringRef title, int titleColor, id target, SEL action) {
    id button = pvc.button.withType(pvc.button.cls, pvc.button.bwt, ButtonTypeSystem);
    setTitle(button, title, ControlStateNormal);
    CFRelease(title);
    setTitleColor(button, getColor(titleColor), ControlStateNormal);
    setTitleColor(button, getColor(ColorDisabled), ControlStateDisabled);
    addTarget(button, target, action, ControlEventTouchUpInside);

    CFStringRef style = UIFontTextStyleBody;
    CGFloat weight = UIFontWeightRegular;
    if (titleColor == ColorLabel) {
        style = UIFontTextStyleHeadline;
        weight = UIFontWeightSemibold;
    }

    id metrics = pvc.metrics.init(alloc(pvc.metrics.cls), pvc.metrics.si, style);
    id baseFont = pvc.font.system(pvc.font.cls, pvc.font.sf, 17, weight);
    id scaledFont = pvc.metrics.scaledFont(metrics, pvc.metrics.sf, baseFont, 23);
    setupLabelFont(pvc.button.titleLabel(button, pvc.button.tl), scaledFont);
    releaseObject(metrics);
    return button;
}

id createSegmentedControl(CFStringRef format, int startIndex) {
    CFStringRef segments[3];
    fillStringArray(segments, format, 3);
    CFArrayRef segs = CFArrayCreate(NULL, (const void **)segments, 3, NULL);
    id control = msgV(objSig(id, CFArrayRef), alloc(objc_getClass("UISegmentedControl")),
                      sel_getUid("initWithItems:"), segs);
    setSelectedSegmentIndex(control, startIndex);
    CFRelease(segs);
    for (int i = 0; i < 3; ++i) {
        CFRelease(segments[i]);
    }
    return control;
}

id createTextField(id delegate, id accessory, CFStringRef hint, int tag) {
    id field = new(pvc.field.cls);
    setTag(field, tag);
    pvc.field.setBG(field, SetBackgroundSel, getColor(ColorTertiaryBG));
    id font = pvc.font.preferred(pvc.font.cls, pvc.font.pf, UIFontTextStyleBody);
    pvc.field.setFont(field, pvc.common.sf, font);
    pvc.field.setAdjustsFontSizeToFitWidth(field, pvc.common.safstfw, true);
    pvc.field.setAdjustsFontForContentSizeCategory(field, pvc.common.saffcsc, true);
    pvc.field.setMinimumFontSize(field, pvc.field.smfs, 14);
    pvc.field.sTextColor(field, ViewTable.common.stec, getColor(ColorLabel));
    pvc.field.setBorderStyle(field, pvc.field.sbs, BorderStyleRoundedRect);
    pvc.field.setDel(field, ViewTable.common.sd, delegate);
    pvc.field.setInputAccessoryView(field, pvc.field.siav, accessory);
    setHeight(field, ViewHeightDefault, true, false);
    setAccessibilityLabel(field, hint);
    CFRelease(hint);
    return field;
}

#pragma mark - Color Updates

void updateSegmentedControlColors(id control, bool darkMode) {
    float redGreen = 0.78f, blue = 0.8f;
    if (darkMode) {
        redGreen = 0.28f;
        blue = 0.29f;
    }
    id tint = createColor(redGreen, redGreen, blue, 1), foreground = getColor(ColorLabel);
    pvc.seg.setTint(control, ViewTable.common.stic, tint);
    releaseObject(tint);

    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    CFDictionaryRef normalDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        foreground, pvc.font.system(pvc.font.cls, pvc.font.sf, FontSizeSmall, UIFontWeightRegular)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef selectedDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        foreground, pvc.font.system(pvc.font.cls, pvc.font.sf, FontSizeSmall, UIFontWeightMedium)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    pvc.seg.setTitleTextAttributes(control, pvc.seg.stta, normalDict, ControlStateNormal);
    pvc.seg.setTitleTextAttributes(control, pvc.seg.stta, selectedDict, ControlStateSelected);
    CFRelease(normalDict);
    CFRelease(selectedDict);
}
