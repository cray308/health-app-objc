//
//  NavBarCoinsView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "NavBarCoinsView.h"
#import "Constants.h"
#import "ImageHelpers.h"

#define AnimationPath @"position"
#define AnimationDuration 0.07
#define AnimationRepeatCount 8
#define AnimationDeadline 0.75

@interface NavBarCoinsView() {
    UIImageView *coinImageView;
    UILabel *tokenLabel;
}

@end

@implementation NavBarCoinsView

- (id) init {
    if (!(self = [super initWithFrame:CGRectMake(40, 0, 20, 20)])) return nil;
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [coinImageView release];
    [tokenLabel release];
    [super dealloc];
}

- (void) setupSubviews {
    coinImageView = [[UIImageView alloc] initWithFrame:CGRectMake(10, 0, 20, 20)];
    coinImageView.clipsToBounds = true;
    coinImageView.contentMode = UIViewContentModeScaleAspectFill;
    UIImage *tempImg = [UIImage imageNamed:TokenIconImageName];
    coinImageView.image = resizeImage(tempImg, 20);
    [self addSubview:coinImageView];

    tokenLabel = [[UILabel alloc] initWithFrame:CGRectMake(40, 0, 20, 20)];
    tokenLabel.font = [UIFont boldSystemFontOfSize:16];
    tokenLabel.adjustsFontSizeToFitWidth = true;
    tokenLabel.textColor = UIColor.labelColor;
    tokenLabel.textAlignment = NSTextAlignmentCenter;
    [self addSubview:tokenLabel];
}

- (void) runAnimation {
    CABasicAnimation *anim = [CABasicAnimation animationWithKeyPath:AnimationPath];
    anim.duration = AnimationDuration;
    anim.repeatCount = AnimationRepeatCount;
    anim.autoreverses = true;
    anim.fromValue = [NSValue valueWithCGPoint:CGPointMake(coinImageView.center.x - 5, coinImageView.center.y)];
    anim.toValue = [NSValue valueWithCGPoint:CGPointMake(coinImageView.center.x + 5, coinImageView.center.y)];
    [coinImageView.layer addAnimation:anim forKey:AnimationPath];

    dispatch_time_t endTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t) NSEC_PER_SEC * AnimationDuration);
    dispatch_after(endTime, dispatch_get_main_queue(), ^ (void) {
        [self.layer removeAllAnimations];
    });
}

- (void) updateTokens: (NSString *)tokenString {
    tokenLabel.text = tokenString;
}

@end
