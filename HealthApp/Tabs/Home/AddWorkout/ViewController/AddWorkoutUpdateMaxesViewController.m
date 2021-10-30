#import <UIKit/UIKit.h>
#include "AddWorkoutCoordinator.h"
#include "ViewControllerHelpers.h"

@interface UpdateMaxesSheet: UIViewController<UITextFieldDelegate> @end
@interface UpdateMaxesSheet() {
    @public AddWorkoutCoordinator *delegate;
    @public Validator validator;
}
@end
@implementation UpdateMaxesSheet
- (void) dealloc {
    validator_free(&validator);
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.secondarySystemBackgroundColor);
    validator_setup(&validator, 8, true, self, @selector(dismissKeyboard));

    CFStringRef titles[4]; fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    UIStackView *stack = createStackView(nil, 0, 1, 0, (Padding){20,0,0,0});
    [self.view addSubview:stack];
    pin(stack, self.view.safeAreaLayoutGuide, (Padding){0}, EdgeBottom);

    for (int i = 0; i < 4; ++i)
        [stack addArrangedSubview:validator_add(&validator, self, titles[i], 1, 999)];

    UIButton *finishButton = createButton(localize(CFSTR("finish")), UIColor.systemBlueColor, 0,
                                          0, self, @selector(didPressFinish), -1);
    setNavButton(self.navigationItem, false, finishButton, self.view.frame.size.width);
    enableButton(finishButton, false);
    validator.button = finishButton;

    [stack release];
}

- (void) didPressFinish {
    delegate->workout->newLifts = malloc(4 * sizeof(short));
    for (int i = 0; i < 4; ++i)
        delegate->workout->newLifts[i] = validator.children[i].result;
    addWorkoutCoordinator_completedWorkout(delegate, true, false);
}

- (void) dismissKeyboard { [self.view endEditing:true]; }

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkInput(&validator, textField, (CFRange){range.location,range.length},_cfstr(string));
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    return [textField resignFirstResponder];
}
@end

id updateMaxesVC_init(AddWorkoutCoordinator *delegate) {
    UpdateMaxesSheet *this = [[UpdateMaxesSheet alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    return this;
}
