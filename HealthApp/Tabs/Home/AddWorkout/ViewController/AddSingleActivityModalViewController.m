//
//  AddSingleActivityModalViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddSingleActivityModalViewController.h"
#import "SingleActivityViewModel.h"
#import "ViewControllerHelpers.h"
#include "unordered_set.h"

gen_uset(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal, DSDefault_shallowCopy, DSDefault_shallowDelete)

@interface AddSingleActivityModalViewController() {
    AddSingleActivityViewModel *viewModel;
    USet_char *durationChars;
    UITextField *durationTextField;
    UISegmentedControl *intensityPicker;
    UIButton *addActivityButton;
}

@end

@implementation AddSingleActivityModalViewController

- (id) initWithViewModel: (AddSingleActivityViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    unsigned short tempArr[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    durationChars = uset_new_fromArray(char, tempArr, 10);
    return self;
}

- (void) dealloc {
    free(viewModel);
    uset_free(char, durationChars);
    [durationTextField release];
    [intensityPicker release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.secondarySystemBackgroundColor;
    [self setupSubviews];
    UITextField *fields[] = {durationTextField, nil};
    viewController_createToolbar(self, @selector(dismissKeyboard), fields);
    [addActivityButton setEnabled:false];
}

- (void) setupSubviews {
    UILabel *durationLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    durationLabel.translatesAutoresizingMaskIntoConstraints = false;
    durationLabel.text = @"Duration (in minutes)";
    durationLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleFootnote];
    durationLabel.textAlignment = NSTextAlignmentLeft;
    [self.view addSubview:durationLabel];

    durationTextField = [[UITextField alloc] initWithFrame:CGRectZero];
    durationTextField.translatesAutoresizingMaskIntoConstraints = false;
    durationTextField.delegate = self;
    durationTextField.backgroundColor = UIColor.tertiarySystemBackgroundColor;
    durationTextField.placeholder = @"Duration";
    durationTextField.textAlignment = NSTextAlignmentLeft;
    durationTextField.borderStyle = UITextBorderStyleRoundedRect;
    durationTextField.keyboardType = UIKeyboardTypeNumberPad;
    [self.view addSubview:durationTextField];

    UILabel *intensityLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    intensityLabel.translatesAutoresizingMaskIntoConstraints = false;
    intensityLabel.text = @"Activity Intensity";
    intensityLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleFootnote];
    intensityLabel.textAlignment = NSTextAlignmentLeft;
    [self.view addSubview:intensityLabel];

    intensityPicker = [[UISegmentedControl alloc] initWithItems:@[@"Low", @"Medium", @"High"]];
    intensityPicker.translatesAutoresizingMaskIntoConstraints = false;
    intensityPicker.selectedSegmentIndex = 0;
    intensityPicker.layer.cornerRadius = 5;
    intensityPicker.tintColor = UIColor.systemGray2Color;
    [intensityPicker addTarget:self action:@selector(updateIntensity:) forControlEvents:UIControlEventValueChanged];
    [self.view addSubview:intensityPicker];

    addActivityButton = [UIButton buttonWithType:UIButtonTypeSystem];
    addActivityButton.translatesAutoresizingMaskIntoConstraints = false;
    [addActivityButton setTitle:@"Add Activity" forState:UIControlStateNormal];
    [addActivityButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [addActivityButton setTitleColor:UIColor.systemGrayColor forState:UIControlStateDisabled];
    addActivityButton.frame = CGRectMake(0, 0, self.view.frame.size.width / 3, 30);
    [addActivityButton addTarget:self action:@selector(didPressAddActivity) forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem *rightItem = [[UIBarButtonItem alloc] initWithCustomView:addActivityButton];
    self.navigationItem.rightBarButtonItem = rightItem;

    [NSLayoutConstraint activateConstraints:@[
        [durationLabel.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:30],
        [durationLabel.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor constant:16],
        [durationLabel.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [durationLabel.heightAnchor constraintEqualToConstant:20],

        [durationTextField.topAnchor constraintEqualToAnchor:durationLabel.bottomAnchor constant:2],
        [durationTextField.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [durationTextField.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [durationTextField.heightAnchor constraintEqualToConstant:40],

        [intensityLabel.topAnchor constraintEqualToAnchor:durationTextField.bottomAnchor constant:20],
        [intensityLabel.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor constant:16],
        [intensityLabel.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [intensityLabel.heightAnchor constraintEqualToConstant:20],

        [intensityPicker.topAnchor constraintEqualToAnchor:intensityLabel.bottomAnchor constant:2],
        [intensityPicker.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [intensityPicker.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [intensityPicker.heightAnchor constraintEqualToConstant:40]
    ]];

    [rightItem release];
    [durationLabel release];
    [intensityLabel release];
}

- (void) didPressAddActivity {
    addSingleActivityViewModel_tappedAddActivity(viewModel, self);
}

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (void) updateIntensity: (UISegmentedControl *)sender {
    viewModel->intensityIndex = (int) sender.selectedSegmentIndex;
}

- (BOOL) textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
    size_t len = string.length;
    if (len) {
        unsigned short buf[len + 1];
        [string getCharacters:buf range:NSMakeRange(0, len)];
        buf[len] = 0;
        for (unsigned short *ptr = buf; *ptr; ++ptr) {
            if (!uset_contains(char, durationChars, *ptr)) return false;
        }
    }

    NSString *initialText = textField.text ? textField.text : @"";
    CFStringRef newText = CFBridgingRetain([initialText stringByReplacingCharactersInRange:range withString:string]);
    int newDuration = CFStringGetIntValue(newText);
    viewModel->duration = newDuration;
    [addActivityButton setEnabled:newDuration > 0 && newDuration < 256];
    CFRelease(newText);
    return true;
}

- (BOOL) textFieldShouldReturn:(UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}

@end
