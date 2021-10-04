//
//  Views.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef Views_h
#define Views_h

#include "Constraints.h"

enum {
    BtnLargeFont = 0x1,
    BtnBackground = 0x2,
    BtnRounded = 0x4
};

void addSubview(id view, id subview);
void removeView(id v);

id getLayer(id view);
void setTag(id view, int tag);
void hideView(id view, bool hide);
void setBackground(id view, id color);
void setTintColor(id view, id color);

void setLabelText(id view, CFStringRef text);
void setTextColor(id view, id color);

void enableButton(id view, bool enabled);
void enableInteraction(id view, bool enabled);
void setButtonTitle(id view, CFStringRef title, int state);
void setButtonColor(id view, id color, int state);

id createObjectWithFrame(const char *name, CGRect rect);
id createView(id color, bool rounded, int width, int height);
id createStackView(id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(CFStringRef text, int style, int alignment, int height);
id createTextfield(id delegate, CFStringRef text, int alignment, int keyboard, int tag, int height);
id createButton(CFStringRef title, id color, int params,
                int tag, id target, SEL action, int height);
id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height);

#endif /* Views_h */
