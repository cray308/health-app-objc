//
//  WorkoutViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#import "WorkoutViewController.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"
#import <UserNotifications/UserNotifications.h>
#import <NotificationCenter/NotificationCenter.h>
#import "ExerciseContainer.h"

static pthread_t exerciseTimerThread;
static pthread_t groupTimerThread;

static void handle_exercise_timer_interrupt(int n _U_) {}
static void handle_group_timer_interrupt(int n _U_) {}

@interface WorkoutViewController() {
    AddWorkoutCoordinator *delegate;
    AddWorkoutViewModel *viewModel;
    UIStackView *groupsStack;
    bool notEndurance;
    NSObject *startObserver;
    NSObject *stopObserver;
    @public WorkoutTimer timers[2];
    struct savedInfo {
        int groupTag;
        struct exerciseInfo {
            int group;
            int tag;
        } exerciseInfo;
    } savedInfo;
}
@end

void *timer_loop(void *arg) {
    WorkoutTimer *t = (WorkoutTimer *) arg;
    int res = 0, duration = 0, container = -1, exercise = -1;
    while (!t->info.stop) {
        pthread_mutex_lock(&t->lock);
        while (t->info.active != 1) pthread_cond_wait(&t->cond, &t->lock);
        duration = t->duration;
        container = t->container;
        exercise = t->exercise;
        pthread_mutex_unlock(&t->lock);

        if (!duration) continue;
        res = sleep(duration);
        t->info.active = 2;
        if (!res) {
            WorkoutViewController *parent = t->parent;
            if (t->info.type == 0 && parent->timers[1].info.active == 1)
                pthread_kill(exerciseTimerThread, SIGUSR1);
            dispatch_async(dispatch_get_main_queue(), ^ (void) {
                [parent finishedWorkoutTimerForType:t->info.type
                                          container:container exercise:exercise];
            });
        }
    }
    return NULL;
}

@implementation WorkoutViewController
- (id) initWithDelegate: (AddWorkoutCoordinator *)_delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    delegate = _delegate;
    viewModel = &delegate->viewModel;
    notEndurance = viewModel->workout->type != WorkoutTypeEndurance;

    struct sigaction sa;
    sa.sa_handler = handle_exercise_timer_interrupt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SIGUSR2, &sa, NULL);

    WorkoutTimer groupTimer = { .info = { .type = TimerTypeGroup }, .parent = self };
    WorkoutTimer exerciseTimer = { .info = { .type = TimerTypeExercise }, .parent = self };
    memcpy(&timers[TimerTypeGroup], &groupTimer, sizeof(WorkoutTimer));
    memcpy(&timers[TimerTypeExercise], &exerciseTimer, sizeof(WorkoutTimer));
    for (int i = 0; i < 2; ++i) {
        pthread_mutex_init(&timers[i].lock, NULL);
        pthread_cond_init(&timers[i].cond, NULL);
    }
    pthread_mutex_init(&timerLock, NULL);
    pthread_create(&exerciseTimerThread, NULL, timer_loop, &timers[TimerTypeExercise]);
    pthread_create(&groupTimerThread, NULL, timer_loop, &timers[TimerTypeGroup]);
    return self;
}

- (void) dealloc {
    if (timers[TimerTypeGroup].info.active == 1) pthread_kill(groupTimerThread, SIGUSR2);
    if (timers[TimerTypeExercise].info.active == 1) pthread_kill(exerciseTimerThread, SIGUSR1);
    startWorkoutTimer(&timers[TimerTypeGroup], 0, 0, -1);
    startWorkoutTimer(&timers[TimerTypeExercise], 0, -1, 0);
    pthread_join(exerciseTimerThread, NULL);
    pthread_join(groupTimerThread, NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&timers[i].cond);
        pthread_mutex_destroy(&timers[i].lock);
    }
    pthread_mutex_destroy(&timerLock);
    [UNUserNotificationCenter.currentNotificationCenter removeAllPendingNotificationRequests];
    [UNUserNotificationCenter.currentNotificationCenter removeAllDeliveredNotifications];
    [NSNotificationCenter.defaultCenter removeObserver:startObserver];
    [NSNotificationCenter.defaultCenter removeObserver:stopObserver];
    [groupsStack release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = (__bridge NSString*) viewModel->workout->title;

    groupsStack = createStackView(NULL, 0, 1, 20, 0, (HAEdgeInsets){0, 4, 4, 0});
    UIButton *startBtn = createButton(CFSTR("Start"), UIColor.systemGreenColor, nil,
                                      UIFontTextStyleSubheadline,
                                      UIColor.secondarySystemGroupedBackgroundColor, true, true, 0,
                                      self, @selector(startEndWorkout:));
    UIView *btnContainer = createView(nil, false);
    [btnContainer addSubview:startBtn];

    Workout *w = viewModel->workout;
    for (int i = 0; i < (int) w->activities->size; ++i) {
        if (i > 0) {
            UIView *d = createDivider();
            [groupsStack addArrangedSubview:d];
            [d release];
        }
        ExerciseContainer *v = [[ExerciseContainer alloc] initWithGroup:&w->activities->arr[i] tag:i
                                                                 parent:self timer:&timers[0]
                                                          exerciseTimer:&timers[1]];
        [groupsStack addArrangedSubview:v];
        [v release];
    }

    UIStackView *vStack = createStackView((id[]){btnContainer, groupsStack}, 2, 1, 20, 0,
                                          (HAEdgeInsets){10, 0, 0, 0});
    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [scrollView.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:guide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:guide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [startBtn.topAnchor constraintEqualToAnchor:btnContainer.topAnchor],
        [startBtn.bottomAnchor constraintEqualToAnchor:btnContainer.bottomAnchor],
        [startBtn.leadingAnchor constraintEqualToAnchor:btnContainer.leadingAnchor],
        [startBtn.widthAnchor constraintEqualToAnchor:btnContainer.widthAnchor multiplier:0.4],
        [startBtn.heightAnchor constraintEqualToConstant: 30]
    }, 14);

    startObserver = [NSNotificationCenter.defaultCenter
                     addObserverForName:UIApplicationDidBecomeActiveNotification object:nil
                     queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note _U_) {
        [self restartTimers];
    }];
    stopObserver = [NSNotificationCenter.defaultCenter
                    addObserverForName:UIApplicationWillResignActiveNotification object:nil
                    queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note _U_) {
        [self stopTimers];
    }];
    [btnContainer release];
    [vStack release];
    [scrollView release];
}

