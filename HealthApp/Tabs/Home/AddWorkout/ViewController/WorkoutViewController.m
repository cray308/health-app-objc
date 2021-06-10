//
//  WorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#import "WorkoutViewController.h"
#import "AddWorkoutViewModel.h"
#import "Divider.h"

NSString *exercise_getTitleString(ExerciseEntry *e) {
    switch (e->type) {
        case ExerciseTypeReps:
            if (e->weight > 1) {
                return [[NSString alloc] initWithFormat:@"%@ x %u @ %u lbs", e->name, e->reps, e->weight];
            }
            return [[NSString alloc] initWithFormat:@"%@ x %u", e->name, e->reps];

        case ExerciseTypeDuration:
            if (e->reps > 120) {
                double minutes = (double) e->reps / 60.0;
                return [[NSString alloc] initWithFormat:@"%@ for %.1f mins", e->name, minutes];
            }
            return [[NSString alloc] initWithFormat:@"%@ for %u sec", e->name, e->reps];

        default: ;
            unsigned int rowingDist = (5 * e->reps) / 4;
            return [[NSString alloc] initWithFormat:@"%@ %u/%u meters", e->name, e->reps, rowingDist];
    }
}

@interface ExerciseView: UIView

- (id) initWithExercise: (ExerciseEntry *)exercise;
- (UIButton *) getButton;
- (bool) handleTap;
- (void) reset;

@end

@interface ExerciseView() {
    ExerciseEntry *exercise;
    int completedSets;
    bool resting;
    NSString *restStr;
    UILabel *setsLabel;
    UIView *checkbox;
    UIButton *button;
}

@end

@implementation ExerciseView

- (id) initWithExercise: (ExerciseEntry *)exerciseEntry {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    exercise = exerciseEntry;
    if (exerciseEntry->rest) {
        restStr = [[NSString alloc] initWithFormat:@"Rest: %u s", exerciseEntry->rest];
    }
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    [setsLabel release];
    [checkbox release];
    if (restStr) [restStr release];
    [super dealloc];
}

