//
//  AddWorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutViewController.h"
#import "SingleActivityTableViewCell.h"
#import "AddWorkoutCoordinator.h"
#import "AddWorkoutViewModel.h"
#import "CustomTokenPopup.h"
#import "CustomTokenPopupViewModel.h"

#define ReuseIdentifier @"SingleActivityTableViewCell"

@interface AddWorkoutViewController() {
    UITableView *tableView;
    AddWorkoutViewModel *viewModel;
    UIButton *saveButton;
}

@end

@implementation AddWorkoutViewController

- (id) initWithViewModel: (AddWorkoutViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [tableView release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;
    self.navigationItem.title = @"Add Workouts";
    [self setupSubviews];
    [saveButton setEnabled:false];
}

- (void) setupSubviews {
    UIButton *plusButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [plusButton setImage:[UIImage systemImageNamed:@"plus"] forState:UIControlStateNormal];
    plusButton.frame = CGRectMake(0, 0, 30, 30);
    [plusButton addTarget:self action:@selector(pressedPlusButton) forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem *rightItem = [[UIBarButtonItem alloc] initWithCustomView:plusButton];
    self.navigationItem.rightBarButtonItem = rightItem;

    saveButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [saveButton setTitle:@"Save Workouts" forState:UIControlStateNormal];
    saveButton.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    [saveButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [saveButton setTitleColor:UIColor.systemGrayColor forState:UIControlStateDisabled];
    saveButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    saveButton.titleLabel.adjustsFontSizeToFitWidth = true;
    saveButton.frame = CGRectMake(0, 0, 1, 40);
    [saveButton addTarget:self action:@selector(pressedSaveButton) forControlEvents:UIControlEventTouchUpInside];

    tableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
    tableView.translatesAutoresizingMaskIntoConstraints = false;
    tableView.backgroundColor = UIColor.systemGroupedBackgroundColor;
    [tableView registerClass:SingleActivityTableViewCell.class forCellReuseIdentifier:ReuseIdentifier];
    tableView.dataSource = self;
    tableView.delegate = self;
    tableView.tableFooterView = saveButton;
    [self.view addSubview:tableView];

    [NSLayoutConstraint activateConstraints:@[
        [tableView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [tableView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor],
        [tableView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [tableView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor]
    ]];

    [rightItem release];
}

- (void) insertEntryInRow: (int)row {
    [tableView beginUpdates];
    [tableView insertRowsAtIndexPaths:@[[NSIndexPath indexPathForRow:row inSection:0]] withRowAnimation:UITableViewRowAnimationAutomatic];
    [tableView endUpdates];
    [saveButton setEnabled:true];
}

- (void) pressedSaveButton {
    addWorkoutViewModel_tappedSaveButton(viewModel);
}

- (void) pressedPlusButton {
    addWorkoutCoordinator_pushModalViewController(viewModel->delegate);
}

- (void) showTokenPopup {
    CGFloat popupWidth = 4 * self.view.frame.size.width / 5;
    CustomTokenPopupViewModel *popupVM = customTokenPopupViewModel_init(viewModel->delegate, popupWidth, viewModel->newTokens);

    CustomTokenPopup *popupView = [[CustomTokenPopup alloc] initWithViewModel:popupVM];
    popupView.translatesAutoresizingMaskIntoConstraints = false;
    [self.view addSubview:popupView];

    [NSLayoutConstraint activateConstraints: @[
        [popupView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [popupView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [popupView.topAnchor constraintEqualToAnchor:self.view.topAnchor],
        [popupView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]
    ]];
    [popupView release];
}

#pragma mark - TableView Delegate

- (NSInteger) numberOfSectionsInTableView: (UITableView *)tableView { return 1; }

- (NSInteger) tableView: (UITableView *)tableView numberOfRowsInSection: (NSInteger)section {
    return array_size(viewModel->workouts);
}

- (UITableViewCell *) tableView: (UITableView *)tableView cellForRowAtIndexPath: (NSIndexPath *)indexPath {
    SingleActivityTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:ReuseIdentifier forIndexPath:indexPath];
    if (!cell) return [[UITableViewCell alloc] init];

    SingleActivityModel *model = &viewModel->workouts->arr[indexPath.row];
    [cell configureWithIntensity:viewModel->intensityStrings[model->intensity] duration:model->duration];
    return cell;
}

- (CGFloat) tableView: (UITableView *)tableView heightForRowAtIndexPath: (NSIndexPath *)indexPath { return 50; }

- (BOOL) tableView: (UITableView *)tableView canEditRowAtIndexPath: (NSIndexPath *)indexPath { return true; }

- (void) tableView: (UITableView *)tableView commitEditingStyle: (UITableViewCellEditingStyle)editingStyle forRowAtIndexPath: (NSIndexPath *)indexPath {
    if (editingStyle != UITableViewCellEditingStyleDelete) return;

    array_erase_SingleActivityModel(viewModel->workouts, (int) indexPath.row, 1);
    [tableView beginUpdates];
    [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    [tableView endUpdates];
    [saveButton setEnabled:viewModel->workouts->size];
}

@end
