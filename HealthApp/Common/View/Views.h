#ifndef Views_h
#define Views_h

#include "CocoaHelpers.h"
#include "ViewCache.h"

extern Class VC;
extern Class View;
extern size_t VCSize;
extern size_t ViewSize;

extern CFStringRef NSForegroundColorAttributeName;
extern CFStringRef NSFontAttributeName;

extern CFStringRef UIFontTextStyleTitle3;
extern CFStringRef UIFontTextStyleSubheadline;
extern CFStringRef UIFontTextStyleBody;
extern CFStringRef UIFontTextStyleFootnote;

extern CGFloat UIFontWeightRegular;

extern uint32_t UIAccessibilityAnnouncementNotification;
extern void UIAccessibilityPostNotification(uint32_t, id);
extern BOOL UIAccessibilityIsVoiceOverRunning(void);

typedef struct {
    short top, left, bottom, right;
} Padding;

void pin(ConstraintCache const *tbl, id v, id container);

id createStackView(VCacheRef tbl, id *subviews, int count, int axis, int spacing, Padding margins);
id createScrollView(void);
id createLabel(VCacheRef tbl, CCacheRef clr, CFStringRef text CF_CONSUMED, CFStringRef style, int lines);
id createTextfield(VCacheRef tbl, CCacheRef clr, id delegate, CFStringRef text,
                   CFStringRef hint CF_CONSUMED, int alignment, int keyboard, int tag);
id createButton(VCacheRef tbl, CCacheRef clr, CFStringRef title CF_CONSUMED,
                int color, CFStringRef style, int tag, id target, SEL action);
id createSegmentedControl(CFBundleRef bundle, CFStringRef format, int startIndex);
void addVStackToScrollView(VCacheRef tbl, id view, id vStack, id scrollView);
void setNavButtons(id vc, id *buttons);
void setVCTitle(id vc, CFStringRef title CF_CONSUMED);

void updateSegmentedControl(CCacheRef clr, id view, unsigned char darkMode);

#endif /* Views_h */
