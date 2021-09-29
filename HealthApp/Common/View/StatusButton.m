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
    id top = [this->button.topAnchor constraintEqualToAnchor:this.topAnchor constant:4];
    id right = [this->button.trailingAnchor constraintEqualToAnchor:this.trailingAnchor];

    if (!hideViews) {
        [this addSubview:this->headerLabel];
        [this addSubview:this->box];
        top = [this->button.topAnchor constraintEqualToAnchor:this->headerLabel.bottomAnchor
                                                     constant:4];
        right = [this->button.trailingAnchor constraintEqualToAnchor:this->box.leadingAnchor
                                                            constant:-5];
        activateConstraints((id []){
            [this->headerLabel.topAnchor constraintEqualToAnchor:this.topAnchor constant:2],
            [this->headerLabel.heightAnchor constraintEqualToConstant:20],
            [this->headerLabel.leadingAnchor constraintEqualToAnchor:this.leadingAnchor],
            [this->headerLabel.trailingAnchor constraintEqualToAnchor:this.trailingAnchor],
            [this->box.trailingAnchor constraintEqualToAnchor:this.trailingAnchor],
            [this->box.centerYAnchor constraintEqualToAnchor:this->button.centerYAnchor],
            [this->box.widthAnchor constraintEqualToConstant:20],
            [this->box.heightAnchor constraintEqualToAnchor:this->box.widthAnchor]
        }, 8);
    }
    activateConstraints((id []){
        top, right,
        [this->button.heightAnchor constraintEqualToConstant:50],
        [this->button.bottomAnchor constraintEqualToAnchor:this.bottomAnchor constant:-4],
        [this->button.leadingAnchor constraintEqualToAnchor:this.leadingAnchor],
    }, 5);
    return this;
}

@implementation StatusButton
- (void) dealloc {
    [headerLabel release];
    [box release];
    [super dealloc];
}
@end
