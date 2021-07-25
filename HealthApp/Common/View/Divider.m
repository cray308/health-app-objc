//
//  Divider.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "Divider.h"

UIView *createDivider(void) {
    UIView *view = [[UIView alloc] initWithFrame:CGRectZero];
    view.translatesAutoresizingMaskIntoConstraints = false;
    view.backgroundColor = UIColor.separatorColor;
    [view.heightAnchor constraintEqualToConstant:1].active = true;
    return view;
}
