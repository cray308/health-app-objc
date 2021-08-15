//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#include "CocoaBridging.h"
#include "unordered_set.h"

gen_uset_headers(char, unsigned short)

void createToolbar(id target, SEL doneSelector, id *fields);
id createDivider(void);
USet_char *createNumberCharacterSet(void);
bool validateNumericInput(USet_char *set, CFStringRef str, CFStringInlineBuffer *buf);

#endif /* ViewControllerHelpers_h */
