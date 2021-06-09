//
//  HomeViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewModel.h"
#import "HomeTabCoordinator.h"
#import "AppCoordinator.h"
#import "AppUserData.h"
#import "PersistenceService.h"
#import "HomeBarChartViewModel.h"
#import "HomePieChartViewModel.h"
#import "CalendarDateHelpers.h"
#import "ActivityEntry+CoreDataClass.h"
#import "ViewControllerHelpers.h"

Array_SingleActivityModel *fetchWeeklyActivity(CFCalendarRef calendar);
NSFetchRequest *setupHomeFetchRequest(CFCalendarRef calendar);

void homeViewModel_clear(HomeViewModel *model) {
    {
        HomeBarChartViewModel *viewModel = model->barChartViewModel;
        for (int i = 0; i < 7; ++i) { viewModel->cumulativeArray[i].y = 0; }
        viewModel->totalTokens = 0;
    }
    {
        HomePieChartViewModel *viewModel = model->pieChartViewModel;
        viewModel->totalMinutes = 0;
        for (int i = 0; i < 3; ++i) {
            viewModel->entries[i].value = 0;
            viewModel->activityTypes[i] = 0;
        }
    }
}

int homeViewModel_addNewTokens(HomeViewModel *model, int newTokens) {
    HomeBarChartViewModel *viewModel = model->barChartViewModel;
    int index = date_indexForWeekday(model->delegate->delegate->currentDay);
    viewModel->cumulativeArray[index].y += newTokens;
    viewModel->totalTokens += newTokens;
    return viewModel->totalTokens;
}

void homeViewModel_addIntensityDurations(HomeViewModel *model, int *durations) {
    HomePieChartViewModel *viewModel = model->pieChartViewModel;
    for (int i = 0; i < 3; ++i) {
        viewModel->activityTypes[i] += durations[i];
        viewModel->totalMinutes += durations[i];
    }
    homePieChartViewModel_formatEntries(viewModel);
}

void homeViewModel_fetchData(HomeViewModel *model) {
    homeViewModel_clear(model);
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    Array_SingleActivityModel *activities = fetchWeeklyActivity(calendar);

    if (!activities) {
        CFRelease(calendar);
        return;
    }

    SingleActivityModel *activity;
    array_iter(activities, activity) {
        model->pieChartViewModel->activityTypes[activity->intensity] += activity->duration;
        model->pieChartViewModel->totalMinutes += activity->duration;
        if (!activity->tokens) continue;
        int index = date_indexForDate(activity->date, calendar);
        model->barChartViewModel->cumulativeArray[index].y += activity->tokens;
    }

    {
        HomeBarChartViewModel *viewModel = model->barChartViewModel;
        int totalTokens = 0;
        int endIndex = date_indexForDate(CFAbsoluteTimeGetCurrent(), calendar);
        for (int i = 0; i <= endIndex; ++i) {
            totalTokens += (int) viewModel->cumulativeArray[i].y;
            viewModel->cumulativeArray[i].y = totalTokens;
        }
        viewModel->totalTokens = totalTokens;
        appCoordinator_updateNavBarTokens(model->delegate->delegate, totalTokens);
    }
    homePieChartViewModel_formatEntries(model->pieChartViewModel);
    CFRelease(calendar);
    array_free(SingleActivityModel, activities);
}

bool homeViewModel_updateTimeOfDay(HomeViewModel *model) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    int timeOfDay = model->timeOfDay;
    int hour = (int) CFCalendarGetOrdinalityOfUnit(calendar, kCFCalendarUnitHour, kCFCalendarUnitDay, CFAbsoluteTimeGetCurrent());
    CFRelease(calendar);
    if (hour >= 5 && hour < 12 && timeOfDay != Morning) {
        model->timeOfDay = Morning;
        return true;
    } else if (hour < 17 && timeOfDay != Afternoon) {
        model->timeOfDay = Afternoon;
        return true;
    } else if (timeOfDay != Evening) {
        model->timeOfDay = Evening;
        return true;
    }
    return false;
}

void homeViewModel_updateForNewDay(HomeViewModel *model, int mostRecentDay, int currentDay) {
    int startIndex = date_indexForWeekday(mostRecentDay);
    int endIndex = date_indexForWeekday(currentDay);
    HomeBarChartViewModel *viewModel = model->barChartViewModel;
    int mostRecent = (int) viewModel->cumulativeArray[mostRecentDay].y;

    for (int i = startIndex + 1; i <= endIndex; ++i) {
        viewModel->cumulativeArray[i].y = mostRecent;
    }
}

NSString *homeViewModel_getGreeting(HomeViewModel *model) {
    NSString *greet;
    switch (model->timeOfDay) {
        case Morning:
            greet = @"morning";
            break;
        case Afternoon:
            greet = @"afternoon";
            break;
        default:
            greet = @"evening";
            break;
    }
    return [[NSString alloc] initWithFormat:@"Good %@!", greet];
}

NSString *homeViewModel_getCurrentTokensText(HomeViewModel *model) {
    return [[NSString alloc] initWithFormat:model->currentTokenTextFormat, model->barChartViewModel->totalTokens];
}

AlertDetails *homeViewModel_getAlertDetailsForMeetingTokenGoal(void) {
    return alertDetails_init(@"Nicely done!", @"Great job meeting your workout goal this week.");
}

#pragma mark - Helpers

NSFetchRequest *setupHomeFetchRequest(CFCalendarRef calendar) {
    double startTime = 0, endTime = 0;
    date_calcWeekEndpoints(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, true, &startTime, &endTime);
    NSFetchRequest *fetchRequest = [ActivityEntry fetchRequest];
    fetchRequest.predicate = [NSPredicate predicateWithFormat:@"(timestamp >= %f) AND (timestamp < %f)",
                              startTime, endTime];
    return fetchRequest;
}

Array_SingleActivityModel *fetchWeeklyActivity(CFCalendarRef calendar) {
    NSFetchRequest *fetchRequest = setupHomeFetchRequest(calendar);
    size_t count = 0;

    NSArray<ActivityEntry*> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!(data && (count = (data.count)) != 0)) return NULL;

    Array_SingleActivityModel *activities = array_new(SingleActivityModel);
    for (size_t i = 0; i < count; ++i) {
        ActivityEntry *object = data[i];
        SingleActivityModel model = {
            .intensity = object.type, .duration = object.duration, .tokens = object.tokens, .date = object.timestamp
        };
        array_push_back(SingleActivityModel, activities, model);
    }
    return activities;
}
