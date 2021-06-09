//
//  ConfettiEmitterView.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include <math.h>
#import "ConfettiEmitterView.h"

@interface ConfettiEmitterView() {
    CGPoint emitterCenter;
    CGSize emitterSize;
    CAEmitterCell *cells[16];
    CAEmitterLayer *particleLayer;
}

@end

@implementation ConfettiEmitterView

- (id) initWithFrame: (CGRect)frame {
    if (!(self = [super initWithFrame:frame])) return nil;
    emitterCenter = CGPointMake(frame.size.width / 2, 0);
    emitterSize = CGSizeMake(frame.size.width - 16, 1);
    self.backgroundColor = [UIColor.systemGrayColor colorWithAlphaComponent:0.8];
    [self setupCells];
    return self;
}

- (void) dealloc {
    [particleLayer release];
    for (int i = 0; i < 16; ++i) {
        [cells[i] release];
    }
    [super dealloc];
}

- (void) setupCells {
    UIColor *colors[] = {UIColor.systemRedColor, UIColor.systemBlueColor,
        UIColor.systemGreenColor, UIColor.systemYellowColor};
    UIImage *images[] = {[UIImage imageNamed:@"Box"], [UIImage imageNamed:@"Triangle"],
        [UIImage imageNamed:@"Circle"], [UIImage imageNamed:@"Spiral"]};
    int velocities[] = {100, 90, 150, 200};

    for (int i = 0; i < 16; ++i) {
        CAEmitterCell *cell = [[CAEmitterCell alloc] init];
        cell.birthRate = 4;
        cell.lifetime = 14;
        cell.lifetimeRange = 0;
        cell.velocity = velocities[arc4random_uniform(4)];
        cell.velocityRange = 0;
        cell.emissionLongitude = M_PI;
        cell.emissionRange = 0.5;
        cell.spin = 3.5;
        cell.spinRange = 0;
        cell.color = colors[i / 4].CGColor;
        cell.contents = (id) images[i % 4].CGImage;
        cell.scaleRange = 0.25;
        cell.scale = 0.1;
        cells[i] = cell;
    }

    particleLayer = [[CAEmitterLayer alloc] init];
    [self.layer addSublayer:particleLayer];
}

- (void) startAnimation {
    particleLayer.emitterPosition = emitterCenter;
    particleLayer.emitterShape = kCAEmitterLayerLine;
    particleLayer.emitterSize = emitterSize;
    particleLayer.emitterCells = [NSArray arrayWithObjects:cells count:16];
}

@end
