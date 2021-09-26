//
//  InputView.m
//  HealthApp
//
//  Created by Christopher Ray on 9/24/21.
//

#import "InputView.h"
#include "ViewControllerHelpers.h"

static CFStringRef inputFieldError;

@implementation InputView
- (id) initWithDelegate: (id)delegate
              fieldHint: (CFStringRef)fieldHint tag: (int)tag min: (int)min max: (int)max {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    if (!inputFieldError)
        inputFieldError = localize(CFSTR("inputFieldError"));
    CFStringRef errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    hintLabel = createLabel(fieldHint, UIFontTextStyleFootnote, 4);
    field = createTextfield(delegate, NULL, 4, 4);
    field.tag = tag;
    errorLabel = createLabel(errorText, UIFontTextStyleFootnote, 4);
    errorLabel.textColor = UIColor.systemRedColor;
    UIStackView *vStack = createStackView((id []){hintLabel, field, errorLabel}, 3, 1, 4, 0,
                                          (HAEdgeInsets){4, 8, 4, 8});
    self.translatesAutoresizingMaskIntoConstraints = false;
    [self addSubview:vStack];
    activateConstraints((id []){
        [vStack.topAnchor constraintEqualToAnchor:self.topAnchor],
        [vStack.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [hintLabel.heightAnchor constraintEqualToConstant:20],
        [field.heightAnchor constraintEqualToConstant:40],
        [errorLabel.heightAnchor constraintEqualToConstant:20]
    }, 7);
    [vStack release];
    CFRelease(errorText);
    [self toggleError:false];
    return self;
}

- (void) dealloc {
    [hintLabel release];
    [field release];
    [errorLabel release];
    [super dealloc];
}

- (void) toggleError: (bool)show { [errorLabel setHidden:!show]; }
@end
