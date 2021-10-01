//
//  StatusButton.m
//  HealthApp
//
//  Created by Christopher Ray on 9/24/21.
//

#import "StatusButton.h"
#include "ViewControllerHelpers.h"

id statusButton_init(CFStringRef text, bool hideViews, int tag, id target, SEL action) {
    StatusButton *this = [[StatusButton alloc] initWithFrame:CGRectZero];
    setTag(this, tag);
    int params = BtnLargeFont | BtnBackground | BtnRounded;
    this->button = createButton(text, UIColor.labelColor, params, tag, target, action, 50);
    this->headerLabel = createLabel(NULL, TextSubhead, 4, 20);
    this->box = createView(UIColor.systemGrayColor, true, 20, 20);
    UIStackView *hStack = createStackView((id []){this->button, this->box}, 2, 0, 5, (Padding){0});
    hStack.alignment = UIStackViewAlignmentCenter;
    id vStack = createStackView((id []){this->headerLabel, hStack}, 2, 1, 4, (Padding){4, 0, 4, 0});
    addSubview(this, vStack);
    pin(vStack, this, (Padding){0}, 0);
    if (hideViews) {
        [this->headerLabel setHidden:true];
        [this->box setHidden:true];
    }
    [hStack release];
    releaseObj(vStack);
    return this;
}

@implementation StatusButton
- (void) dealloc {
    [headerLabel release];
    [box release];
    [super dealloc];
}
@end
