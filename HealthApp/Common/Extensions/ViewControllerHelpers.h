//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaHelpers.h"
#include "unordered_set.h"

gen_uset_headers(char, unsigned short)

#define _cfstr(x) ((__bridge CFStringRef) x)
#define _nsstr(x) ((__bridge NSString*) x)
#define _nsarr(x) ((__bridge NSArray*) x)
#define _cfarr(x) ((__bridge CFArrayRef) x)

typedef struct {
    USet_char *set;
    id button;
    int count;
    struct InputView {
        id view;
        id hintLabel;
        id field;
        id errorLabel;
        short minVal;
        short maxVal;
        short result;
        bool valid;
    } children[4];
} Validator;

struct AnchorNames {
    const char *top;
    const char *bottom;
    const char *left;
    const char *right;
    const char *width;
    const char *height;
};

typedef struct {
    CGFloat top, left, bottom, right;
} Padding;

typedef enum {
    BtnLargeFont = 0x1,
    BtnBackground = 0x2,
    BtnRounded = 0x4
} ButtonParams;

typedef enum {
    TextFootnote = 1,
    TextSubhead,
    TextBody,
    TextHead,
    TextTitle1,
    TextTitle2,
    TextTitle3
} TextStyle;

extern struct AnchorNames anchors;

id createToolbar(id target, SEL doneSelector);
void setNavButton(id navItem, bool left, id button, CGFloat totalWidth);
id createDivider(void);
void textValidator_setup(Validator *this);
void textValidator_free(Validator *this);
id validator_addChild(Validator *this, id delegate, CFStringRef hint, int min, int max, id toolbar);
void inputView_toggleError(struct InputView *this, bool show);
void inputView_reset(struct InputView *this, short value);
bool checkInput(Validator *this, id field, CFRange range, CFStringRef replacement);

id getFirstVC(id navVC);
void setupNavVC(id navVC, id firstVC);
id allocNavVC(void);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter);

id getRootView(id vc);
void addSubview(id view, id subview);
id createAlertController(CFStringRef title, CFStringRef message);
id createAlertAction(CFStringRef title, int style, CallbackBlock handler);
void addAlertAction(id ctrl, id action);

id getAnchor(id view, const char *name);
id createConstraint(id a1, id a2, CGFloat constant);
id createObjectWithFrame(const char *name, CGRect rect);
id createView(id color, bool rounded);
id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text, int style, int alignment);
id createTextfield(id delegate, CFStringRef text, int alignment, int keyboard, int tag);
id createButton(CFStringRef title, id color, int params, int tag, id target, SEL action);
id createSegmentedControl(CFStringRef *items, int count, int startIndex, id target, SEL action);

void enableButton(id view, bool enabled);
void activateConstraints(id *constraints, int count);
void setTextColor(id view, id color);
void setTag(id view, int tag);
void setBackground(id view, id color);
void setTintColor(id view, id color);
void setLabelText(id view, CFStringRef text);
void setLabelFontWithStyle(id view, int style);
void setLabelFontWithSize(id view, CGFloat size);
void setButtonTitle(id view, CFStringRef title, int state);
void setButtonColor(id view, id color, int state);

#endif /* ViewControllerHelpers_h */
