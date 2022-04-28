#ifndef InputVC_h
#define InputVC_h

#include "ColorCache.h"
#include "ViewCache.h"
#include <CoreFoundation/CFCharacterSet.h>

extern Class InputVCClass;

typedef struct {
    id hintLabel;
    id field;
    id errorLabel;
    CFCharacterSetRef set;
    int minVal;
    int maxVal;
    float result;
    bool valid;
} InputView;

typedef struct {
    id view;
    InputView *data;
} IVPair;

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    IVPair children[4];
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

void inputVC_addChild(id self, CFStringRef hint CF_CONSUMED, int kb, short min, short max);
void inputVC_updateFields(InputVC *self, const short *vals);

#endif /* InputVC_h */
