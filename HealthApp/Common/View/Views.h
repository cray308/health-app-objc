#ifndef Views_h
#define Views_h

#include "Constraints.h"

#define disableAutoresizing(_v) \
setBool(_v, sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), false)

#define addSubview(_v, _sv) setObject(_v, sel_getUid("addSubview:"), _sv)

#define addArrangedSubview(_v, _sv) setObject(_v, sel_getUid("addArrangedSubview:"), _sv)

#define setStackSpacing(_sv, _val) setCGFloat(_sv, sel_getUid("setSpacing:"), _val)

#define setSpacingAfter(_sv, _v, _val) (((void(*)(id,SEL,CGFloat,id))objc_msgSend)\
((_sv), sel_getUid("setCustomSpacing:afterView:"), (_val), (_v)))

#define setMargins(_sv, _val) (((void(*)(id,SEL,HAInsets))objc_msgSend)\
((_sv), sel_getUid("setLayoutMargins:"), (_val)))

#define getSubviews(_v) getArray(_v, sel_getUid("subviews"))

#define getArrangedSubviews(_sv) getArray(_sv, sel_getUid("arrangedSubviews"))

#define removeView(_v) voidFunc(_v, sel_getUid("removeFromSuperview"))

#define getLayoutGuide(_v) getObject(_v, sel_getUid("safeAreaLayoutGuide"))

#define getLayer(_v) getObject(_v, sel_getUid("layer"))

#define setTag(_v, _tag) setInt(_v, sel_getUid("setTag:"), _tag)

#define getTag(_v) getInt(_v, sel_getUid("tag"))

#define getSelectedSegment(_v) getInt(_v, sel_getUid("selectedSegmentIndex"))

#define setSelectedSegment(_v, _i) setInt(_v, sel_getUid("setSelectedSegmentIndex:"), _i)

#define centerHStack(_v) setInt(_v, sel_getUid("setAlignment:"), 3)

#define setTextAlignment(_v, _a) setInt(_v, sel_getUid("setTextAlignment:"), _a)

#define hideView(_v, _hide) setBool(_v, sel_getUid("setHidden:"), _hide)

#define setBackground(_v, _color) setObject(_v, sel_getUid("setBackgroundColor:"), _color)

#define setTintColor(_v, _color) setObject(_v, sel_getUid("setTintColor:"), _color)

#define setAccessibilityLabel(_v, _text) setString(_v, sel_getUid("setAccessibilityLabel:"), _text)

#define setLabelText(_v, _text) setString(_v, sel_getUid("setText:"), _text)

#define getTitleLabel(_v) getObject(_v, sel_getUid("titleLabel"))

#define getText(_v) (((CFStringRef(*)(id,SEL))objc_msgSend)((_v), sel_getUid("text")))

#define setTextColor(_v, _color) setObject(_v, sel_getUid("setTextColor:"), _color)

#define setKBColor(_v, _a) setInt(_v, sel_getUid("setKeyboardAppearance:"), _a)

#define addTarget(_v, _target, _action, _event) (((void(*)(id,SEL,id,SEL,unsigned long))objc_msgSend)\
((_v), sel_getUid("addTarget:action:forControlEvents:"), (_target), (_action), (_event)))

#define enableButton(_v, _enabled) setBool(_v, sel_getUid("setEnabled:"), _enabled)

#define toggleInteraction(_v, _enable) setBool(_v, sel_getUid("setUserInteractionEnabled:"), _enable)

#define setDelegate(_v, _d) setObject(_v, sel_getUid("setDelegate:"), _d)

#define setInputAccessory(_v, _a) setObject(_v, sel_getUid("setInputAccessoryView:"), _a)

#define enableInteraction(_v, _enabled) setBool(_v, sel_getUid("setUserInteractionEnabled:"), _enabled)

#define setButtonTitle(_v, _title, _state) (((void(*)(id,SEL,CFStringRef,unsigned long))objc_msgSend)\
((_v), sel_getUid("setTitle:forState:"), (_title), (_state)))

#define setButtonColor(_v, _color, _state) (((void(*)(id,SEL,id,unsigned long))objc_msgSend)\
((_v), sel_getUid("setTitleColor:forState:"), (_color), (_state)))

extern size_t ViewSize;
extern Class ViewClass;

enum {
    TextFootnote = 1,
    TextSubhead,
    TextBody,
    TextHead,
    TextTitle1,
    TextTitle3
};

enum {
    WeightReg,
    WeightMed,
    WeightSemiBold
};

enum {
    BtnLargeFont = 0x1,
    BtnBackground = 0x2,
    BtnRounded = 0x4
};

id createFont(int style);
id createCustomFont(int style, int size);
CFDictionaryRef createTitleTextDict(id color, id font);

id createObjectWithFrame(Class cls, CGRect frame);
id createView(void);
id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text, int style, bool accessible);
id createTextfield(id delegate, CFStringRef text, CFStringRef hint,
                   int alignment, int keyboard, int tag);
id createButton(CFStringRef title, int color, int params, int tag, id target, SEL action);
id createSegmentedControl(CFStringRef format, int startIndex);
void addVStackToScrollView(id vStack, id scrollView);

void updateSegmentedControl(id view, id foreground, unsigned char darkMode);

#endif /* Views_h */
