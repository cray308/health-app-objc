//
//  WorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#import "WorkoutViewController.h"
#import "AddWorkoutViewModel.h"
#import "Divider.h"
#import "AppDelegate.h"
#include <UserNotifications/UserNotifications.h>
#include <pthread.h>

#define _U_ __attribute__((__unused__))

@interface ExerciseView: UIView
- (id) initWithExercise: (ExerciseEntry *)exercise;
- (bool) handleTap;
- (void) reset;
@end

@interface ExerciseContainer: UIView
- (id) initWithGroup: (ExerciseGroup *)exerciseGroup;
- (void) startCircuitAndTimer: (bool)startTimer;
- (void) handleTap: (UIButton *)btn;
- (void) stopExerciseAtIndex: (int)index moveToNext: (bool)moveToNext;
- (void) finishGroupAtIndex: (int)index;
- (void) restartExerciseAtIndex: (int)index moveToNext: (bool)moveToNext refTime: (int64_t)refTime;
- (void) restartGroupAtIndex: (int)index refTime: (int64_t)refTime;
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
    @public WorkoutViewController *parent;
    ExerciseGroup *group;
    UILabel *headerLabel;
    ExerciseView **viewsArr;
    int currentIndex;
    int size;
}
@end

typedef enum {
    TimerTypeGroup,
    TimerTypeExercise
} TimerType;

typedef enum {
    WorkoutNotificationExerciseCompleted,
    WorkoutNotificationAMRAPCompleted
} WorkoutNotification;

typedef struct {
    WorkoutViewController *parent;
    const unsigned char type;
    unsigned char active;
    bool stop;
    int container;
    int exercise;
    unsigned int duration;
    int64_t refTime;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WorkoutTimer;

static pthread_t exerciseTimerThread;
static pthread_t groupTimerThread;
static WorkoutTimer *gPtr = NULL;
static WorkoutTimer *ePtr = NULL;
static pthread_mutex_t sharedLock;

void handle_exercise_timer_interrupt(int n __attribute__((__unused__))) {
}

void handle_group_timer_interrupt(int n __attribute__((__unused__))) {
}

void startTimerForGroup(WorkoutTimer *t, unsigned int duration, int container) {
    pthread_mutex_lock(&t->lock);
    t->refTime = CFAbsoluteTimeGetCurrent();
    t->duration = duration;
    t->container = container;
    t->stop = !duration;
    t->active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

void startTimerForExercise(WorkoutTimer *t, unsigned int duration, int exercise) {
    pthread_mutex_lock(&t->lock);
    t->refTime = CFAbsoluteTimeGetCurrent();
    t->duration = duration;
    t->exercise = exercise;
    t->stop = !duration;
    t->active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

void *timer_loop(void *arg) {
    WorkoutTimer *t = (WorkoutTimer *) arg;
    unsigned int res = 0, duration = 0;
    int container = -1, exercise = -1;
    while (!t->stop) {
        pthread_mutex_lock(&t->lock);
        while (t->active != 1) pthread_cond_wait(&t->cond, &t->lock);
        duration = t->duration;
        container = t->container;
        exercise = t->exercise;
        pthread_mutex_unlock(&t->lock);

        if (!duration) continue;
        res = sleep(duration);
        t->active = 2;
        if (!res) {
            if (t->type == TimerTypeGroup && ePtr->active == 1) {
                pthread_kill(exerciseTimerThread, SIGUSR1);
            }
            dispatch_async(dispatch_get_main_queue(), ^ (void) {
                [t->parent finishedWorkoutTimerForType:t->type container:container exercise:exercise];
            });
        }
    }
    return NULL;
}

void scheduleNotification(unsigned int secondsFromNow, unsigned char messageType) {
    static CFStringRef const messages[] = {CFSTR("Finished exercise!"), CFSTR("Finished AMRAP circuit!")};
    static CFStringRef const idFormat = CFSTR("%u");
    static unsigned int identifier = 0;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, idFormat, identifier++);

    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = @"Workout Update";
    content.subtitle = (__bridge NSString*) messages[messageType];
    content.sound = UNNotificationSound.defaultSound;

    UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:secondsFromNow
                                                                                                    repeats:false];
    UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier:(__bridge NSString*)idString
                                                                      content:content
                                                                      trigger:trigger];

    [UNUserNotificationCenter.currentNotificationCenter addNotificationRequest:req
                                                         withCompletionHandler:^(NSError *_Nullable error _U_) {}];
    CFRelease(idString);
    [content release];
}

@implementation ExerciseView
- (id) initWithExercise: (ExerciseEntry *)exerciseEntry {
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
    [self reset];
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
            startTimerForExercise(ePtr, exercise->reps, (int) button.tag);
            scheduleNotification(exercise->reps, WorkoutNotificationExerciseCompleted);
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
- (id) initWithGroup: (ExerciseGroup *)exerciseGroup {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    group = exerciseGroup;
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
        ExerciseView *v = [[ExerciseView alloc] initWithExercise:exerciseGroup_getExercise(group, i)];
        v->button.tag = i;
        [v->button addTarget:self action:@selector(handleTap:) forControlEvents:UIControlEventTouchUpInside];
        [exerciseStack addArrangedSubview:v];
        viewsArr[i] = v;
    }
    [exerciseStack release];
}

