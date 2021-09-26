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

typedef struct {
    USet_char *set;
    id button;
    int count;
    struct ChildValidator {
        id inputView;
        short minVal;
        short maxVal;
        short result;
        bool valid;
    } children[4];
} TextValidator;

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
} HAEdgeInsets;

typedef enum {
    StatusViewStateDisabled = 0,
    StatusViewStateActive = 1,
    StatusViewStateFinished = 3
} StatusViewState;

extern struct AnchorNames anchors;

id createToolbar(id target, SEL doneSelector);
void setNavButton(id navItem, bool left, id button, CGFloat totalWidth);
id createDivider(void);
USet_char *createNumberCharacterSet(void);
void resetInputChild(struct ChildValidator *child, short value);
bool checkInput(id field, CFRange range, CFStringRef replacement, TextValidator *validator);

id createVCWithDelegate(const char *name, void *delegate);
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
id createStackView(id *subviews, int count, int axis, CGFloat spacing,
                   int distribution, HAEdgeInsets margins);
id createScrollView(void);
id createLabel(CFStringRef text, id style, int alignment);
id createTextfield(id delegate, CFStringRef text, int alignment, int keyboard);
id createButton(CFStringRef title, id color, id style, id background,
                bool rounded, bool enabled, int tag, id target, SEL action);
id createSegmentedControl(CFStringRef *items, int count, int startIndex, id target, SEL action);

void enableButton(id view, bool enabled);
void activateConstraints(id *constraints, int count);
void setTag(id view, int tag);
void setBackground(id view, id color);
void setTintColor(id view, id color);
void setLabelText(id view, CFStringRef text);
void setLabelFontWithStyle(id view, id style);
void setLabelFontWithSize(id view, CGFloat size);
void setButtonTitle(id view, CFStringRef title, int state);
void setButtonColor(id view, id color, int state);

#endif /* ViewControllerHelpers_h */
