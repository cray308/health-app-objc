#ifndef CustomButton_h
#define CustomButton_h

#include <objc/objc.h>

extern Class CustomButton;

uint64_t customButton_getAccessibilityTraits(id self, SEL _cmd);

extern id createCustomButton(Class ButtonClass);

#endif /* CustomButton_h */
