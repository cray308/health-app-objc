#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include "ColorCache.h"
#include "ViewCache.h"

#define FieldMaxDefault 999

enum {
    KeyboardTypeNumberPad = 4,
    KeyboardTypeDecimalPad = 8
};

extern Class InputViewClass;
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

#define getKeyboardForLocale(l) (isMetric(l) ? KeyboardTypeDecimalPad : KeyboardTypeNumberPad)

void initValidatorStrings(Class Field);

void inputView_deinit(id self, SEL _cmd);

void inputVC_addChild(id self, CFStringRef hint CF_CONSUMED, int kb, short min, short max);
void inputVC_updateFields(InputVC *self, const short *vals);

id inputVC_init(id self, SEL _cmd, VCacheRef tbl, CCacheRef clr);
void inputVC_deinit(id self, SEL _cmd);
void inputVC_viewDidLoad(id self, SEL _cmd);
void inputVC_viewDidAppear(id self, SEL _cmd, bool animated);
void inputVC_dismissKeyboard(id self, SEL _cmd);
void inputVC_jumpToPrev(id self, SEL _cmd);
void inputVC_jumpToNext(id self, SEL _cmd);
void inputVC_fieldBeganEditing(id self, SEL _cmd, id field);
void inputVC_fieldStoppedEditing(id self, SEL _cmd, id field);
bool inputVC_fieldChanged(id self, SEL _cmd, id field, CFRange range, CFStringRef replacement);

#endif /* InputVC_h */