- (void) startCircuitAndTimer: (bool)startTimer {
    ePtr->container = (int) self.tag;
    currentIndex = 0;
    for (int i = 0; i < size; ++i) {
        [viewsArr[i] reset];
    }
    if (group->type == ExerciseContainerTypeAMRAP && startTimer) {
        unsigned int duration = 60 * group->reps;
        startTimerForGroup(gPtr, duration, (int) self.tag);
        scheduleNotification(duration, WorkoutNotificationAMRAPCompleted);
    }
    [viewsArr[0] handleTap];
}

- (void) restartExerciseAtIndex: (int)index moveToNext: (bool)moveToNext refTime: (int64_t)refTime {
    ExerciseView *v = nil;
    bool endExercise = false;
    pthread_mutex_lock(&sharedLock);

    if (group && index == currentIndex) {
        ExerciseView *temp = viewsArr[index];
        if (temp->exercise->type == ExerciseTypeDuration) {
            v = temp;
            unsigned int diff = (unsigned int)(refTime - ePtr->refTime);
            if (diff >= ePtr->duration) {
                endExercise = true;
            } else {
                unsigned int duration = ePtr->duration - diff;
                startTimerForExercise(ePtr, duration, (int) v->button.tag);
            }
        }
    }
    pthread_mutex_unlock(&sharedLock);

    if (v && endExercise) {
        [self stopExerciseAtIndex:index moveToNext:moveToNext];
    }
}

- (void) restartGroupAtIndex: (int)index refTime: (int64_t)refTime {
    WorkoutViewController *p = nil;
    bool endGroup = false;
    pthread_mutex_lock(&sharedLock);

    if (group && index == self.tag && group->type == ExerciseContainerTypeAMRAP) {
        p = parent;
        unsigned int diff = (unsigned int)(refTime - gPtr->refTime);
        if (diff >= gPtr->duration) {
            endGroup = true;
        } else {
            unsigned int duration = gPtr->duration - diff;
            startTimerForGroup(gPtr, duration, (int) self.tag);
        }
    }
    pthread_mutex_unlock(&sharedLock);

    if (p && endGroup) {
        [self finishGroupAtIndex:index];
    }
}

- (void) stopExerciseAtIndex: (int)index moveToNext: (bool)moveToNext {
    ExerciseView *v = nil;
    pthread_mutex_lock(&sharedLock);
    if (group && index == currentIndex) {
        ExerciseView *temp = viewsArr[index];
        if (temp->exercise->type == ExerciseTypeDuration) {
            v = temp;
        }
    }
    pthread_mutex_unlock(&sharedLock);
    if (v) {
        if (moveToNext) {
            [self handleTap:v->button];
        } else {
            v->button.userInteractionEnabled = true;
        }
    }
}

- (void) finishGroupAtIndex: (int)index {
    WorkoutViewController *p = nil;
    pthread_mutex_lock(&sharedLock);
    if (group && index == self.tag && group->type == ExerciseContainerTypeAMRAP) {
        p = parent;
        group = NULL;
    }
    pthread_mutex_unlock(&sharedLock);
    if (p) [p finishedExerciseGroup];
}

