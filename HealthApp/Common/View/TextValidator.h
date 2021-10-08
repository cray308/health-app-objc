//
//  TextValidator.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef TextValidator_h
#define TextValidator_h

#include <CoreFoundation/CFBase.h>
#include "Constraints.h"
#include "unordered_set.h"

gen_uset_headers(char, unsigned short)

typedef struct {
    USet_char *set;
    id button;
    id toolbar;
    int count;
    Padding padding;
    struct InputView {
        id view;
        id hintLabel;
        id field;
        id errorLabel;
        short minVal;
        short maxVal;
        short result;
        bool valid;
    } children[4];
} Validator;

void validator_setup(Validator *this, short margins, bool createSet, id target, SEL doneSelector);
void validator_free(Validator *this);
id validator_add(Validator *this, id delegate, CFStringRef hint, short min, short max);
void inputView_reset(struct InputView *this, short value);
bool checkInput(Validator *this, id field, CFRange range, CFStringRef replacement);

#endif /* TextValidator_h */
