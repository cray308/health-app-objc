//
//  HomeWorkoutDayButton.m
//  HealthApp
//
//  Created by Christopher Ray on 6/8/21.
//

#import "HomeWorkoutDayButton.h"

@interface HomeWorkoutDayButton() {
    UIView *checkbox;
    UIButton *button;
}

@end

@implementation HomeWorkoutDayButton

- (id) initWithTitle: (NSString *)title day: (NSString *)day {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    [self setupSubviewsWithTitle:title day:day];
    return self;
}

- (void) dealloc {
    [checkbox release];
    [button release];
    [super dealloc];
}

- (void) setupSubviewsWithTitle: (NSString *)title day: (NSString *)day {
    button = [UIButton buttonWithType:UIButtonTypeSystem];
    button.translatesAutoresizingMaskIntoConstraints = false;
    [button setTitle:title forState:UIControlStateNormal];
    [button setTitleColor:UIColor.labelColor forState: UIControlStateNormal];
    [button setTitleColor:UIColor.secondaryLabelColor forState:UIControlStateDisabled];
    button.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    button.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    button.layer.cornerRadius = 5;

    UILabel *topLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    topLabel.translatesAutoresizingMaskIntoConstraints = false;
    topLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
    topLabel.adjustsFontSizeToFitWidth = true;
    topLabel.text = day;
    topLabel.textColor = UIColor.labelColor;

    checkbox = [[UIView alloc] initWithFrame:CGRectZero];
    checkbox.translatesAutoresizingMaskIntoConstraints = false;
    checkbox.backgroundColor = UIColor.systemGrayColor;
    checkbox.layer.cornerRadius = 5;

    [self addSubview:topLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    [NSLayoutConstraint activateConstraints:@[
        [topLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [topLabel.heightAnchor constraintEqualToConstant:20],
        [topLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [topLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],

        [button.topAnchor constraintEqualToAnchor:topLabel.bottomAnchor constant:5],
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor constant:-5],

        [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
        [checkbox.widthAnchor constraintEqualToConstant:20],
        [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
    ]];
    [topLabel release];
}

- (UIButton *) getButton {
    return button;
}

- (void) setEnabled: (bool)enabled {
    [button setEnabled:enabled];
    [button setUserInteractionEnabled:enabled];

    if (enabled) {
        checkbox.backgroundColor = UIColor.systemGrayColor;
    } else {
        checkbox.backgroundColor = UIColor.systemGreenColor;
    }
}

@end
