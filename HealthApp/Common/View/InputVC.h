#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CFBundle.h>
#include <objc/runtime.h>

extern Class InputVCClass;
extern Class InputViewClass;
extern size_t InputVCSize;

typedef struct {
    id hintLabel;
    id field;
    id errorLabel;
    int minVal;
    int maxVal;
    short result;
    bool valid;
} InputView;

typedef struct {
    InputView *children[4];
    id button;
    id activeField;
    id scrollView;
    id vStack;
    id toolbar;
    int count;
    int scrollHeight;
    short topOffset;
    short bottomOffset;
    bool setKB;
} InputVC;

void inputView_deinit(id self, SEL _cmd);

void initValidatorStrings(CFBundleRef bundle);
id inputVC_init(id self, SEL _cmd);
void inputVC_deinit(id self, SEL _cmd);
void inputVC_viewDidLoad(id self, SEL _cmd);
void inputVC_viewDidAppear(id self, SEL _cmd, bool animated);
void inputVC_dismissKeyboard(id self, SEL _cmd);
void inputVC_fieldBeganEditing(id self, SEL _cmd, id field);
void inputVC_fieldStoppedEditing(id self, SEL _cmd, id field);
bool inputVC_fieldChanged(id self, SEL _cmd, id field, CFRange range, CFStringRef replacement);

void inputVC_addChild(id self, CFStringRef hint CF_CONSUMED, short min, short max);
void inputVC_updateFields(InputVC *self, short *vals);

#endif /* InputVC_h */
