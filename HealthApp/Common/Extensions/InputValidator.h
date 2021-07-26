//
//  InputValidator.h
//  HealthApp
//
//  Created by Christopher Ray on 7/25/21.
//

#ifndef InputValidator_h
#define InputValidator_h

#include "unordered_set.h"
#include <CoreFoundation/CoreFoundation.h>

gen_uset_headers(char, unsigned short)

USet_char *inputValidator_createNumberCharacterSet(void);
bool inputValidator_validateNumericInput(USet_char *set, CFStringRef str);

#endif /* InputValidator_h */
