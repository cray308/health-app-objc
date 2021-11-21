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

#define removeView(_v) voidFunc(_v, sel_getUid("removeFromSuperview"))

#define getLayer(_v) getObject(_v, sel_getUid("layer"))

#define getLayoutGuide(_v) getObject(_v, sel_getUid("safeAreaLayoutGuide"))

#define setTag(_v, _tag) setInt(_v, sel_getUid("setTag:"), _tag)

#define getTag(_v) getInt(_v, sel_getUid("tag"))

#define getSelectedSegment(_v) getInt(_v, sel_getUid("selectedSegmentIndex"))

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

#define enableButton(_v, _enabled) setBool(_v, sel_getUid("setEnabled:"), _enabled)

#define toggleInteraction(_v, _enable) setBool(_v, sel_getUid("setUserInteractionEnabled:"), _enable)

#define setDelegate(_v, _d) setObject(_v, sel_getUid("setDelegate:"), _d)

#define setInputAccessory(_v, _a) setObject(_v, sel_getUid("setInputAccessoryView:"), _a)

#define enableInteraction(_v, _enabled) setBool(_v, sel_getUid("setUserInteractionEnabled:"), _enabled)

#define setButtonTitle(_v, _title, _state) (((void(*)(id,SEL,CFStringRef,int))objc_msgSend)\
((_v), sel_getUid("setTitle:forState:"), (_title), (_state)))

#define setButtonColor(_v, _color, _state) (((void(*)(id,SEL,id,int))objc_msgSend)\
((_v), sel_getUid("setTitleColor:forState:"), (_color), (_state)))

extern Class DMButtonClass;
extern Class DMLabelClass;
extern Class DMTextFieldClass;
extern Class DMBackgroundViewClass;

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

typedef struct {
    Class isa;
    int colorCode;
    bool background;
} DMButton;

typedef struct {
    Class isa;
    int colorCode;
} DMLabel;

typedef struct {
    Class isa;
    int colorCode;
} DMBackgroundView;

id createCustomFont(int style, int size);
CFDictionaryRef createTitleTextDict(id color, id font);

id createObjectWithFrame(Class cls, CGRect frame);
id createBackgroundView(int color, int height);
id createView(bool rounded, int size);
id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text, int style, int alignment, bool accessible);
id createTextfield(id delegate, CFStringRef text, CFStringRef hint,
                   int alignment, int keyboard, int tag);
id createButton(CFStringRef title, int color, int params,
                int tag, id target, SEL action, int height);
id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height);
void addVStackToScrollView(id vStack, id scrollView);

void updateSegmentedControl(id view);
void dmButton_updateColors(DMButton* self, SEL _cmd);
void dmLabel_updateColors(DMLabel *self, SEL _cmd);
void dmField_updateColors(id self, SEL _cmd);
void dmBackgroundView_updateColors(DMBackgroundView *self, SEL _cmd);

#endif /* Views_h */
