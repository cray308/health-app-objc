//
//  StatusButton.h
//  HealthApp
//
//  Created by Christopher Ray on 9/24/21.
//

#ifndef StatusButton_h
#define StatusButton_h

#import <UIKit/UIKit.h>
#include "ViewControllerHelpers.h"

@interface StatusButton: UIView
- (id) initWithButtonText: (CFStringRef)buttonText hideHeader: (bool)hideHeader
                  hideBox: (bool)hideBox tag: (int)tag target: (id)target action: (SEL)action;
- (void) updateWithLabelText: (CFStringRef)labelText buttonText: (CFStringRef)buttonText
                       state: (StatusViewState)state enable:(bool)enable;
- (void) updateState: (StatusViewState)state enable:(bool)enable;
@end

@interface StatusButton() {
    @public UIView *checkbox;
    @public UILabel *headerLabel;
    @public UIButton *button;
}
@end

#endif /* StatusButton_h */
