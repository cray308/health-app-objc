#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CFBase.h>
#include <objc/objc.h>

extern Class InputVCClass;
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

void inputVC_addChild(id self, CFStringRef hint CF_CONSUMED, short min, short max);
void inputVC_updateFields(InputVC *self, const short *vals);

#endif /* InputVC_h */
