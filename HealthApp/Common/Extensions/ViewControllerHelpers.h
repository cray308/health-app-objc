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
void setNavButton(id navItem, bool left, id button, CGFloat totalWidth);
id createDivider(void);
USet_char *createNumberCharacterSet(void);
bool validateNumericInput(USet_char *set, CFStringRef str, CFStringInlineBuffer *buf);
bool checkTextfield(id field, CFRange range, CFStringRef replacement, USet_char *set, id button,
                    id *fields, int count, short *maxes, short *results, bool *valid);

#endif /* ViewControllerHelpers_h */
