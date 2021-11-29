#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>
#include "unordered_set.h"

gen_uset_headers(char, unsigned short)

extern Class InputVCClass;
extern Ivar InputVCDataRef;
extern Class InputViewClass;
extern Ivar InputViewDataRef;

typedef struct __inputVData {
    short minVal;
    short maxVal;
    short result;
    bool valid;
    CFStringRef hintText;
    CFStringRef errorText;
    id hintLabel;
    id field;
    id errorLabel;
} InputViewData;

typedef struct __inputVCData {
    int count;
    int scrollHeight;
    short topOffset;
    short bottomOffset;
    id children[4];
    USet_char *set;
    id button;
    id activeField;
    id scrollView;
    id vStack;
    id toolbar;
} InputVCData;

void inputView_reset(InputViewData *data, short value);
void inputView_deinit(id self, SEL _cmd);

id inputVC_init(id self, SEL _cmd, id nibName, id bundle);
void inputVC_deinit(id self, SEL _cmd);
void inputVC_viewDidLoad(id self, SEL _cmd);
void inputVC_viewDidAppear(id self, SEL _cmd, bool animated);
void inputVC_dismissKeyboard(id self, SEL _cmd);
bool inputVC_fieldShouldReturn(id self, SEL _cmd, id field);
void inputVC_keyboardShown(id self, SEL _cmd, id notif);
void inputVC_keyboardWillHide(id self, SEL _cmd, id notif);
void inputVC_fieldBeganEditing(id self, SEL _cmd, id field);
void inputVC_fieldStoppedEditing(id self, SEL _cmd, id field);
bool inputVC_fieldChanged(id self, SEL _cmd, id field, CFRange range, CFStringRef replacement);

void inputVC_addChild(id self, CFStringRef hint, short min, short max);

#endif /* InputVC_h */
