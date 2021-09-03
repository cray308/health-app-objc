//
//  ExerciseView.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "ExerciseView.h"
#include "ViewControllerHelpers.h"

@implementation ExerciseView
- (id) initWithEntry: (ExerciseEntry *)e tag: (uint)tag target: (id)target action: (SEL)action {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;

    button = createButton(CFSTR(""), UIColor.labelColor, UIColor.secondaryLabelColor,
                          UIFontTextStyleHeadline, UIColor.secondarySystemGroupedBackgroundColor,
                          true, true, (int) tag, target, action);
    checkbox = createView(nil, true);
    setsLabel = createLabel(NULL, UIFontTextStyleSubheadline, NSTextAlignmentNatural);

    [self addSubview:setsLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    activateConstraints((id []){
        [setsLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [setsLabel.heightAnchor constraintEqualToConstant:20],
        [setsLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [setsLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],

        [button.topAnchor constraintEqualToAnchor:setsLabel.bottomAnchor constant:5],
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor constant:-5],

        [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
        [checkbox.widthAnchor constraintEqualToConstant:20],
        [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
    }, 13);
    [self configureWithEntry:e];
    return self;
}

- (void) dealloc {
    [setsLabel release];
    [checkbox release];
    [super dealloc];
}

- (void) configureWithEntry: (ExerciseEntry *)e {
    CFStringRef setsStr = exerciseEntry_createSetsTitle(e);
    CFStringRef title = exerciseEntry_createTitle(e);
    setButtonTitle(button, title, 0);
    setLabelText(setsLabel, setsStr);

    switch (e->state) {
        case ExerciseStateDisabled:
            setBackground(checkbox, UIColor.systemGrayColor);
            enableButton(button, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseTypeDuration)
                button.userInteractionEnabled = false;
        case ExerciseStateResting:
            enableButton(button, true);
            setBackground(checkbox, UIColor.systemOrangeColor);
            break;
        case ExerciseStateCompleted:
            enableButton(button, false);
            setBackground(checkbox, UIColor.systemGreenColor);
    }

    CFRelease(title);
    if (setsStr)
        CFRelease(setsStr);
}
@end
