#import <UIKit/UIKit.h>
#include "ViewControllerHelpers.h"

@interface TextFieldViewController: UIViewController<UITextFieldDelegate> @end
@interface TextFieldViewController() {
    @public Validator validator;
}
@end
