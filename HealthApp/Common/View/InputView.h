//
//  InputView.h
//  HealthApp
//
//  Created by Christopher Ray on 9/24/21.
//

#ifndef InputView_h
#define InputView_h

#import <UIKit/UIKit.h>

@interface InputView: UIView
- (id) initWithDelegate: (id)delegate
              fieldHint: (CFStringRef)fieldHint tag: (int)tag min: (int)min max: (int)max;
- (void) toggleError: (bool)show;
@end

@interface InputView() {
    @public UILabel *hintLabel;
    @public UITextField *field;
    @public UILabel *errorLabel;
}
@end

#endif /* InputView_h */
