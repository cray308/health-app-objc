#include "Views.h"

extern CGFloat UIFontWeightRegular;

struct VCache ViewTable;
Class VC;
Class View;
Class Button;
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
        SEL sf;
        id (*system)(Class, SEL, CGFloat, CGFloat);
    } font;
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
        void (*sTextAlignment)(id, SEL, long);
        void (*setMinimumScaleFactor)(id, SEL, CGFloat);
    } label;
    const struct {
        SEL bwt, tl;
        id (*withType)(Class, SEL, long);
        id (*titleLabel)(id, SEL);
    } button;
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

void initViewData(void (*inits[])(void), int count) {
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
    SEL cmsta = sel_getUid("setTextAlignment:"), cmsd = sel_getUid("setDelegate:");
    SEL cmsf = sel_getUid("setFont:"), cmsafstfw = sel_getUid("setAdjustsFontSizeToFitWidth:");
    SEL cmsaffcsc = sel_getUid("setAdjustsFontForContentSizeCategory:");

    SEL vcgv = sel_getUid("view"), vcnc = sel_getUid("navigationController");

    SEL vas = sel_getUid("addSubview:");
    SEL vgl = sel_getUid("layer"), vscr = sel_getUid("setCornerRadius:"), vst = sel_getUid("setTag:");
    SEL vgt = sel_getUid("tag"), vsh = sel_getUid("setHidden:"), vsa = sel_getUid("setAlpha:");
    SEL vschp = sel_getUid("setContentHuggingPriority:forAxis:");
    SEL vsccrp = sel_getUid("setContentCompressionResistancePriority:forAxis:");
    SEL vslm = sel_getUid("setLayoutMargins:"), vsuie = sel_getUid("setUserInteractionEnabled:");
    SEL vsal = sel_getUid("setAccessibilityLabel:"), vsah = sel_getUid("setAccessibilityHint:");
    SEL vsav = sel_getUid("setAccessibilityValue:"), vsat = sel_getUid("setAccessibilityTraits:");
    SEL vsiae = sel_getUid("setIsAccessibilityElement:"), vvwt = sel_getUid("viewWithTag:");
    SEL vgb = sel_getUid("bounds"), vcr = sel_getUid("convertRect:fromView:");

    Class Stack = objc_getClass("UIStackView");
    SEL iStack = sel_getUid("initWithArrangedSubviews:"), staas = sel_getUid("addArrangedSubview:");
    SEL stss = sel_getUid("setSpacing:"), stscs = sel_getUid("setCustomSpacing:afterView:");
    SEL stslmra = sel_getUid("setLayoutMarginsRelativeArrangement:");
    SEL stsax = sel_getUid("setAxis:"), stsal = sel_getUid("setAlignment:");

    Class Label = objc_getClass("UILabel");
    SEL lsnl = sel_getUid("setNumberOfLines:"), lslbm = sel_getUid("setLineBreakMode:");
    SEL lsmsf = sel_getUid("setMinimumScaleFactor:");

    Button = objc_getClass("UIButton");
    SEL bbwt = sel_getUid("buttonWithType:");
    SEL bge = sel_getUid("isEnabled"), bsnl = sel_getUid("setNeedsLayout");
    SEL bat = sel_getUid("addTarget:action:forControlEvents:"), bse = sel_getUid("setEnabled:");
    SEL bst = sel_getUid("setTitle:forState:"), bstc = sel_getUid("setTitleColor:forState:");
    SEL bct = sel_getUid("currentTitle"), btl = sel_getUid("titleLabel");

    BarButtonItem = objc_getClass("UIBarButtonItem");
    SEL iItemImg = sel_getUid("initWithImage:style:target:action:");
    SEL iItemCustom = sel_getUid("initWithCustomView:");

    Class Field = objc_getClass("UITextField");
    SEL tfbfr = sel_getUid("becomeFirstResponder"), tfsmfs = sel_getUid("setMinimumFontSize:");
    SEL tfsbs = sel_getUid("setBorderStyle:"), tfsiav = sel_getUid("setInputAccessoryView:");

    Class Scroll = objc_getClass("UIScrollView");
    SEL scsci = sel_getUid("setContentInset:"), scsr = sel_getUid("scrollRectToVisible:animated:");

    memcpy(&ViewTable, &(struct VCache){
        {Font, fpf, (id(*)(Class, SEL, CFStringRef))getClassMethodImp(Font, fpf)},
        {
            Constraint, cncwi, cnac, cnsa, cnsp,
            ((id(*)(Class, SEL, id, long, long, id, long, CGFloat, CGFloat))
             getClassMethodImp(Constraint, cncwi)),
            (void(*)(Class, SEL, CFArrayRef))getClassMethodImp(Constraint, cnac),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Constraint, cnsa),
            (void(*)(id, SEL, float))class_getMethodImplementation(Constraint, cnsp)
        },
        {
            cmstic,
            cmstamic, cmsbc, cmste, cmgt, cmstec, cmsta, cmsd, sel_getUid("setTitle:")
        },
        {
            class_getInstanceSize(VC), vcgv, vcnc,
            (id(*)(id, SEL))class_getMethodImplementation(VC, vcgv),
            (id(*)(id, SEL))class_getMethodImplementation(VC, vcnc)
        },
        {
            class_getInstanceSize(View),
            vas, vgl, vscr, vst, vgt, vsh, vsa, vschp, vsccrp, vslm, vsuie,
            vsal, vsah, vsav, vsat, vsiae, vvwt, vgb, vcr,
            (void(*)(id, SEL, bool))class_getMethodImplementation(View, cmstamic),
            (void(*)(id, SEL, id))class_getMethodImplementation(View, cmsbc),
            (void(*)(id, SEL, id))class_getMethodImplementation(View, vas),
            (id(*)(id, SEL))class_getMethodImplementation(View, vgl),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(objc_getClass("CALayer"), vscr),
            (void(*)(id, SEL, long))class_getMethodImplementation(View, vst),
            (long(*)(id, SEL))class_getMethodImplementation(View, vgt),
            (void(*)(id, SEL, bool))class_getMethodImplementation(View, vsh),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(View, vsa),
            (void(*)(id, SEL, float, long))class_getMethodImplementation(View, vschp),
            (void(*)(id, SEL, float, long))class_getMethodImplementation(View, vsccrp),
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
            staas, stss, stscs, stslmra,
            (void(*)(id, SEL, bool))class_getMethodImplementation(Stack, cmstamic),
            (void(*)(id, SEL, id))class_getMethodImplementation(Stack, staas),
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
            bat, bse, bge, bst, bstc, bsnl, bct,
            (void(*)(id, SEL, id, SEL, u_long))class_getMethodImplementation(Button, bat),
            (void(*)(id, SEL, bool))class_getMethodImplementation(Button, bse),
            (bool(*)(id, SEL))class_getMethodImplementation(Button, bge),
            (void(*)(id, SEL, CFStringRef, u_long))class_getMethodImplementation(Button, bst),
            (void(*)(id, SEL, id, u_long))class_getMethodImplementation(Button, bstc),
            (void(*)(id, SEL))class_getMethodImplementation(Button, bsnl),
            (CFStringRef(*)(id, SEL))class_getMethodImplementation(Button, bct)
        },
        {
            iItemImg, iItemCustom,
            (id(*)(id, SEL, id, long, id, SEL))class_getMethodImplementation(BarButtonItem, iItemImg),
            (id(*)(id, SEL, id))class_getMethodImplementation(BarButtonItem, iItemCustom)
        },
        {
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
        {fsf, (id(*)(Class, SEL, CGFloat, CGFloat))getClassMethodImp(Font, fsf)},
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
            (void(*)(id, SEL, long))class_getMethodImplementation(Label, cmsta),
            (void(*)(id, SEL, CGFloat))class_getMethodImplementation(Label, lsmsf)
        },
        {
            bbwt, btl,
            (id(*)(Class, SEL, long))getClassMethodImp(Button, bbwt),
            (id(*)(id, SEL))class_getMethodImplementation(Button, btl)
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

    for (int i = 0; i < count; ++i) {
        inits[i]();
    }
}

static void setupLabelFont(id label, id font) {
    pvc.label.setFont(label, pvc.common.sf, font);
    pvc.label.setAdjustsFontSizeToFitWidth(label, pvc.common.safstfw, true);
    pvc.label.setAdjustsFontForContentSizeCategory(label, pvc.common.saffcsc, true);
    pvc.label.setMinimumScaleFactor(label, pvc.label.smsf, 0.35);
}

static void configureMultilineLabel(id label, id font) {
    setupLabelFont(label, font);
    pvc.label.setNumberOfLines(label, pvc.label.snl, 0);
    pvc.label.setLineBreakMode(label, pvc.label.slbm, LineBreakByWordWrapping);
}

#pragma mark - Constraints

void pin(id view, id container) {
    CFArrayRef constraints = CFArrayCreate(NULL, (const void *[]){
        makeConstraint(view, LayoutAttributeTop, 0, container, LayoutAttributeTop, 0),
        makeConstraint(view, LayoutAttributeBottom, 0, container, LayoutAttributeBottom, 0),
        makeConstraint(view, LayoutAttributeLeading, 0, container, LayoutAttributeLeading, 0),
        makeConstraint(view, LayoutAttributeTrailing, 0, container, LayoutAttributeTrailing, 0)
    }, 4, NULL);
    activateConstraints(constraints);
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
    setTextColor(label, getColor(textColor));
    configureMultilineLabel(label, getPreferredFont(style));
    return label;
}

static void configureMultilineButton(id button, CFStringRef style) {
    id label = pvc.button.titleLabel(button, pvc.button.tl);
    configureMultilineLabel(label, getPreferredFont(style));
    pvc.label.sTextAlignment(label, ViewTable.common.sta, TextAlignmentCenter);
    setContentHuggingPriority(button, 249, ConstraintAxisVertical);
    setContentCompressionResistancePriority(button, 745, ConstraintAxisVertical);
    CFArrayRef constraints = CFArrayCreate(NULL, (const void *[]){
        makeConstraint(label, LayoutAttributeTop, 1, button, LayoutAttributeTop, 0),
        makeConstraint(label, LayoutAttributeBottom, -1, button, LayoutAttributeBottom, 0),
        makeConstraint(label, LayoutAttributeLeading, 1, button, LayoutAttributeLeading, 0),
        makeConstraint(label, LayoutAttributeTrailing, -1, button, LayoutAttributeTrailing, 0)
    }, 4, NULL);
    activateConstraints(constraints);
    CFRelease(constraints);
}

id createButton(CFStringRef title, int titleColor, bool singleLine, id target, SEL action) {
    id button = pvc.button.withType(Button, pvc.button.bwt, ButtonTypeSystem);
    setTitle(button, title, ControlStateNormal);
    CFRelease(title);
    setTitleColor(button, getColor(titleColor), ControlStateNormal);
    setTitleColor(button, getColor(ColorDisabled), ControlStateDisabled);
    addTarget(button, target, action, ControlEventTouchUpInside);

    if (singleLine) {
        id metrics = pvc.metrics.init(alloc(pvc.metrics.cls), pvc.metrics.si, UIFontTextStyleBody);
        id baseFont = pvc.font.system(ViewTable.font.cls, pvc.font.sf, 17, UIFontWeightRegular);
        id scaledFont = pvc.metrics.scaledFont(metrics, pvc.metrics.sf, baseFont, 23);
        setupLabelFont(pvc.button.titleLabel(button, pvc.button.tl), scaledFont);
        releaseObject(metrics);
    } else {
        configureMultilineButton(button, UIFontTextStyleBody);
    }
    return button;
}

id createCustomButton(Class ButtonClass) {
    id button = pvc.button.withType(ButtonClass, pvc.button.bwt, ButtonTypeSystem);
    setTitleColor(button, getColor(ColorLabel), ControlStateNormal);
    setTitleColor(button, getColor(ColorDisabled), ControlStateDisabled);
    configureMultilineButton(button, UIFontTextStyleHeadline);
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
    pvc.field.setFont(field, pvc.common.sf, getPreferredFont(UIFontTextStyleBody));
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
