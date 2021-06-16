//
//  WorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#import "WorkoutViewController.h"
#import "AddWorkoutViewModel.h"
#import "Divider.h"
#include <pthread.h>

@interface ExerciseView: UIView

- (id) initWithExercise: (ExerciseEntry *)exercise tag: (int)tag target: (id)target action: (SEL)action;
- (bool) handleTap;
- (void) reset;

@end

@interface ExerciseContainer: UIView

- (id) initWithGroup: (ExerciseGroup *)exerciseGroup parent: (WorkoutViewController *)parentVC;
- (void) startCircuit;
- (void) handleTap: (UIButton *)btn;
- (void) stopExercise;
- (void) finishGroup;

@end

@interface ExerciseView() {
    @public ExerciseEntry *exercise;
    int completedSets;
    bool resting;
    NSString *restStr;
    UILabel *setsLabel;
    UIView *checkbox;
    @public UIButton *button;
}

@end

@interface ExerciseContainer() {
    WorkoutViewController *parent;
    @public ExerciseGroup *group;
    UILabel *headerLabel;
    @public ExerciseView **viewsArr;
    @public int currentIndex;
    @public int size;
}

@end

typedef enum {
    TimerTypeGroup,
    TimerTypeExercise
} TimerType;

typedef struct {
    WorkoutViewController *parent;
    const unsigned char type;
    unsigned char active;
    unsigned char stop;
    unsigned int duration;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WorkoutTimer;

static pthread_t exerciseTimerThread;
static pthread_t groupTimerThread;

static WorkoutTimer groupTimer = { .type = TimerTypeGroup };
static WorkoutTimer exerciseTimer = { .type = TimerTypeExercise };

static WorkoutTimer *gPtr = &groupTimer;
static WorkoutTimer *ePtr = &exerciseTimer;

static pthread_mutex_t sharedLock;

void handle_exercise_timer_interrupt(int n __attribute__((__unused__))) {
    ePtr->active = 0;
}

void handle_group_timer_interrupt(int n __attribute__((__unused__))) {
    gPtr->active = 0;
}

void timer_start(WorkoutTimer *t, unsigned int duration) {
    pthread_mutex_lock(&t->lock);
    t->duration = duration;
    t->stop = !duration;
    t->active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

void *timer_loop(void *arg) {
    WorkoutTimer *t = (WorkoutTimer *) arg;
    unsigned int res = 0;
    while (!t->stop) {
        pthread_mutex_lock(&t->lock);
        while (!t->active) pthread_cond_wait(&t->cond, &t->lock);
        unsigned int duration = t->duration;
        pthread_mutex_unlock(&t->lock);

        if (!duration) continue;

        res = sleep(duration);
        t->active = 0;
        if (!res) {
            if (t->type == TimerTypeGroup && exerciseTimer.active) {
                pthread_kill(exerciseTimerThread, SIGUSR1);
            }
            dispatch_async(dispatch_get_main_queue(), ^ (void) {
                [t->parent finishedWorkoutTimerForType:t->type];
            });
        }
    }
    return NULL;
}

@implementation ExerciseView

- (id) initWithExercise: (ExerciseEntry *)exerciseEntry tag: (int)tag target: (id)target action: (SEL)action {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    exercise = exerciseEntry;
    if (exerciseEntry->rest) {
        restStr = [[NSString alloc] initWithFormat:@"Rest: %u s", exerciseEntry->rest];
    }

    button = [UIButton buttonWithType:UIButtonTypeSystem];
    button.translatesAutoresizingMaskIntoConstraints = false;
    [button setTitle:@"" forState:UIControlStateNormal];
    [button setTitleColor:UIColor.labelColor forState: UIControlStateNormal];
    [button setTitleColor:UIColor.secondaryLabelColor forState:UIControlStateDisabled];
    button.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    button.titleLabel.adjustsFontSizeToFitWidth = true;
    button.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    button.layer.cornerRadius = 5;
    button.tag = tag;
    [button addTarget:target action:action forControlEvents:UIControlEventTouchUpInside];

    setsLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    setsLabel.translatesAutoresizingMaskIntoConstraints = false;
    setsLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleSubheadline];
    setsLabel.adjustsFontSizeToFitWidth = true;
    CFStringRef setsStr = exercise_createSetsString(exercise);
    if (setsStr) {
        setsLabel.text = (__bridge NSString*) setsStr;
        CFRelease(setsStr);
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
    return self;
}

- (void) dealloc {
    [setsLabel release];
    [checkbox release];
    if (restStr) [restStr release];
    [super dealloc];
}

- (bool) handleTap {
    if (!button.enabled) {
        [button setEnabled:true];
        checkbox.backgroundColor = UIColor.systemOrangeColor;
        if (exercise->type == ExerciseTypeDuration) { // start timer
            button.userInteractionEnabled = false;
            timer_start(ePtr, exercise->reps);

        }
    } else {
        button.userInteractionEnabled = true;

        if (restStr && !resting) {
            resting = true;
            [button setTitle:restStr forState:UIControlStateNormal];
        } else {
            resting = false;
            if (++exercise->completedSets == exercise->sets) {
                [button setEnabled:false];
                checkbox.backgroundColor = UIColor.systemGreenColor;
                return true;
            }

            CFStringRef setsStr = exercise_createSetsString(exercise);
            CFStringRef title = exercise_createTitleString(exercise);
            setsLabel.text = (__bridge NSString*) setsStr;
            [button setTitle:(__bridge NSString*)title forState:UIControlStateNormal];
            CFRelease(setsStr);
            CFRelease(title);
        }
    }
    return false;
}

- (void) reset {
    checkbox.backgroundColor = UIColor.systemGrayColor;
    [button setEnabled:false];
    resting = false;
    exercise->completedSets = 0;
    CFStringRef title = exercise_createTitleString(exercise);
    [button setTitle:(__bridge NSString*)title forState:UIControlStateNormal];
    CFRelease(title);
}

@end

@implementation ExerciseContainer

- (id) initWithGroup: (ExerciseGroup *)exerciseGroup parent: (WorkoutViewController *)parentVC {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    group = exerciseGroup;
    parent = parentVC;
    size = exerciseGroup_getNumberOfExercises(group);
    viewsArr = calloc(size, sizeof(ExerciseView *));
    [self setupSubviews];
    return self;
}

- (void) dealloc {
    for (int i = 0; i < size; ++i) { [viewsArr[i] release]; }
    free(viewsArr);
    [headerLabel release];
    [super dealloc];
}

- (void) setupSubviews {
    headerLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    headerLabel.translatesAutoresizingMaskIntoConstraints = false;
    headerLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle3];
    headerLabel.adjustsFontSizeToFitWidth = true;
    CFStringRef headerStr = exerciseGroup_createHeaderText(group);
    if (headerStr) {
        headerLabel.text = (__bridge NSString*) headerStr;
        CFRelease(headerStr);
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
        [headerLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [headerLabel.heightAnchor constraintEqualToConstant: 20],

        [exerciseStack.topAnchor constraintEqualToAnchor:headerLabel.bottomAnchor],
        [exerciseStack.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [exerciseStack.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [exerciseStack.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
    ]];

    for (int i = 0; i < size; ++i) {
        ExerciseView *v = [[ExerciseView alloc] initWithExercise:exerciseGroup_getExercise(group, i) tag:i target:self action:@selector(handleTap:)];
        [v reset];
        [exerciseStack addArrangedSubview:v];
        viewsArr[i] = v;
    }
    [exerciseStack release];
}

- (void) startCircuit {
    currentIndex = 0;
    for (int i = 0; i < size; ++i) {
        [viewsArr[i] reset];
    }
    if (group->type == ExerciseContainerTypeAMRAP && !groupTimer.active) {
        timer_start(gPtr, 60 * group->reps);
    }
    [viewsArr[0] handleTap];
}

- (void) stopExercise {
    if (currentIndex >= size) return;
    ExerciseView *v = viewsArr[currentIndex];
    if (v->exercise->type == ExerciseTypeDuration) {
        [self handleTap:v->button];
    }
}

- (void) finishGroup {
    pthread_mutex_lock(&sharedLock);
    group = NULL;
    pthread_mutex_unlock(&sharedLock);
    [parent finishedExerciseGroup];
}

- (void) handleTap: (UIButton *)btn {
    unsigned char isDone = 0;
    pthread_mutex_lock(&sharedLock);
    if (group && (int) btn.tag == currentIndex) {
        ExerciseView *v = viewsArr[currentIndex];
        if ([v handleTap]) {
            if (++currentIndex == size) {
                switch (group->type) {
                    case ExerciseContainerTypeRounds:
                        if (++group->completedReps == group->reps) {
                            isDone = 1;
                            group = NULL;
                        } else {
                            CFStringRef headerStr = exerciseGroup_createHeaderText(group);
                            headerLabel.text = (__bridge NSString*) headerStr;
                            CFRelease(headerStr);
                        }
                        break;

                    case ExerciseContainerTypeDecrement:
                        if (--group->completedReps == 0) {
                            isDone = 1;
                            group = NULL;
                        } else {
                            for (int i = 0; i < size; ++i) {
                                ExerciseEntry *e = exerciseGroup_getExercise(group, i);
                                if (e->type == ExerciseTypeReps) e->reps -= 1;
                            }
                        }
                        break;

                    default:
                        break;
                }
                if (group) {
                    [self startCircuit];
                }
            } else {
                [viewsArr[currentIndex] handleTap];
            }
        }
    }
    pthread_mutex_unlock(&sharedLock);
    if (isDone) {
        [parent finishedExerciseGroup];
    }
}

@end

@interface WorkoutViewController() {
    AddWorkoutViewModel *viewModel;
    UIStackView *groupsStack;
    int currentIndex;
    int size;
}

@end

@implementation WorkoutViewController

- (id) initWithViewModel: (AddWorkoutViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    size = (int) workout_getNumberOfActivities(viewModel->workout);
    groupTimer.parent = exerciseTimer.parent = self;
    groupTimer.stop = exerciseTimer.stop = 0;

    struct sigaction sa;
    sa.sa_handler = handle_exercise_timer_interrupt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SIGUSR2, &sa, NULL);

    pthread_mutex_init(&exerciseTimer.lock, NULL);
    pthread_mutex_init(&groupTimer.lock, NULL);
    pthread_cond_init(&exerciseTimer.cond, NULL);
    pthread_cond_init(&groupTimer.cond, NULL);
    pthread_mutex_init(&sharedLock, NULL);

    pthread_create(&exerciseTimerThread, NULL, timer_loop, &exerciseTimer);
    pthread_create(&groupTimerThread, NULL, timer_loop, &groupTimer);
    return self;
}

- (void) dealloc {
    [groupsStack release];
    if (exerciseTimer.active) pthread_kill(exerciseTimerThread, SIGUSR1);
    if (groupTimer.active) pthread_kill(groupTimerThread, SIGUSR2);
    timer_start(gPtr, 0);
    timer_start(ePtr, 0);
    pthread_join(exerciseTimerThread, NULL);
    pthread_join(groupTimerThread, NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    pthread_cond_destroy(&groupTimer.cond);
    pthread_cond_destroy(&exerciseTimer.cond);
    pthread_mutex_destroy(&groupTimer.lock);
    pthread_mutex_destroy(&exerciseTimer.lock);
    pthread_mutex_destroy(&sharedLock);
    groupTimer.parent = exerciseTimer.parent = nil;
    groupTimer.active = exerciseTimer.active = 0;
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;
    self.navigationItem.title = (__bridge NSString*) viewModel->workout->title;
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

    Workout *w = viewModel->workout;
    for (int i = 0; i < workout_getNumberOfActivities(w); ++i) {
        if (i > 0) {
            Divider *d = [[Divider alloc] init];
            [groupsStack addArrangedSubview:d];
            [d release];
        }
        ExerciseContainer *v = [[ExerciseContainer alloc] initWithGroup:workout_getExerciseGroup(w, i) parent:self];
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
        viewModel->startTime = CFAbsoluteTimeGetCurrent();
        [v startCircuit];
    } else {
        AddWorkoutViewModel *m = NULL;
        pthread_mutex_lock(&sharedLock);
        if (viewModel) {
            m = viewModel;
            m->stopTime = CFAbsoluteTimeGetCurrent();
            viewModel = NULL;
        }
        pthread_mutex_unlock(&sharedLock);
        if (m) {
            addWorkoutViewModel_stoppedWorkout(m);
        }
    }
}

- (void) finishedExerciseGroup {
    AddWorkoutViewModel *m = NULL;
    pthread_mutex_lock(&sharedLock);
    if (viewModel) {
        if (size - currentIndex <= 1) {
            m = viewModel;
            m->stopTime = CFAbsoluteTimeGetCurrent();
            currentIndex = size;
            viewModel = NULL;
        } else {
            NSArray<UIView *> *views = groupsStack.arrangedSubviews;
            [views[currentIndex] setHidden:true];
            [views[currentIndex + 1] setHidden:true];
            ExerciseContainer *next = (ExerciseContainer *) views[currentIndex + 2];
            currentIndex += 2;
            [next startCircuit];
        }
    }
    pthread_mutex_unlock(&sharedLock);

    if (m) {
        addWorkoutViewModel_completedWorkout(m, nil, true);
    }
}

- (void) finishedWorkoutTimerForType: (unsigned char)type {
    ExerciseContainer *v = nil;
    UIButton *b = nil;
    pthread_mutex_lock(&sharedLock);

    if (viewModel) {
        NSArray<UIView *> *views = groupsStack.arrangedSubviews;
        v = (ExerciseContainer *) views[currentIndex];

        if (type == TimerTypeGroup) {
            if (v->group->type != ExerciseContainerTypeAMRAP) {
                v = nil;
            }
        } else {
            if (v->currentIndex < v->size) {
                ExerciseView *temp = v->viewsArr[v->currentIndex];
                if (temp->exercise->type == ExerciseTypeDuration) {
                    b = temp->button;
                }
            }
        }
    }
    pthread_mutex_unlock(&sharedLock);

    if (v) {
        if (type == TimerTypeGroup) {
            [v finishGroup];
        } else if (b) {
            [v handleTap:b];
        }
    }
}

@end
