#import "TextFieldViewController.h"
#include "AddWorkoutCoordinator.h"

@interface UpdateMaxesSheet: TextFieldViewController @end
@interface UpdateMaxesSheet() {
    @public AddWorkoutCoordinator *delegate;
}
@end
@implementation UpdateMaxesSheet
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.secondarySystemBackgroundColor);
    validator_setup(&validator, 8, true, self);

    CFStringRef titles[4]; fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    validator.vStack = createStackView(nil, 0, 1, 0, (Padding){20,0,0,0});
    addVStackToScrollView(validator.vStack, validator.scrollView);

    for (int i = 0; i < 4; ++i)
        [validator.vStack addArrangedSubview:validator_add(&validator, self, titles[i], 1, 999)];

    UIButton *finishButton = createButton(localize(CFSTR("finish")), UIColor.systemBlueColor, 0,
                                          0, self, @selector(didPressFinish), -1);
    setNavButton(self.navigationItem, false, finishButton, self.view.frame.size.width);
    enableButton(finishButton, false);
    validator.button = finishButton;
}

- (void) didPressFinish {
    delegate->workout->newLifts = malloc(4 * sizeof(short));
    for (int i = 0; i < 4; ++i)
        delegate->workout->newLifts[i] = validator.children[i].result;
    addWorkoutCoordinator_completedWorkout(delegate, true, false);
}
@end

id updateMaxesVC_init(AddWorkoutCoordinator *delegate) {
    UpdateMaxesSheet *this = [[UpdateMaxesSheet alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    return this;
}