- (void) handleTap: (UIButton *)btn {
    bool isDone = false;
    pthread_mutex_lock(&sharedLock);
    if (group && (int) btn.tag == currentIndex) {
        ExerciseView *v = viewsArr[currentIndex];
        if ([v handleTap]) {
            if (++currentIndex == size) {
                switch (group->type) {
                    case ExerciseContainerTypeRounds:
                        if (++group->completedReps == group->reps) {
                            isDone = true;
                            group = NULL;
                        } else {
                            CFStringRef headerStr = exerciseGroup_createHeaderText(group);
                            headerLabel.text = (__bridge NSString*) headerStr;
                            CFRelease(headerStr);
                        }
                        break;

                    case ExerciseContainerTypeDecrement:
                        if (--group->completedReps == 0) {
                            isDone = true;
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
                    [self startCircuitAndTimer:0];
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
    int workoutType;
    WorkoutTimer timers[2];
    struct savedInfo {
        int groupTag;
        struct exerciseInfo {
            int group;
            int tag;
        } exerciseInfo;
    } savedInfo;
}
@end

@implementation WorkoutViewController
- (id) initWithViewModel: (AddWorkoutViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    workoutType = viewModel->workout->type;

    struct sigaction sa;
    sa.sa_handler = handle_exercise_timer_interrupt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SIGUSR2, &sa, NULL);

    WorkoutTimer groupTimer = { .type = TimerTypeGroup, .parent = self };
    WorkoutTimer exerciseTimer = { .type = TimerTypeExercise, .parent = self };
    memcpy(&timers[TimerTypeGroup], &groupTimer, sizeof(WorkoutTimer));
    memcpy(&timers[TimerTypeExercise], &exerciseTimer, sizeof(WorkoutTimer));
    for (int i = 0; i < 2; ++i) {
        pthread_mutex_init(&timers[i].lock, NULL);
        pthread_cond_init(&timers[i].cond, NULL);
    }
    gPtr = &timers[TimerTypeGroup];
    ePtr = &timers[TimerTypeExercise];
    pthread_mutex_init(&sharedLock, NULL);

    pthread_create(&exerciseTimerThread, NULL, timer_loop, &timers[TimerTypeExercise]);
    pthread_create(&groupTimerThread, NULL, timer_loop, &timers[TimerTypeGroup]);
    AppDelegate *delegate = (AppDelegate *) UIApplication.sharedApplication.delegate;
    if (delegate) delegate->workoutVC = self;
    return self;
}

- (void) dealloc {
    AppDelegate *delegate = (AppDelegate *) UIApplication.sharedApplication.delegate;
    if (delegate) delegate->workoutVC = nil;
    if (timers[TimerTypeGroup].active == 1) pthread_kill(groupTimerThread, SIGUSR2);
    if (timers[TimerTypeExercise].active == 1) pthread_kill(exerciseTimerThread, SIGUSR1);
    startTimerForGroup(&timers[TimerTypeGroup], 0, 0);
    startTimerForExercise(&timers[TimerTypeExercise], 0, 0);
    pthread_join(exerciseTimerThread, NULL);
    pthread_join(groupTimerThread, NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&timers[i].cond);
        pthread_mutex_destroy(&timers[i].lock);
    }
    gPtr = ePtr = NULL;
    pthread_mutex_destroy(&sharedLock);
    [UNUserNotificationCenter.currentNotificationCenter removeAllPendingNotificationRequests];
    [UNUserNotificationCenter.currentNotificationCenter removeAllDeliveredNotifications];
    [groupsStack release];
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
        ExerciseContainer *v = [[ExerciseContainer alloc] initWithGroup:workout_getExerciseGroup(w, i)];
        v.tag = i;
        v->parent = self;
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
        [v startCircuitAndTimer:1];
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
        NSArray<UIView *> *views = groupsStack.arrangedSubviews;
        if (views.count >= 3) {
            ExerciseContainer *next = (ExerciseContainer *) views[2];
            [views[0] removeFromSuperview];
            [views[1] removeFromSuperview];
            [next startCircuitAndTimer:1];
        } else {
            m = viewModel;
            m->stopTime = CFAbsoluteTimeGetCurrent();
            viewModel = NULL;
        }
    }
    pthread_mutex_unlock(&sharedLock);

    if (m) {
        addWorkoutViewModel_completedWorkout(m, nil, true);
    }
}

- (void) finishedWorkoutTimerForType: (unsigned char)type container: (int)container exercise: (int)exercise {
    ExerciseContainer *v = nil;
    pthread_mutex_lock(&sharedLock);
    if (viewModel) {
        NSArray<UIView *> *views = groupsStack.arrangedSubviews;
        v = (ExerciseContainer *) views[0];
    }
    pthread_mutex_unlock(&sharedLock);

    if (v) {
        switch (type) {
            case TimerTypeGroup:
                [v finishGroupAtIndex:container];
                break;
            default:
                [v stopExerciseAtIndex:exercise moveToNext:workoutType != WorkoutTypeEndurance];
                break;
        }
    }
}

- (void) stopTimers {
    pthread_mutex_lock(&sharedLock);
    if (timers[TimerTypeGroup].active == 1) {
        savedInfo.groupTag = timers[TimerTypeGroup].container;
        pthread_kill(groupTimerThread, SIGUSR2);
    } else {
        savedInfo.groupTag = -1;
    }

    if (timers[TimerTypeExercise].active == 1) {
        savedInfo.exerciseInfo.group = timers[TimerTypeExercise].container;
        savedInfo.exerciseInfo.tag = timers[TimerTypeExercise].exercise;
        pthread_kill(exerciseTimerThread, SIGUSR1);
    } else {
        savedInfo.exerciseInfo.group = savedInfo.exerciseInfo.tag = -1;
    }
    pthread_mutex_unlock(&sharedLock);
}

- (void) restartTimers {
    ExerciseContainer *v = nil;
    bool eTimerActive = false, gTimerActive = false;
    pthread_mutex_lock(&sharedLock);
    if (viewModel) {
        gTimerActive = savedInfo.groupTag >= 0;
        eTimerActive = savedInfo.exerciseInfo.group >= 0;
        v = (ExerciseContainer *) groupsStack.arrangedSubviews[0];
    }
    pthread_mutex_unlock(&sharedLock);

    if (v) {
        int64_t now = CFAbsoluteTimeGetCurrent();
        if (eTimerActive && v.tag == savedInfo.exerciseInfo.group) {
            [v restartExerciseAtIndex:savedInfo.exerciseInfo.tag moveToNext:workoutType != WorkoutTypeEndurance refTime:now];
        }
        if (gTimerActive) {
            [v restartGroupAtIndex:savedInfo.groupTag refTime:now];
        }
    }
}
@end
