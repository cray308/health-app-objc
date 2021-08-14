//
//  InputValidator.c
//  HealthApp
//
//  Created by Christopher Ray on 7/25/21.
//

#include "InputValidator.h"

gen_uset_source(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal,
                DSDefault_shallowCopy, DSDefault_shallowDelete)

USet_char *inputValidator_createNumberCharacterSet(void) {
    unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    return uset_new_fromArray(char, nums, 10);
}

bool inputValidator_validateNumericInput(USet_char *set, CFStringRef str) {
    int len = (int) CFStringGetLength(str);
    if (!len) return true;

    CFStringInlineBuffer buf;
    CFStringInitInlineBuffer(str, &buf, CFRangeMake(0, len));
    for (int i = 0; i < len; ++i) {
        if (!uset_contains(char, set, CFStringGetCharacterFromInlineBuffer(&buf, i))) return false;
    }
    return true;
}
