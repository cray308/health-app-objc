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
    short top, left, bottom, right;
} Padding;

typedef struct {
    USet_char *set;
    id button;
    int count;
    Padding padding;
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

typedef struct {
    id view;
    id divider;
    id headerLabel;
    id stack;
    Array_object *views;
} SectionContainer;

typedef enum {
    HideDivider = 0x1,
    HideLabel = 0x2
} HiddenView;

typedef enum {
    BtnLargeFont = 0x1,
    BtnBackground = 0x2,
    BtnRounded = 0x4
} ButtonParams;

typedef enum {
    EdgeTop = 0x1,
    EdgeLeft = 0x2,
    EdgeBottom = 0x4,
    EdgeRight = 0x8
} ExcludedEdge;

typedef enum {
    TextFootnote = 1,
    TextSubhead,
    TextBody,
    TextHead,
    TextTitle1,
    TextTitle3
} TextStyle;

id createToolbar(id target, SEL doneSelector);
void setNavButton(id navItem, bool left, id button, CGFloat totalWidth);
void textValidator_setup(Validator *this, short margins);
void textValidator_free(Validator *this);
id validator_add(Validator *this, id delegate, CFStringRef hint, int min, int max, id toolbar);
void inputView_reset(struct InputView *this, short value);
bool checkInput(Validator *this, id field, CFRange range, CFStringRef replacement);

id createContainer(SectionContainer *c, CFStringRef title, int hidden, int spacing, bool margins);
void containers_free(SectionContainer *c, int size);
void container_add(SectionContainer *c, id v);

id getFirstVC(id navVC);
void setupNavVC(id navVC, id firstVC);
id allocNavVC(void);
void presentVC(id presenter, id child);
void presentModalVC(id presenter, id modal);
void dismissPresentedVC(id presenter);

void addSubview(id view, id subview);
id createAlertController(CFStringRef title, CFStringRef message);
void addAlertAction(id ctrl, CFStringRef title, int style, CallbackBlock handler);

void setHeight(id v, int height);
void setEqualWidths(id v, id container);
void pin(id v, id container, Padding margins, uint excluded);
id createTabController(void);
id createObjectWithFrame(const char *name, CGRect rect);
id createView(id color, bool rounded, int width, int height);
id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text, int style, int alignment, int height);
id createTextfield(id delegate, CFStringRef text, int alignment, int keyboard, int tag, int height);
id createButton(CFStringRef title, id color, int params, int tag, id target, SEL action, int height);
id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height);

id getLayer(id view);
void hideView(id view, bool hide);
void enableInteraction(id view, bool enabled);
void removeView(id v);
void enableButton(id view, bool enabled);
void setTextColor(id view, id color);
void setTag(id view, int tag);
void setBackground(id view, id color);
void setTintColor(id view, id color);
void setLabelText(id view, CFStringRef text);
void setButtonTitle(id view, CFStringRef title, int state);
void setButtonColor(id view, id color, int state);

#endif /* ViewControllerHelpers_h */
