//
//  StatusButton.m
//  HealthApp
//
//  Created by Christopher Ray on 9/24/21.
//

#import "StatusButton.h"

@implementation StatusButton
- (id) initWithButtonText: (CFStringRef)buttonText hideHeader: (bool)hideHeader
                  hideBox: (bool)hideBox tag: (int)tag target: (id)target action: (SEL)action {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    setTag(self, tag);
    button = createButton(buttonText, UIColor.labelColor, UIFontTextStyleHeadline,
                          UIColor.secondarySystemGroupedBackgroundColor,
                          true, true, tag, target, action);
    headerLabel = createLabel(NULL, UIFontTextStyleSubheadline, NSTextAlignmentNatural);
    checkbox = createView(nil, true);
    [self addSubview:button];
    NSLayoutConstraint *buttonTop = [button.topAnchor constraintEqualToAnchor:self.topAnchor
                                                                     constant:4];
    NSLayoutConstraint *buttonRight = [button.trailingAnchor
                                       constraintEqualToAnchor:self.trailingAnchor];

    if (!hideHeader) {
        [self addSubview:headerLabel];
        buttonTop = [button.topAnchor constraintEqualToAnchor:headerLabel.bottomAnchor constant:4];
        activateConstraints((id []){
            [headerLabel.topAnchor constraintEqualToAnchor:self.topAnchor constant:2],
            [headerLabel.heightAnchor constraintEqualToConstant:20],
            [headerLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
            [headerLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor]
        }, 4);
    }
    if (!hideBox) {
        [self addSubview:checkbox];
        buttonRight = [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor
                                                            constant:-5];
        activateConstraints((id []){
            [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
            [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
            [checkbox.widthAnchor constraintEqualToConstant:20],
            [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
        }, 4);
    }

    activateConstraints((id []){
        buttonTop, buttonRight,
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor constant:-4],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
    }, 5);

    [self updateWithLabelText:NULL buttonText:buttonText state:0 enable:true];
    return self;
}

- (void) dealloc {
    [headerLabel release];
    [checkbox release];
    [super dealloc];
}

- (void) updateWithLabelText: (CFStringRef)labelText buttonText: (CFStringRef)buttonText
                       state: (StatusViewState)state enable:(bool)enable {
    setLabelText(headerLabel, labelText);
    setButtonTitle(button, buttonText, 0);
    [self updateState:state enable:enable];
}

- (void) updateState: (StatusViewState)state enable:(bool)enable {
    enableButton(button, enable);
    switch (state) {
        case StatusViewStateDisabled:
            setBackground(checkbox, UIColor.systemGrayColor);
            break;
        case StatusViewStateActive:
            setBackground(checkbox, UIColor.systemOrangeColor);
            break;
        default:
            setBackground(checkbox, UIColor.systemGreenColor);
    }
}
@end
