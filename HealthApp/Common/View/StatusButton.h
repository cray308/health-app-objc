//
//  StatusButton.h
//  HealthApp
//
//  Created by Christopher Ray on 9/24/21.
//

#ifndef StatusButton_h
#define StatusButton_h

#import <UIKit/UIKit.h>

@interface StatusButton: UIView @end
@interface StatusButton() {
    @public UIView *box;
    @public UILabel *headerLabel;
    @public UIButton *button;
}
@end

id statusButton_init(CFStringRef text, bool hideViews, int tag, id target, SEL action);

#endif /* StatusButton_h */