- (void) setupSubviews {
    button = [UIButton buttonWithType:UIButtonTypeSystem];
    button.translatesAutoresizingMaskIntoConstraints = false;
    [button setTitle:@"" forState:UIControlStateNormal];
    [button setTitleColor:UIColor.labelColor forState: UIControlStateNormal];
    [button setTitleColor:UIColor.secondaryLabelColor forState:UIControlStateDisabled];
    button.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    button.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    button.layer.cornerRadius = 5;

    setsLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    setsLabel.translatesAutoresizingMaskIntoConstraints = false;
    setsLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
    setsLabel.adjustsFontSizeToFitWidth = true;
    if (exercise->sets > 1) {
        NSString *s = [[NSString alloc] initWithFormat:@"Set 1 of %u", exercise->sets];
        setsLabel.text = s;
        [s release];
    }
    setsLabel.textColor = UIColor.labelColor;

    checkbox = [[UIView alloc] initWithFrame:CGRectZero];
    checkbox.translatesAutoresizingMaskIntoConstraints = false;
    checkbox.backgroundColor = UIColor.systemGrayColor;
    checkbox.layer.cornerRadius = 5;

    [self addSubview:setsLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    [NSLayoutConstraint activateConstraints:@[
        [setsLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [setsLabel.heightAnchor constraintEqualToConstant:20],
        [setsLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [setsLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],

        [button.topAnchor constraintEqualToAnchor:setsLabel.bottomAnchor constant:5],
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor constant:-5],

        [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
        [checkbox.widthAnchor constraintEqualToConstant:20],
        [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
    ]];
}

- (bool) handleTap {
    if (!button.enabled) {
        [button setEnabled:true];
        checkbox.backgroundColor = UIColor.systemOrangeColor;
    } else if (restStr && !resting) {
        resting = true;
        [button setTitle:restStr forState:UIControlStateNormal];
    } else {
        resting = false;
        if (++exercise->completedSets == exercise->sets) {
            [button setEnabled:false];
            checkbox.backgroundColor = UIColor.systemGreenColor;
            return true;
        }

        NSString *setsText = [[NSString alloc] initWithFormat:@"Set %u of %u", exercise->completedSets + 1, exercise->sets];
        setsLabel.text = setsText;
        [setsText release];

        NSString *title = exercise_getTitleString(exercise);
        [button setTitle:title forState:UIControlStateNormal];
        [title release];
    }
    return false;
}

- (UIButton *) getButton {
    return button;
}

- (void) reset {
    checkbox.backgroundColor = UIColor.systemGrayColor;
    [button setEnabled:false];
    resting = false;
    exercise->completedSets = 0;
    NSString *title = exercise_getTitleString(exercise);
    [button setTitle:title forState:UIControlStateNormal];
    [title release];
}

@end

@interface ExerciseContainer: UIView

- (id) initWithGroup: (ExerciseGroup *)exerciseGroup parent: (WorkoutViewController *)parentVC;
- (void) startCircuit;

@end

@interface ExerciseContainer() {
    WorkoutViewController *parent;
    ExerciseGroup *group;
    UILabel *headerLabel;
    ExerciseView **viewsArr;
    UIButton *amrapBtn;
    int currentIndex;
}

@end

@implementation ExerciseContainer

- (id) initWithGroup: (ExerciseGroup *)exerciseGroup parent: (WorkoutViewController *)parentVC {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    group = exerciseGroup;
    parent = parentVC;
    if (exerciseGroup->type == ExerciseContainerTypeDecrement) {
        ExerciseEntry *e = array_at(exEntry, group->exercises, 0);
        group->completedReps = e ? e->reps : 10;
    }
    viewsArr = calloc(group->exercises->size, sizeof(ExerciseView *));
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    for (size_t i = 0; i < group->exercises->size; ++i) { [viewsArr[i] release]; }
    free(viewsArr);
    [headerLabel release];
    [super dealloc];
}

- (void) setupSubviews {
    headerLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    headerLabel.translatesAutoresizingMaskIntoConstraints = false;
    headerLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle3];
    headerLabel.adjustsFontSizeToFitWidth = true;
    if (group->type == ExerciseContainerTypeRounds && group->reps > 1) {
        NSString *s = [[NSString alloc] initWithFormat:@"Round 1 of %u", group->reps];
        headerLabel.text = s;
        [s release];
    } else if (group->type == ExerciseContainerTypeAMRAP) {
        NSString *s = [[NSString alloc] initWithFormat:@"AMRAP %u mins", group->reps];
        headerLabel.text = s;
        [s release];
    }
    headerLabel.textColor = UIColor.labelColor;
    [self addSubview:headerLabel];

    UIStackView *exerciseStack = [[UIStackView alloc] initWithFrame:CGRectZero];
    exerciseStack.translatesAutoresizingMaskIntoConstraints = false;
    exerciseStack.axis = UILayoutConstraintAxisVertical;
    exerciseStack.spacing = 5;
    [exerciseStack setLayoutMarginsRelativeArrangement:true];
    exerciseStack.layoutMargins = UIEdgeInsetsMake(5, 4, 4, 0);
    [self addSubview:exerciseStack];

    [NSLayoutConstraint activateConstraints:@[
        [headerLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [headerLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [headerLabel.widthAnchor constraintEqualToAnchor:self.widthAnchor multiplier:0.6],
        [headerLabel.heightAnchor constraintEqualToConstant: 20],

        [exerciseStack.topAnchor constraintEqualToAnchor:headerLabel.bottomAnchor],
        [exerciseStack.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [exerciseStack.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [exerciseStack.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
    ]];

    if (group->type == ExerciseContainerTypeAMRAP) {
        amrapBtn = [UIButton buttonWithType:UIButtonTypeSystem];
        amrapBtn.translatesAutoresizingMaskIntoConstraints = false;
        [amrapBtn setTitle:@"Finish Circuit" forState:UIControlStateNormal];
        [amrapBtn setTitleColor:UIColor.systemBlueColor forState: UIControlStateNormal];
        [amrapBtn setTitleColor:UIColor.secondaryLabelColor forState:UIControlStateDisabled];
        amrapBtn.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
        amrapBtn.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
        amrapBtn.layer.cornerRadius = 5;
        [amrapBtn addTarget:self action:@selector(finishGroup) forControlEvents:UIControlEventTouchUpInside];
        [self addSubview:amrapBtn];
        [NSLayoutConstraint activateConstraints:@[
            [amrapBtn.topAnchor constraintEqualToAnchor:self.topAnchor],
            [amrapBtn.leadingAnchor constraintEqualToAnchor:headerLabel.trailingAnchor],
            [amrapBtn.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
            [amrapBtn.heightAnchor constraintEqualToConstant: 20]
        ]];
        [amrapBtn setEnabled:false];
    }

    ExerciseEntry *e;
    Array_exEntry *exercises = group->exercises;
    int i = 0;
    array_iter(exercises, e) {
        ExerciseView *v = [[ExerciseView alloc] initWithExercise:e];
        UIButton *btn = [v getButton];
        btn.tag = i;
        [btn addTarget:self action:@selector(handleTap:) forControlEvents:UIControlEventTouchUpInside];
        [v reset];
        [exerciseStack addArrangedSubview:v];
        viewsArr[i++] = v;
    }
    [exerciseStack release];
}

- (void) startCircuit {
    currentIndex = 0;
    if (amrapBtn) [amrapBtn setEnabled:true];
    for (int i = 0; i < array_size(group->exercises); ++i) {
        [viewsArr[i] reset];
    }
    [viewsArr[0] handleTap];
}

- (void) finishGroup {
    [parent finishedExerciseGroup];
}

- (void) handleTap: (UIButton *)btn {
    ExerciseView *v = viewsArr[btn.tag];
    if ([v handleTap]) {
        if (++currentIndex == array_size(group->exercises)) {
            switch (group->type) {
                case ExerciseContainerTypeRounds:
                    if (++group->completedReps == group->reps) {
                        [self finishGroup];
                        return;
                    }

                    NSString *h = [[NSString alloc] initWithFormat:@"Round %u of %u", group->completedReps + 1, group->reps];
                    headerLabel.text = h;
                    [h release];
                    break;

                case ExerciseContainerTypeDecrement:
                    if (--group->completedReps == 0) {
                        [self finishGroup];
                        return;
                    }
                    ExerciseEntry *e;
                    Array_exEntry *exercises = group->exercises;
                    array_iter(exercises, e) {
                        if (e->type == ExerciseTypeReps) {
                            e->reps -= 1;
                        }
                    }
                    break;

                default:
                    break;
            }
            [self startCircuit];
        } else {
            [viewsArr[currentIndex] handleTap];
        }
    }
}

@end

@interface WorkoutViewController() {
    AddWorkoutViewModel *viewModel;
    UIStackView *groupsStack;
    int currentIndex;
    double startTime;
}

@end

@implementation WorkoutViewController

- (id) initWithViewModel: (AddWorkoutViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    [groupsStack release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;
    self.navigationItem.title = viewModel->workout->title;
    [self setupSubviews];
}

- (void) setupSubviews {
    UIButton *startBtn = [UIButton buttonWithType:UIButtonTypeSystem];
    startBtn.translatesAutoresizingMaskIntoConstraints = false;
    [startBtn setTitle:@"Start" forState:UIControlStateNormal];
    [startBtn setTitleColor:UIColor.systemGreenColor forState: UIControlStateNormal];
    startBtn.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
    startBtn.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    startBtn.layer.cornerRadius = 5;
    [startBtn addTarget:self action:@selector(startEndWorkout:) forControlEvents:UIControlEventTouchUpInside];

    UIView *btnContainer = [[UIView alloc] initWithFrame:CGRectZero];
    [btnContainer addSubview:startBtn];

    groupsStack = [[UIStackView alloc] initWithFrame:CGRectZero];
    groupsStack.axis = UILayoutConstraintAxisVertical;
    groupsStack.spacing = 20;
    [groupsStack setLayoutMarginsRelativeArrangement:true];
    groupsStack.layoutMargins = UIEdgeInsetsMake(0, 4, 4, 0);

    Array_exGroup *groups = viewModel->workout->activities;
    ExerciseGroup *g;
    int i = 0;
    array_iter(groups, g) {
        if (i++ > 0) {
            Divider *d = [[Divider alloc] init];
            [groupsStack addArrangedSubview:d];
            [d release];
        }
        ExerciseContainer *v = [[ExerciseContainer alloc] initWithGroup:g parent:self];
        [groupsStack addArrangedSubview:v];
        [v release];
    }

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[btnContainer, groupsStack]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 20;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = UIEdgeInsetsMake(10, 0, 0, 0);

    UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:CGRectZero];
    scrollView.translatesAutoresizingMaskIntoConstraints = false;
    scrollView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    scrollView.bounces = true;
    scrollView.showsVerticalScrollIndicator = true;
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];

    [NSLayoutConstraint activateConstraints:@[
        [scrollView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [startBtn.topAnchor constraintEqualToAnchor:btnContainer.topAnchor],
        [startBtn.bottomAnchor constraintEqualToAnchor:btnContainer.bottomAnchor],
        [startBtn.leadingAnchor constraintEqualToAnchor:btnContainer.leadingAnchor],
        [startBtn.widthAnchor constraintEqualToAnchor:btnContainer.widthAnchor multiplier:0.4],
        [startBtn.heightAnchor constraintEqualToConstant: 30],
    ]];

    [btnContainer release];
    [vStack release];
    [scrollView release];
}

- (void) startEndWorkout: (UIButton *)btn {
    if ([btn.titleLabel.text caseInsensitiveCompare:@"start"] == NSOrderedSame) { // tapped start
        [btn setTitle:@"End" forState:UIControlStateNormal];
        [btn setTitleColor:UIColor.systemRedColor forState:UIControlStateNormal];
        ExerciseContainer *v =  (ExerciseContainer *) groupsStack.arrangedSubviews[0];
        startTime = CFAbsoluteTimeGetCurrent();
        [v startCircuit];
    } else {
        NSLog(@"early exit");
        unsigned int minutes = (unsigned int) ((CFAbsoluteTimeGetCurrent() - startTime) / 60.0);
        addWorkoutViewModel_stoppedWorkout(viewModel, minutes);
    }
}

- (void) finishedExerciseGroup {
    NSArray<UIView *> *views = groupsStack.arrangedSubviews;

    if (views.count >= 3) {
        ExerciseContainer *next = (ExerciseContainer *) views[2];
        [views[0] removeFromSuperview];
        [views[1] removeFromSuperview];
        [next startCircuit];
    } else {
        NSLog(@"finished workout entirely");
        unsigned int minutes = (unsigned int) ((CFAbsoluteTimeGetCurrent() - startTime) / 60.0);
        addWorkoutViewModel_completedWorkout(viewModel, minutes);
    }
}

@end
