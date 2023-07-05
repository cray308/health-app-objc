#include "CustomButton.h"
#include "Views.h"

extern uint64_t UIAccessibilityTraitNotEnabled;

Class CustomButton;

uint64_t customButton_getAccessibilityTraits(id self _U_, SEL _cmd _U_) {
    uint64_t mask = UIAccessibilityTraitButton;
    if (!isEnabled(self)) mask |= UIAccessibilityTraitNotEnabled;
    return mask;
}
