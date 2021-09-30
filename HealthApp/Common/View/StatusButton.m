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
    this->button = createButton(text, UIColor.labelColor, params, tag, target, action);
    this->headerLabel = createLabel(NULL, TextSubhead, 4);
    this->box = createView(UIColor.systemGrayColor, true);
    [this addSubview:this->button];

    if (!hideViews) {
        [this addSubview:this->headerLabel];
        [this addSubview:this->box];

        pin(this->headerLabel, this, (Padding){2, 0, 0, 0}, EdgeBottom);
        setHeight(this->headerLabel, 20);
        pinTopToBottom(this->button, this->headerLabel, 4);
        setWidth(this->box, 20);
        setHeight(this->box, 20);
        pinRightToRight(this->box, this, 0);
        setEqualCenterY(this->box, this->button);
        pinRightToLeft(this->button, this->box, -5);
    } else {
        pinTopToTop(this->button, this, 4);
        pinRightToRight(this->button, this, 0);
    }
    pin(this->button, this, (Padding){0, 0, 4, 0}, EdgeTop | EdgeRight);
    setHeight(this->button, 50);
    return this;
}

@implementation StatusButton
- (void) dealloc {
    [headerLabel release];
    [box release];
    [super dealloc];
}
@end
