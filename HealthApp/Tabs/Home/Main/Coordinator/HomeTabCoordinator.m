//
//  HomeTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeTabCoordinator.h"
#import "AppCoordinator.h"
#import "AddWorkoutCoordinator.h"
#import "HomeViewModel.h"
#import "HomeViewController.h"
#import "HomeBarChartViewModel.h"
#import "HomePieChartViewModel.h"
#import "ActivityType.h"
#import "CalendarDateHelpers.h"
#import "AppUserData.h"

#define UpdateGreeting 0x1
#define UpdateCharts 0x2

HomeViewModel *homeViewModel_init(HomeTabCoordinator *delegate);
void homeViewModel_free(HomeViewModel *model);
void homeCoordinator_updateUI(HomeTabCoordinator *coordinator, unsigned char updates);

static inline HomeViewController *getHomeViewController(UINavigationController *controller) {
    return (HomeViewController *) controller.viewControllers[0];
}

HomeTabCoordinator *homeCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate) {
    HomeTabCoordinator *coordinator = calloc(1, sizeof(HomeTabCoordinator));
    if (!coordinator) return NULL;
    if (!(coordinator->viewModel = homeViewModel_init(coordinator))) {
        free(coordinator);
        return NULL;
    }
    coordinator->delegate = delegate;
    coordinator->navigationController = navVC;
    return coordinator;
}

void homeCoordinator_free(HomeTabCoordinator *coordinator) {
    if (coordinator->childCoordinator) addWorkoutCoordinator_free(coordinator->childCoordinator);
    homeViewModel_free(coordinator->viewModel);
    free(coordinator);
}

void homeCoordinator_start(HomeTabCoordinator *coordinator) {
    HomeViewController *vc = [[HomeViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

void homeCoordinator_updateNavBarTokens(HomeTabCoordinator *coordinator, NSString *label) {
    HomeViewController *vc = getHomeViewController(coordinator->navigationController);
    [vc updateNavBarCoins:label];
}

void homeCoordinator_navigateToAddWorkouts(HomeTabCoordinator *coordinator) {
    AddWorkoutCoordinator *child = addWorkoutCoordinator_init(coordinator->navigationController, coordinator);
    if (!child) return;
    coordinator->childCoordinator = child;
    addWorkoutCoordinator_start(child);
}

void homeCoordinator_didFinishAddingWorkouts(HomeTabCoordinator *coordinator, int newTokens, int *durations) {
    int tokenGoal = 6; // change to number of workouts this week, only check if it's not 0
    int currentTokens = coordinator->viewModel->barChartViewModel->totalTokens;
    HomeViewController *homeVC = getHomeViewController(coordinator->navigationController);
    bool showConfetti = currentTokens < tokenGoal && currentTokens + newTokens >= tokenGoal;

    if (newTokens) {
        int result = homeViewModel_addNewTokens(coordinator->viewModel, newTokens);
        appCoordinator_updateNavBarTokens(coordinator->delegate, result);
        [homeVC runNavBarAnimation];
    }

    homeViewModel_addIntensityDurations(coordinator->viewModel, durations);
    [homeVC updateCharts];
    addWorkoutCoordinator_free(coordinator->childCoordinator);
    coordinator->childCoordinator = NULL;
    [coordinator->navigationController popViewControllerAnimated:true];

    if (showConfetti) {
        dispatch_time_t endTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t) NSEC_PER_SEC * 0.75);
        dispatch_after(endTime, dispatch_get_main_queue(), ^ (void) {
            [homeVC showConfetti];
        });
    }
}

void homeCoordinator_performForegroundUpdate(HomeTabCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator, UpdateGreeting);
}

void homeCoordinator_updateForNewWeek(HomeTabCoordinator *coordinator) {
    homeViewModel_clear(coordinator->viewModel);
    homeCoordinator_updateUI(coordinator, UpdateCharts | UpdateGreeting);
}

void homeCoordinator_updateForNewDay(HomeTabCoordinator *coordinator, int mostRecentDay, int currentDay) {
    homeViewModel_updateForNewDay(coordinator->viewModel, mostRecentDay, currentDay);
    homeCoordinator_updateUI(coordinator, UpdateCharts | UpdateGreeting);
}

void homeCoordinator_handleUserInfoChange(HomeTabCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator, UpdateCharts | UpdateGreeting);
}

void homeCoordinator_handleDataDeletion(HomeTabCoordinator *coordinator) {
    homeViewModel_clear(coordinator->viewModel);
    homeCoordinator_updateUI(coordinator, UpdateCharts);
}

#pragma mark - Helpers

void homeCoordinator_updateUI(HomeTabCoordinator *coordinator, unsigned char updates) {
    HomeViewController *homeVC = getHomeViewController(coordinator->navigationController);
    if (updates & UpdateGreeting) {
        [homeVC updateGreeting];
    }
    if (updates & UpdateCharts) {
        [homeVC updateCharts];
    }
}



HomeViewModel *homeViewModel_init(HomeTabCoordinator *delegate) {
    HomeViewModel *model = malloc(sizeof(HomeViewModel));
    if (!model) return NULL;
    model->barChartViewModel = calloc(1, sizeof(HomeBarChartViewModel));
    model->pieChartViewModel = calloc(1, sizeof(HomePieChartViewModel));

    if (!(model->barChartViewModel && model->pieChartViewModel)) {
        if (model->barChartViewModel) free(model->barChartViewModel);
        if (model->pieChartViewModel) free(model->pieChartViewModel);
        free(model);
        return NULL;
    }

    {
        HomeBarChartViewModel *viewModel = model->barChartViewModel;
        for (int i = 0; i < 7; ++i) {
            viewModel->cumulativeArray[i] = [[BarChartDataEntry alloc] initWithX:i y:0];
        }
    }
    {
        HomePieChartViewModel *viewModel = model->pieChartViewModel;
        for (int i = 0; i < 3; ++i) {
            NSString *prefix = activityType_getString(i, true);
            viewModel->entries[i] = [[PieChartDataEntry alloc] initWithValue:0 label:prefix];
            viewModel->legendLabelFormats[i] = [[NSString alloc] initWithFormat:@"%@ (%%@)", prefix];
            [prefix release];
        }
    }

    model->delegate = delegate;
    model->currentTokenTextFormat = @"Tokens this week: %d";
    homeViewModel_updateTimeOfDay(model);
    return model;
}

void homeViewModel_free(HomeViewModel *model) {
    {
        HomeBarChartViewModel *viewModel = model->barChartViewModel;
        for (int i = 0; i < 7; ++i) {
            [viewModel->cumulativeArray[i] release];
        }
    }
    {
        HomePieChartViewModel *viewModel = model->pieChartViewModel;
        for (int i = 0; i < 3; ++i) {
            [viewModel->entries[i] release];
            [viewModel->legendLabelFormats[i] release];
        }
    }
    free(model);
}
