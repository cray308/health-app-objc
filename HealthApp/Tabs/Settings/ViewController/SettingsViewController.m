//
//  SettingsViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsViewController.h"
#import "SettingsViewModel.h"
#import "ViewControllerHelpers.h"
#import "SettingsTabCoordinator.h"
#import "NavBarCoinsView.h"
#import "AppCoordinator.h"
#import "AppUserData.h"
#import "PersistenceService.h"
#import "Constants.h"

@interface SettingsViewController() {
    SettingsViewModel *viewModel;
    NavBarCoinsView *navBarCoinsView;
    UILabel *tokenGoalLabel;
    UIStepper *tokenStepper;
    UITextField *usernameTextField;
    UIButton *saveButton;
}

@end

@implementation SettingsViewController

- (id) initWithViewModel: (SettingsViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [navBarCoinsView release];
    [usernameTextField release];
    [tokenGoalLabel release];
    [tokenStepper release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemBackgroundColor;
    self.navigationItem.title = @"App Settings";
    [self setupSubviews];
    appCoordinator_setTabToLoaded(viewModel->delegate->delegate, LoadedViewController_Settings);
}

- (void) setupSubviews {
    tokenGoalLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    tokenGoalLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    tokenGoalLabel.textColor = UIColor.labelColor;
    tokenGoalLabel.textAlignment = NSTextAlignmentLeft;

    tokenStepper = [[UIStepper alloc] initWithFrame:CGRectZero];
    tokenStepper.minimumValue = 1;
    tokenStepper.maximumValue = 50;
    tokenStepper.wraps = false;
    tokenStepper.autorepeat = true;
    [tokenStepper addTarget:self action:@selector(stepperChanged:) forControlEvents: UIControlEventValueChanged];

    UILabel *usernameLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    usernameLabel.text = @"Change Username:";
    usernameLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    usernameLabel.textColor = UIColor.labelColor;
    usernameLabel.textAlignment = NSTextAlignmentLeft;

    usernameTextField = [[UITextField alloc] initWithFrame:CGRectZero];
    usernameTextField.placeholder = @"New Username...";
    usernameTextField.borderStyle = UITextBorderStyleRoundedRect;
    usernameTextField.textColor = UIColor.labelColor;
    usernameTextField.delegate = self;

    UIStackView *hStackTokenGoal = [[UIStackView alloc] initWithArrangedSubviews:@[tokenGoalLabel, tokenStepper]];
    hStackTokenGoal.backgroundColor = UIColor.secondarySystemBackgroundColor;
    hStackTokenGoal.spacing = 5;
    [hStackTokenGoal setLayoutMarginsRelativeArrangement:true];
    hStackTokenGoal.layoutMargins = UIEdgeInsetsMake(4, 5, 4, 8);

    UIStackView *hStackUsername = [[UIStackView alloc] initWithArrangedSubviews:@[usernameLabel, usernameTextField]];
    hStackUsername.backgroundColor = UIColor.secondarySystemBackgroundColor;
    hStackUsername.spacing = 5;
    [hStackUsername setLayoutMarginsRelativeArrangement:true];
    hStackUsername.layoutMargins = UIEdgeInsetsMake(4, 5, 4, 8);

    saveButton = [UIButton buttonWithType:UIButtonTypeSystem];
    saveButton.translatesAutoresizingMaskIntoConstraints = false;
    saveButton.backgroundColor = UIColor.tertiarySystemBackgroundColor;
    [saveButton setTitle:@"Save Settings" forState:UIControlStateNormal];
    saveButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    saveButton.titleLabel.adjustsFontSizeToFitWidth = true;
    [saveButton setTitleColor:UIColor.systemBlueColor forState: UIControlStateNormal];
    [saveButton setTitleColor:UIColor.systemGrayColor forState: UIControlStateDisabled];
    [saveButton addTarget:self action:@selector(saveButtonPressed) forControlEvents:UIControlEventTouchUpInside];

    UIButton *deleteDataButton = [UIButton buttonWithType:UIButtonTypeSystem];
    deleteDataButton.translatesAutoresizingMaskIntoConstraints = false;
    deleteDataButton.backgroundColor = UIColor.tertiarySystemBackgroundColor;
    [deleteDataButton setTitle:@"Delete Workout Data" forState: UIControlStateNormal];
    deleteDataButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    deleteDataButton.titleLabel.adjustsFontSizeToFitWidth = true;
    [deleteDataButton setTitleColor:UIColor.systemRedColor forState:UIControlStateNormal];
    [deleteDataButton addTarget:self action:@selector(deleteButtonPressed) forControlEvents:UIControlEventTouchUpInside];

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[
        hStackTokenGoal, hStackUsername, saveButton, deleteDataButton
    ]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 20;
    [vStack setContentHuggingPriority:UILayoutPriorityDefaultHigh forAxis:UILayoutConstraintAxisVertical];
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = UIEdgeInsetsMake(20, 0, 0, 0);
    [self.view addSubview:vStack];

    UIView *paddingView = [[UIView alloc] initWithFrame:CGRectZero];
    paddingView.translatesAutoresizingMaskIntoConstraints = false;
    [paddingView setContentCompressionResistancePriority:UILayoutPriorityDefaultLow forAxis:UILayoutConstraintAxisVertical];
    [self.view addSubview:paddingView];

    [NSLayoutConstraint activateConstraints:@[
        [vStack.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [vStack.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [paddingView.topAnchor constraintEqualToAnchor:vStack.bottomAnchor],
        [paddingView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [paddingView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],

        [hStackTokenGoal.heightAnchor constraintEqualToConstant:40],
        [hStackUsername.heightAnchor constraintEqualToConstant:40],
        [saveButton.heightAnchor constraintEqualToConstant:40],
        [deleteDataButton.heightAnchor constraintEqualToConstant:40]
    ]];

    [vStack setCustomSpacing:40 afterView:hStackUsername];
    [vStack setCustomSpacing:40 afterView:saveButton];

    navBarCoinsView = [[NavBarCoinsView alloc] init];
    UIBarButtonItem *leftBarButtonItem = [[UIBarButtonItem alloc] initWithCustomView:navBarCoinsView];
    self.navigationItem.leftBarButtonItem = leftBarButtonItem;
    UITextField *fields[] = {usernameTextField, nil};
    viewController_createToolbar(self, @selector(dismissKeyboard), fields);

    /*

     Text fields for Squat, bench, weighted pull-up maxes

     picker for currently active plan


     */

    [paddingView release];
    [vStack release];
    [hStackUsername release];
    [usernameLabel release];
    [hStackTokenGoal release];
    [leftBarButtonItem release];
}

- (void) updateNavBarCoins: (NSString *)text {
    [navBarCoinsView updateTokens:text];
}

#pragma mark - Selectors

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (void) stepperChanged: (UIStepper *)sender {
    int value = sender.value;
    tokenGoalLabel.text = [NSString stringWithFormat:viewModel->tokenGoalLabelFormat, value];
}

- (void) saveButtonPressed {
    int goal = tokenStepper.value;
    NSString *name = [[NSString alloc] initWithString:usernameTextField.text];
    AppCoordinator *delegate = viewModel->delegate->delegate;

    AlertDetails *details = settingsViewModel_getAlertDetailsForSaveButton(goal, name);
    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"Save" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action _U_) {
        appCoordinator_updatedUserInfo(delegate);
    }];
    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil];
    viewController_showAlert(self, details, okAction, cancelAction);
}

- (void) deleteButtonPressed {
    AppCoordinator *delegate = viewModel->delegate->delegate;

    AlertDetails *details = settingsViewModel_getAlertDetailsForDeleteButton();
    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"Delete" style:UIAlertActionStyleDestructive handler:^(UIAlertAction * _Nonnull action _U_) {
        persistenceService_deleteUserData();
        appCoordinator_deletedAppData(delegate);
    }];
    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil];
    viewController_showAlert(self, details, okAction, cancelAction);
}

#pragma mark - TextField Delegate

- (BOOL) textField: (UITextField *)textField shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    if (range.length + range.location > textField.text.length) return false;
    size_t newLength = textField.text.length + string.length - range.length;
    [saveButton setEnabled:newLength != 0];
    return newLength <= MAX_USERNAME_LEN;
}

-(BOOL) textFieldShouldReturn: (UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}

@end
