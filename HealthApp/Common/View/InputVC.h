#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>

extern Class InputVCClass;
extern Class InputViewClass;
extern size_t InputVCSize;

typedef struct {
    short minVal;
    short maxVal;
    short result;
    bool valid;
    id hintLabel;
    id field;
    id errorLabel;
} InputView;

typedef struct {
    int count;
    int scrollHeight;
    short topOffset;
    short bottomOffset;
    id children[4];
    id button;
    id activeField;
    id scrollView;
    id vStack;
    id toolbar;
} InputVC;

void inputView_reset(InputView *data, short value);
void inputView_deinit(id self, SEL _cmd);

void initValidatorStrings(void);
id inputVC_init(id self, SEL _cmd);
void inputVC_deinit(id self, SEL _cmd);
void inputVC_viewDidLoad(id self, SEL _cmd);
void inputVC_viewDidAppear(id self, SEL _cmd, bool animated);
void inputVC_dismissKeyboard(id self, SEL _cmd);
void inputVC_keyboardShown(id self, SEL _cmd, id notif);
void inputVC_keyboardWillHide(id self, SEL _cmd, id notif);
void inputVC_fieldBeganEditing(id self, SEL _cmd, id field);
void inputVC_fieldStoppedEditing(id self, SEL _cmd, id field);
bool inputVC_fieldChanged(id self, SEL _cmd, id field, CFRange range, CFStringRef replacement);

void inputVC_addChild(id self, CFStringRef hint, short min, short max);

#endif /* InputVC_h */