- (void) startEndWorkout: (UIButton *)btn {
    if (!btn.tag) { // tapped start
        setButtonTitle(btn, CFSTR("End"), 0);
        setButtonColor(btn, UIColor.systemRedColor, 0);
        ExerciseContainer *v =  (ExerciseContainer *) groupsStack.arrangedSubviews[0];
        setTag(btn, 1);
        viewModel->startTime = time(NULL);
        [v startCircuitAndTimer:1];
    } else {
        AddWorkoutViewModel *m = NULL;
        pthread_mutex_lock(&timerLock);
        if (viewModel) {
            m = viewModel;
            m->stopTime = time(NULL) + 1;
            viewModel = NULL;
        }
        pthread_mutex_unlock(&timerLock);
        if (m)
            addWorkoutCoordinator_stoppedWorkout(delegate);
    }
}

- (void) finishedExerciseGroup {
    AddWorkoutViewModel *m = NULL;
    pthread_mutex_lock(&timerLock);
    if (viewModel) {
        CFArrayRef views = (__bridge CFArrayRef) groupsStack.arrangedSubviews;
        if (CFArrayGetCount(views) >= 3) {
            UIView *first = CFArrayGetValueAtIndex(views, 0);
            UIView *second = CFArrayGetValueAtIndex(views, 1);
            ExerciseContainer *next = CFArrayGetValueAtIndex(views, 2);
            [first removeFromSuperview];
            [second removeFromSuperview];
            [next startCircuitAndTimer:1];
        } else {
            m = viewModel;
            m->stopTime = time(NULL) + 1;
            viewModel = NULL;
        }
    }
    pthread_mutex_unlock(&timerLock);

    if (m)
        addWorkoutCoordinator_completedWorkout(delegate, false, true);
}

- (void) finishedWorkoutTimerForType: (unsigned char)type
                           container: (int)container exercise: (int)exercise {
    ExerciseContainer *v = nil;
    pthread_mutex_lock(&timerLock);
    if (viewModel) {
        CFArrayRef views = (__bridge CFArrayRef) groupsStack.arrangedSubviews;
        v = CFArrayGetValueAtIndex(views, 0);
    }
    pthread_mutex_unlock(&timerLock);

    if (v) {
        if (!type)
            [v finishGroupAtIndex:container];
        else
            [v stopExerciseAtIndex:exercise moveToNext:notEndurance];
    }
}

- (void) stopTimers {
    pthread_mutex_lock(&timerLock);
    if (timers[0].info.active == 1) {
        savedInfo.groupTag = timers[0].container;
        pthread_kill(groupTimerThread, SIGUSR2);
    } else {
        savedInfo.groupTag = -1;
    }

    if (timers[1].info.active == 1) {
        savedInfo.exerciseInfo.group = timers[1].container;
        savedInfo.exerciseInfo.tag = timers[1].exercise;
        pthread_kill(exerciseTimerThread, SIGUSR1);
    } else {
        savedInfo.exerciseInfo.group = savedInfo.exerciseInfo.tag = -1;
    }
    pthread_mutex_unlock(&timerLock);
}

- (void) restartTimers {
    ExerciseContainer *v = nil;
    bool eTimerActive = false, gTimerActive = false;
    pthread_mutex_lock(&timerLock);
    if (viewModel) {
        gTimerActive = savedInfo.groupTag >= 0;
        eTimerActive = savedInfo.exerciseInfo.group >= 0;
        v = (ExerciseContainer *) groupsStack.arrangedSubviews[0];
    }
    pthread_mutex_unlock(&timerLock);

    if (v) {
        time_t now = time(NULL);
        if (eTimerActive && v.tag == savedInfo.exerciseInfo.group)
            [v restartExerciseAtIndex:savedInfo.exerciseInfo.tag
                           moveToNext:notEndurance refTime:now];
        if (gTimerActive)
            [v restartGroupAtIndex:savedInfo.groupTag refTime:now];
    }
}
@end
