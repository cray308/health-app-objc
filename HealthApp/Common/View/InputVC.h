#ifndef InputVC_h
#define InputVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

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
    CFCharacterSetRef chars;
    int min;
    int max;
    float result;
    uint8_t state;
} InputView;

typedef struct {
    id view;
    InputView *data;
} IVPair;

typedef struct {
    IVPair children[4];
    id button;
    id scrollView;
    id vStack;
    id toolbar;
    int count;
    int activeTag;
    bool keyboardAppearance;
} InputVC;

#define getKeyboardForLocale(l) (isMetric(l) ? KeyboardTypeDecimalPad : KeyboardTypeNumberPad)

#define getJumpToPrevSel() sel_getUid("jumpToPrev")
#define getJumpToNextSel() sel_getUid("jumpToNext")
#define getDismissKeyboardSel() sel_getUid("dismissKeyboard")

void initValidatorData(void);

void inputView_deinit(id self, SEL _cmd);

void inputVC_addField(id self, CFStringRef hint CF_CONSUMED, int keyboardType, int min, int max);
void inputVC_updateFields(InputVC *d, int const *values);

void inputVC_deinit(id self, SEL _cmd);
void inputVC_viewDidLoad(id self, SEL _cmd);
void inputVC_dismissKeyboard(id self, SEL _cmd);
void inputVC_jumpToPrev(id self, SEL _cmd);
void inputVC_jumpToNext(id self, SEL _cmd);
void inputVC_textFieldDidBeginEditing(id self, SEL _cmd, id field);
void inputVC_textFieldDidEndEditing(id self, SEL _cmd, id field);
bool inputVC_textFieldShouldReturn(id self, SEL _cmd, id field);
bool inputVC_shouldChange(id self, SEL _cmd, id field, CFRange range, CFStringRef replacement);

#endif /* InputVC_h */
