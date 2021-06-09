//
//  SingleActivityTableViewCell.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SingleActivityTableViewCell.h"
#import "ActivityType.h"

@interface SingleActivityTableViewCell() {
    UILabel *infoLabel;
}

@end

@implementation SingleActivityTableViewCell

- (id) initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    if (!(self = [super initWithStyle:style reuseIdentifier:reuseIdentifier])) return nil;
    self.selectionStyle = UITableViewCellSelectionStyleNone;
    self.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [infoLabel release];
    [super dealloc];
}

- (void) setupSubviews {
    infoLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    infoLabel.translatesAutoresizingMaskIntoConstraints = false;
    infoLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    infoLabel.textColor = UIColor.labelColor;
    infoLabel.adjustsFontSizeToFitWidth = true;
    [self.contentView addSubview:infoLabel];

    [NSLayoutConstraint activateConstraints:@[
        [infoLabel.leadingAnchor constraintEqualToAnchor:self.contentView.leadingAnchor constant:8],
        [infoLabel.topAnchor constraintEqualToAnchor:self.contentView.topAnchor constant:8],
        [infoLabel.heightAnchor constraintEqualToConstant:30]
    ]];
}

- (void) configureWithIntensity: (NSString *)intensity duration: (int)duration {
    infoLabel.text = [NSString stringWithFormat:@"Intensity: %@, Duration: %d minutes", intensity, duration];
}

@end
