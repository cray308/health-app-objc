//
//  TextValidator.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef TextValidator_h
#define TextValidator_h

#include <CoreGraphics/CGGeometry.h>
#include "Constraints.h"
#include "unordered_set.h"

gen_uset_headers(char, unsigned short)

typedef struct {
    USet_char *set;
    int scrollHeight;
    id button;
    id scrollView;
    id vStack;
    id activeField;
    id toolbar;
    int count;
    Padding padding;
    struct InputView {
        id view;
        id hintLabel;
        id field;
        id errorLabel;
        CFStringRef hintText;
        CFStringRef errorText;
        short minVal;
        short maxVal;
        short result;
        bool valid;
    } children[4];
} Validator;

void validator_setup(Validator *this, short margins, bool createSet, id target);
void validator_free(Validator *this, id target);
id validator_add(Validator *this, id delegate, CFStringRef hint, short min, short max);
void validator_getScrollHeight(Validator *this);
void validator_handleKeyboardShow(Validator *this, id view, id notif);
void validator_handleKeyboardHide(Validator *this);
void inputView_reset(struct InputView *this, short value);
bool checkInput(Validator *this, id field, CFRange range, CFStringRef replacement);

#endif /* TextValidator_h */
