//
//  Views.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef Views_h
#define Views_h

#include "Constraints.h"
#include <CoreFoundation/CFArray.h>
#include <CoreGraphics/CGGeometry.h>

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
    id view;
    id label;
    id switchView;
} SwitchContainer;

id createCustomFont(int style, int size);
void addSubview(id view, id subview);
void removeView(id v);

id getLayer(id view);
void setTag(id view, int tag);
void hideView(id view, bool hide);
void setBackground(id view, id color);
void setTintColor(id view, id color);

void setAccessibilityLabel(id view, CFStringRef text);
void setLabelText(id view, CFStringRef text);
void setTextColor(id view, id color);

void enableButton(id view, bool enabled);
void enableInteraction(id view, bool enabled);
void setButtonTitle(id view, CFStringRef title, int state);
void setButtonColor(id view, id color, int state);
CFDictionaryRef createTitleTextDict(id color, id font);

id createObjectWithFrame(const char *name, CGRect rect);
id createView(id color, bool rounded, int width, int height);
id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text, int style, int alignment, bool accessible);
id createTextfield(id delegate, CFStringRef text, CFStringRef hint,
                   int alignment, int keyboard, int tag);
id createButton(CFStringRef title, id color, int params,
                int tag, id target, SEL action, int height);
id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height);
SwitchContainer *createSwitch(CFStringRef label, bool enabled);
void addVStackToScrollView(id vStack, id scrollView);

void updateButton(id view, id color);
void updateSegmentedControl(id view);

#endif /* Views_h */
