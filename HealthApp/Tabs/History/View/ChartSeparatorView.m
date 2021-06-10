//
//  ChartSeparatorView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "ChartSeparatorView.h"
#import "Divider.h"

#define TitleHeight 40

@interface ChartSeparatorView()
@end

@implementation ChartSeparatorView

- (id) initWithTitle: (NSString *)title {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    [self setupSubviews:title];
    return self;
}

- (void) setupSubviews: (NSString *)title {
    Divider *topDivider = [[Divider alloc] init];
    [self addSubview:topDivider];

    UILabel *chartTitleLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    chartTitleLabel.translatesAutoresizingMaskIntoConstraints = false;
    chartTitleLabel.text = title;
    chartTitleLabel.font = [UIFont systemFontOfSize:20];
    chartTitleLabel.adjustsFontSizeToFitWidth = true;
    chartTitleLabel.textColor = UIColor.labelColor;
    chartTitleLabel.textAlignment = NSTextAlignmentCenter;
    [self addSubview:chartTitleLabel];

    [NSLayoutConstraint activateConstraints:@[
        [topDivider.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [topDivider.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [topDivider.topAnchor constraintEqualToAnchor:self.topAnchor],

        [chartTitleLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [chartTitleLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [chartTitleLabel.topAnchor constraintEqualToAnchor:topDivider.bottomAnchor constant:5],
        [chartTitleLabel.heightAnchor constraintEqualToConstant:TitleHeight],
        [chartTitleLabel.bottomAnchor constraintEqualToAnchor:self.bottomAnchor]
    ]];

    [topDivider release];
    [chartTitleLabel release];
}

@end
