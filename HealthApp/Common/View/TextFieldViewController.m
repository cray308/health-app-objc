#import "TextFieldViewController.h"

@implementation TextFieldViewController
- (void) dealloc {
    validator_free(&validator, self);
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    validator.scrollView = createScrollView();
    [self.view addSubview:validator.scrollView];
    pin(validator.scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
}

- (void) viewDidAppear: (BOOL)animated {
    [super viewDidAppear:animated];
    if (!validator.scrollHeight) validator_getScrollHeight(&validator);
}

- (void) dismissKeyboard { [self.view endEditing:true]; }

- (void) keyboardShown: (NSNotification *)notif {
    validator_handleKeyboardShow(&validator, self.view, notif);
}

- (void) keyboardWillHide: (NSNotification *)notif { validator_handleKeyboardHide(&validator); }

- (void) textFieldDidBeginEditing: (UITextField *)field { validator.activeField = field; }

- (void) textFieldDidEndEditing: (UITextField *)field { validator.activeField = nil; }

- (BOOL) textField: (UITextField *)field
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkInput(&validator, field, (CFRange){range.location,range.length},_cfstr(string));
}

- (BOOL) textFieldShouldReturn: (UITextField *)field { return [field resignFirstResponder]; }
@end
