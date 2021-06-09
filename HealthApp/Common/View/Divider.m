//
//  Divider.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "Divider.h"

@interface Divider()
@end

@implementation Divider

- (id) init {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self.translatesAutoresizingMaskIntoConstraints = false;
    self.backgroundColor = UIColor.separatorColor;
    [self.heightAnchor constraintEqualToConstant:1].active = true;
    return self;
}

@end
