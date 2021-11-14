#import "StatusButton.h"
#include "ViewControllerHelpers.h"

id statusButton_init(CFStringRef text, bool hideViews, int tag, id target, SEL action) {
    StatusButton *this = [[StatusButton alloc] initWithFrame:CGRectZero];
    setTag(this, tag);
    int params = BtnLargeFont | BtnBackground | BtnRounded;
    this->button = createButton(text, ColorLabel, params, tag, target, action, 50);
    this->headerLabel = createLabel(NULL, TextSubhead, 4, false);
    this->box = createView(true, 20, 20);
    UIStackView *hStack = createStackView((id []){this->button, this->box}, 2, 0, 5, (Padding){0});
    hStack.alignment = UIStackViewAlignmentCenter;
    id vStack = createStackView((id []){this->headerLabel, hStack}, 2, 1, 4, (Padding){4, 0, 4, 0});
    addSubview(this, vStack);
    pin(vStack, this, (Padding){0}, 0);
    if (hideViews) {
        hideView(this->headerLabel, true);
        hideView(this->box, true);
    }
    [hStack release];
    releaseObj(vStack);
    return this;
}

void statusButton_updateAccessibility(StatusButton *b, CFStringRef stateText) {
    CFStringRef header = _cfstr(b->headerLabel.text);
    CFMutableStringRef label = CFStringCreateMutableCopy(NULL, 128, CFSTR(""));
    if (header) CFStringAppendFormat(label, NULL, CFSTR("%@. "), header);
    CFStringAppend(label, _cfstr(b->button.titleLabel.text));
    if (stateText) CFStringAppendFormat(label, NULL, CFSTR(". %@"), stateText);
    setAccessibilityLabel(b->button, label);
    CFRelease(label);
}

@implementation StatusButton
- (void) dealloc {
    [headerLabel release];
    [box release];
    [super dealloc];
}
@end
