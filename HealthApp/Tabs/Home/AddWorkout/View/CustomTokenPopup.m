//
//  CustomTokenPopup.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "CustomTokenPopup.h"
#import "ImageHelpers.h"
#import "CustomTokenPopupViewModel.h"

@interface CustomTokenPopup() {
    CustomTokenPopupViewModel *viewModel;
    UIImageView *coinRows[2][6];
}

@end

@implementation CustomTokenPopup

- (id) initWithViewModel: (CustomTokenPopupViewModel *)model {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    viewModel = model;
    self.backgroundColor = [UIColor.systemGrayColor colorWithAlphaComponent:0.5];
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [viewModel->message release];
    free(viewModel);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; coinRows[i][j]; ++j) {
            [coinRows[i][j] release];
        }
    }
    [super dealloc];
}

- (void) setupSubviews {
    UIView *alertForeground = [[UIView alloc] initWithFrame:CGRectZero];
    alertForeground.translatesAutoresizingMaskIntoConstraints = false;
    alertForeground.backgroundColor = UIColor.systemBackgroundColor;
    alertForeground.layer.cornerRadius = 10;
    [self addSubview:alertForeground];

    UILabel *alertTitle = [[UILabel alloc] initWithFrame:CGRectZero];
    alertTitle.translatesAutoresizingMaskIntoConstraints = false;
    alertTitle.text = @"Workouts Added Successfully!";
    alertTitle.font = [UIFont systemFontOfSize:18];
    alertTitle.adjustsFontSizeToFitWidth = true;
    alertTitle.textColor = UIColor.labelColor;
    alertTitle.textAlignment = NSTextAlignmentCenter;
    [self addSubview:alertTitle];

    UILabel *alertMessage = [[UILabel alloc] initWithFrame:CGRectZero];
    alertMessage.translatesAutoresizingMaskIntoConstraints = false;
    alertMessage.text = viewModel->message;
    alertMessage.font = [UIFont systemFontOfSize:16];
    alertMessage.adjustsFontSizeToFitWidth = true;
    alertMessage.textColor = UIColor.labelColor;
    alertMessage.textAlignment = NSTextAlignmentCenter;
    [self addSubview:alertMessage];

    CGFloat imgSize = viewModel->imageDimension;
    UIImage *animatedImage = [UIImage animatedImageWithImages:@[
        resizeImage([UIImage imageNamed:@"token1"], imgSize), resizeImage([UIImage imageNamed:@"token2"], imgSize),
        resizeImage([UIImage imageNamed:@"token3"], imgSize), resizeImage([UIImage imageNamed:@"token4"], imgSize),
        resizeImage([UIImage imageNamed:@"token5"], imgSize), resizeImage([UIImage imageNamed:@"token6"], imgSize),
        resizeImage([UIImage imageNamed:@"token7"], imgSize), resizeImage([UIImage imageNamed:@"token8"], imgSize)
    ] duration:0.5];

    for (int i = 0; i < 2; ++i) {
        int nTokens = viewModel->tokensForRow[i];
        for (int j = 0; j < nTokens; ++j) {
            UIImageView *imgView = [[UIImageView alloc] initWithImage:animatedImage];
            imgView.translatesAutoresizingMaskIntoConstraints = false;
            imgView.clipsToBounds = true;
            imgView.contentMode = UIViewContentModeScaleAspectFill;
            coinRows[i][j] = imgView;
            [self addSubview:imgView];
        }
        coinRows[i][nTokens] = nil;
    }

    UIButton *endButton = [UIButton buttonWithType:UIButtonTypeSystem];
    endButton.translatesAutoresizingMaskIntoConstraints = false;
    [endButton setTitle:@"OK" forState:UIControlStateNormal];
    endButton.backgroundColor = UIColor.tertiarySystemBackgroundColor;
    [endButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [endButton setEnabled:true];
    endButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    endButton.layer.cornerRadius = 10;
    [endButton addTarget:self action:@selector(endAlert) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:endButton];

    [NSLayoutConstraint activateConstraints:@[
        [alertForeground.centerXAnchor constraintEqualToAnchor:self.centerXAnchor],
        [alertForeground.widthAnchor constraintEqualToConstant:viewModel->foregroundWidth],
        [alertForeground.centerYAnchor constraintEqualToAnchor:self.centerYAnchor],
        [alertForeground.bottomAnchor constraintEqualToAnchor:endButton.bottomAnchor constant:10],

        [alertTitle.leadingAnchor constraintEqualToAnchor:alertForeground.leadingAnchor],
        [alertTitle.trailingAnchor constraintEqualToAnchor:alertForeground.trailingAnchor],
        [alertTitle.topAnchor constraintEqualToAnchor:alertForeground.topAnchor constant:5],
        [alertTitle.heightAnchor constraintEqualToConstant:40],

        [alertMessage.leadingAnchor constraintEqualToAnchor:alertForeground.leadingAnchor],
        [alertMessage.trailingAnchor constraintEqualToAnchor:alertForeground.trailingAnchor],
        [alertMessage.topAnchor constraintEqualToAnchor:alertTitle.bottomAnchor constant:5],
        [alertMessage.heightAnchor constraintEqualToConstant:30]
    ]];

    [self setConstraintsForCoinRow:coinRows[0] leadingAnchor:alertForeground.leadingAnchor
                         topAnchor:alertMessage.bottomAnchor spacing:viewModel->spacingForRow[0]];
    NSLayoutYAxisAnchor *anchorForButton = coinRows[0][0].bottomAnchor;

    if (viewModel->tokensForRow[1]) {
        [self setConstraintsForCoinRow:coinRows[1] leadingAnchor:alertForeground.leadingAnchor
                             topAnchor:anchorForButton spacing:viewModel->spacingForRow[1]];
        anchorForButton = coinRows[1][0].bottomAnchor;
    }

    [NSLayoutConstraint activateConstraints:@[
        [endButton.topAnchor constraintEqualToAnchor:anchorForButton constant:20],
        [endButton.centerXAnchor constraintEqualToAnchor:self.centerXAnchor],
        [endButton.widthAnchor constraintEqualToConstant:viewModel->foregroundWidth / 3]
    ]];

    [alertMessage release];
    [alertTitle release];
    [alertForeground release];
}

- (void) setConstraintsForCoinRow: (UIImageView **)views leadingAnchor: (NSLayoutXAxisAnchor *)leadingAnchor
                        topAnchor: (NSLayoutYAxisAnchor *)topAnchor spacing: (CGFloat)spacing {
    CGFloat imgSize = viewModel->imageDimension;
    [NSLayoutConstraint activateConstraints:@[
        [views[0].topAnchor constraintEqualToAnchor:topAnchor constant:10],
        [views[0].leadingAnchor constraintEqualToAnchor:leadingAnchor constant:spacing],
        [views[0].widthAnchor constraintEqualToConstant:imgSize],
        [views[0].heightAnchor constraintEqualToConstant:imgSize]
    ]];

    for (int i = 1; views[i]; ++i) {
        [NSLayoutConstraint activateConstraints:@[
            [views[i].topAnchor constraintEqualToAnchor:topAnchor constant:10],
            [views[i].leadingAnchor constraintEqualToAnchor:views[i - 1].trailingAnchor constant:spacing],
            [views[i].widthAnchor constraintEqualToConstant:imgSize],
            [views[i].heightAnchor constraintEqualToConstant:imgSize]
        ]];
    }
}

- (void) endAlert {
    // Change to static image and set up transform
    UIImage *staticImage = resizeImage([UIImage imageNamed:TokenIconImageName], viewModel->imageDimension);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; coinRows[i][j]; ++j) {
            UIImageView *view = coinRows[i][j];
            view.image = staticImage;
            [UIView animateWithDuration:2 animations:^ (void) {
                CGAffineTransform transform = CGAffineTransformIdentity;
                transform = CGAffineTransformScale(transform, 0.25, 0.25);
                CGFloat dx = -4 * view.frame.origin.x;
                CGFloat dy = -4 * view.frame.origin.y;
                view.transform = CGAffineTransformTranslate(transform, dx, dy);
            }];
        }
    }

    dispatch_time_t endTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t) NSEC_PER_SEC * 2);
    dispatch_after(endTime, dispatch_get_main_queue(), ^ (void) {
        customTokenPopupViewModel_tappedOkButton(viewModel, self);
    });
}

@end
