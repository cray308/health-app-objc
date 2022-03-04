#ifndef Views_h
#define Views_h

#include <objc/objc.h>
#include <CoreGraphics/CGGeometry.h>
#include <CoreFoundation/CFBundle.h>

#define createObjectWithFrame(cls, f) msg1(id,CGRect,allocClass(cls),sel_getUid("initWithFrame:"),f)

#define getPreferredFont(style)\
 clsF1(id,CFStringRef,FontClass,sel_getUid("preferredFontForTextStyle:"),(CFStringRef)style)
#define getSystemFont(size, weight)\
 clsF2(id,CGFloat,CGFloat,FontClass,sel_getUid("systemFontOfSize:weight:"),size,weight)
#define setFont(v, f) msg1(void,id,v,sel_getUid("setFont:"),f)

#define setUsesAutolayout(v)\
 msg1(void,bool,v,sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"),false)
#define createConstraint(v1, a1, r, v2, a2, c) \
(((id(*)(Class,SEL,id,long,long,id,long,CGFloat,CGFloat))objc_msgSend)\
(ConstraintCls,sel_getUid("constraintWithItem:attribute:relatedBy:toItem:attribute:multiplier:constant:"),\
(v1), (a1), (r), (v2), (a2), 1, (c)))
#define lowerPriority(c) msg1(void,float,c,sel_getUid("setPriority:"),999)
#define activateConstraint(c) msg1(void,bool,c,sel_getUid("setActive:"),true)
#define activateConstraintsArray(a)\
 clsF1(id,CFArrayRef,ConstraintCls,sel_getUid("activateConstraints:"),a)

#define addSubview(v, sv) msg1(void,id,v,sel_getUid("addSubview:"),sv)
#define getSubviews(v) msg0(CFArrayRef,v,sel_getUid("subviews"))
#define removeView(v) msg0(void,v,sel_getUid("removeFromSuperview"))

#define addArrangedSubview(v, sv) msg1(void,id,v,sel_getUid("addArrangedSubview:"),sv)
#define getArrangedSubviews(_sv) msg0(CFArrayRef,_sv,sel_getUid("arrangedSubviews"))
#define setMargins(sv, val) msg1(void,HAInsets,sv,sel_getUid("setLayoutMargins:"),val)
#define centerHStack(v) msg1(void,long,v,sel_getUid("setAlignment:"),3)

#define getLayer(v) msg0(id,v,sel_getUid("layer"))

#define setTag(v, tag) msg1(void,long,v,sel_getUid("setTag:"),tag)
#define getTag(v) msg0(long,v,sel_getUid("tag"))

#define getSelectedSegment(v) msg0(long,v,sel_getUid("selectedSegmentIndex"))
#define setSelectedSegment(v, i) msg1(void,long,v,sel_getUid("setSelectedSegmentIndex:"),i)

#define hideView(v, hide) msg1(void,bool,v,sel_getUid("setHidden:"),hide)
#define setBackground(v, color) msg1(void,id,v,sel_getUid("setBackgroundColor:"),color)
#define setTintColor(v, color) msg1(void,id,v,sel_getUid("setTintColor:"),color)

#define setAccessibilityLabel(v,t) msg1(void,CFStringRef,v,sel_getUid("setAccessibilityLabel:"),t)
#define setLabelText(v, t) msg1(void,CFStringRef,v,sel_getUid("setText:"),t)
#define getText(v) msg0(CFStringRef,v,sel_getUid("text"))
#define getTitleLabel(v) msg0(id,v,sel_getUid("titleLabel"))
#define setTextColor(v, color) msg1(void,id,v,sel_getUid("setTextColor:"),color)

#define setDelegate(v, d) msg1(void,id,v,sel_getUid("setDelegate:"),d)
#define setInputAccessory(v, a) msg1(void,id,v,sel_getUid("setInputAccessoryView:"),a)
#define setKBColor(v, a) msg1(void,long,v,sel_getUid("setKeyboardAppearance:"),a)

#define addTarget(v, t, a, e)\
 msg3(void,id,SEL,unsigned long,v,sel_getUid("addTarget:action:forControlEvents:"),t,a,e)
#define enableButton(v, enabled) msg1(void,bool,v,sel_getUid("setEnabled:"),enabled)
#define setButtonTitle(v, title, state)\
 msg2(void,CFStringRef,unsigned long,v,sel_getUid("setTitle:forState:"),title,state)
#define setButtonColor(v, color, state)\
 msg2(void,id,unsigned long,v,sel_getUid("setTitleColor:forState:"),color, state)

extern size_t ViewSize;
extern Class ViewClass;
extern Class FontClass;
extern Class ConstraintCls;

extern id NSForegroundColorAttributeName;
extern id NSFontAttributeName;

extern id UIFontTextStyleTitle3;
extern id UIFontTextStyleSubheadline;
extern id UIFontTextStyleBody;
extern id UIFontTextStyleFootnote;

extern CGFloat UIFontWeightRegular;

typedef struct {
    CGFloat top, left, bottom, right;
} HAInsets;

typedef struct {
    short top, left, bottom, right;
} Padding;

void setHeight(id v, int height, bool optional);
void pin(id v, id container);

id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text CF_CONSUMED, id style, bool accessible);
id createTextfield(id delegate, CFStringRef text,
                   CFStringRef hint CF_CONSUMED, int alignment, int keyboard, int tag);
id createButton(CFStringRef title CF_CONSUMED, int color, int tag, id target, SEL action);
id createSegmentedControl(CFBundleRef bundle, CFStringRef format, int startIndex);
void addVStackToScrollView(id view, id vStack, id scrollView);

void updateSegmentedControl(id view, id foreground, unsigned char darkMode);

#endif /* Views_h */
