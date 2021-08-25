//
//  DayWorkoutButton.m
//  HealthApp
//
//  Created by Christopher Ray on 8/23/21.
//

#import "DayWorkoutButton.h"
#include "ViewControllerHelpers.h"

@implementation DayWorkoutButton
- (id) initWithTitle: (CFStringRef)title day: (CFStringRef)day
                 tag: (int)tag target: (id)target action: (SEL)action {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    setTag(self, tag);
    button = createButton(title, UIColor.labelColor, UIColor.secondaryLabelColor,
                          UIFontTextStyleHeadline, UIColor.secondarySystemGroupedBackgroundColor,
                          true, true, tag, target, action);

    UILabel *topLabel = createLabel(day, UIFontTextStyleSubheadline, NSTextAlignmentNatural);
    checkbox = createView(UIColor.systemGrayColor, true);

    [self addSubview:topLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    activateConstraints((id []){
        [topLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [topLabel.heightAnchor constraintEqualToConstant:20],
        [topLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [topLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],

        [button.topAnchor constraintEqualToAnchor:topLabel.bottomAnchor constant:5],
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor constant:-5],

        [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
        [checkbox.widthAnchor constraintEqualToConstant:20],
        [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
    }, 13);
    [topLabel release];
    return self;
}

- (void) dealloc {
    [checkbox release];
    [super dealloc];
}
@end
