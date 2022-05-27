#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

#define getIVIVCS(s) ((char *)(s) + sizeof(InputVC))
#define getIVIVC(x) ((char *)(x) + VCSize + sizeof(InputVC))

#define InputViewEncoding "{?=@@@@iifB}"
#define InputVCEncoding "{?=[4{?=@@}]@@@@@iB}"

#define FieldMaxDefault 999
#define getKBForLocale(l) (CFBooleanGetValue(CFLocaleGetValue(l, kCFLocaleUsesMetricSystem)) \
                           ? UIKeyboardTypeDecimalPad : UIKeyboardTypeNumberPad)

enum {
    UIKeyboardTypeNumberPad = 4,
    UIKeyboardTypeDecimalPad = 8
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
    IVPair children[4];
    id button;
    id activeField;
    id scrollView;
    id vStack;
    id toolbar;
    int count;
    bool setKB;
} InputVC;

void initValidatorData(void);

void inputView_deinit(id self, SEL _cmd);

void inputVC_addChild(id self, CFStringRef hint CF_CONSUMED, int kb, short min, short max);
void inputVC_updateFields(InputVC *d, const short *vals);

void inputVC_deinit(id self, SEL _cmd);
void inputVC_viewDidLoad(id self, SEL _cmd);
void inputVC_dismissKeyboard(id self, SEL _cmd);
void inputVC_jumpToPrev(id self, SEL _cmd);
void inputVC_jumpToNext(id self, SEL _cmd);
void inputVC_fieldBeganEditing(id self, SEL _cmd, id field);
void inputVC_fieldStoppedEditing(id self, SEL _cmd, id field);
bool inputVC_fieldShouldReturn(id self, SEL _cmd, id field);
bool inputVC_fieldChanged(id self, SEL _cmd, id field, CFRange range, CFStringRef replacement);

#endif /* InputVC_h */
