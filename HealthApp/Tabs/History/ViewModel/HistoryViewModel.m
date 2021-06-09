//
//  HistoryViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryViewModel.h"
#import "PersistenceService.h"
#import "CalendarDateHelpers.h"
#import "ChartHelpers.h"
#import "HistoryAreaChartViewModel.h"
#import "HistoryPieChartViewModel.h"
#import "HistoryGradientChartViewModel.h"
#import "WeekStats+CoreDataClass.h"

gen_array(weekStats, WeeklyActivitySummaryModel, DSDefault_shallowCopy, DSDefault_shallowDelete)

Array_weekStats *fetchHistoricalDataForIndex(int index, CFCalendarRef calendar);
NSFetchRequest *setupHistoryFetchRequest(int index, CFCalendarRef calendar);

#pragma mark - Coordinator/View Related

void historyViewModel_clear(HistoryViewModel *model) {
    {
        HistoryAreaChartViewModel *vm = model->areaChartViewModel;
        vm->maxActivityTime = 0;
        memset(vm->totalByIntensity, 0, 3 * sizeof(int));
        for (int i = 0; i < 4; ++i) {
            array_clear(chartData, vm->entries[i]);
        }
    }
    {
        HistoryGradientChartViewModel *vm = model->gradientChartViewModel;
        vm->avgTokens = vm->maxTokens = vm->totalTokens = 0;
        array_clear(chartData, vm->entries);
    }
    {
        HistoryPieChartViewModel *vm = model->pieChartViewModel;
        vm->activitySum = vm->entryCount = 0;
        for (int i = 0; i < 7; ++i) {
            vm->totalDailyActivity[i] = 0;
            vm->entries[i].value = 0;
        }
    }
}

void historyViewModel_fetchData(HistoryViewModel *model, int index) {
    historyViewModel_clear(model);
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    Array_weekStats *stats = fetchHistoricalDataForIndex(index, calendar);
    CFRelease(calendar);
    if (!stats) return;

    double referenceTime = stats->arr[0].weekStart;
    NSString *format = array_size(stats) < 7 ? @"MMM dd" : @"M/d/yy";
    [sharedHistoryXAxisFormatter update:referenceTime dateFormat:format];

    WeeklyActivitySummaryModel *entry;
    array_iter(stats, entry) {
        double xValue = (entry->weekStart - referenceTime) / DaySeconds;

        {
            HistoryGradientChartViewModel *viewModel = model->gradientChartViewModel;
            int tokensEarned = entry->tokensEarned;
            viewModel->totalTokens += tokensEarned;
            if (tokensEarned > viewModel->maxTokens) viewModel->maxTokens = tokensEarned;

            array_push_back(chartData, viewModel->entries, [[ChartDataEntry alloc] initWithX:xValue y:tokensEarned]);
        }
        {
            HistoryAreaChartViewModel *viewModel = model->areaChartViewModel;
            int *times = &entry->durationByIntensity[0];

            viewModel->totalByIntensity[0] += times[0];
            for (int i = 1; i < 3; ++i) {
                viewModel->totalByIntensity[i] += times[i];
                times[i] += times[i - 1];
            }

            if (times[2] > viewModel->maxActivityTime) viewModel->maxActivityTime = times[2];

            array_push_back(chartData, viewModel->entries[0], [[ChartDataEntry alloc] initWithX:xValue y:0]);
            for (int i = 1; i < 4; ++i) {
                array_push_back(chartData, viewModel->entries[i], [[ChartDataEntry alloc] initWithX:xValue y:times[i - 1]]);
            }
        }
        {
            HistoryPieChartViewModel *viewModel = model->pieChartViewModel;
            for (int i = 0; i < 7; ++i) {
                viewModel->totalDailyActivity[i] += entry->durationByDay[i];
            }
            viewModel->entryCount += 1;
        }
    }

    // finish sub-view model calculations
    {
        HistoryGradientChartViewModel *viewModel = model->gradientChartViewModel;
        viewModel->avgTokens = (double) viewModel->totalTokens / array_size(stats);
    }
    {
        HistoryPieChartViewModel *viewModel = model->pieChartViewModel;
        int sum = 0;
        for (int i = 0; i < 7; ++i) { sum += viewModel->totalDailyActivity[i]; }
        viewModel->activitySum = sum;
        if (sum) {
            for (int i = 0; i < 7; ++i) {
                viewModel->entries[i].value = (double) viewModel->totalDailyActivity[i] / sum;
            }
        }
    }
    array_free(weekStats, stats);
}

#pragma mark - Core Data Handling

NSFetchRequest *setupHistoryFetchRequest(int index, CFCalendarRef calendar) {
    double startDateTime = 0;
    switch (index) {
        case 0:
            startDateTime = date_lastMonth(calendar);
            break;
        case 1:
            startDateTime = date_sixMonths(calendar);
            break;
        default:
            startDateTime = date_lastYear(calendar);
            break;
    }
    NSFetchRequest *fetchRequest = [WeekStats fetchRequest];
    fetchRequest.predicate = [NSPredicate predicateWithFormat:@"start > %f", startDateTime];
    NSSortDescriptor *descriptor = [[NSSortDescriptor alloc] initWithKey:@"start" ascending:true];
    fetchRequest.sortDescriptors = @[descriptor];
    [descriptor release];
    return fetchRequest;
}

Array_weekStats *fetchHistoricalDataForIndex(int index, CFCalendarRef calendar) {
    NSFetchRequest *fetchRequest = setupHistoryFetchRequest(index, calendar);
    size_t count = 0;

    NSArray<WeekStats*> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!(data && (count = (data.count)) != 0)) return NULL;

    Array_weekStats *stats = array_new(weekStats);
    array_reserve(weekStats, stats, count);
    for (size_t i = 0; i < count; ++i) {
        WeekStats *object = data[i];
        WeeklyActivitySummaryModel model = {
            .weekStart = object.start, .weekEnd = object.end, .tokensEarned = object.tokens
        };
        [object populateIntensities:&model.durationByIntensity[0]];
        [object populateDurationByDay:&model.durationByDay[0]];
        array_push_back(weekStats, stats, model);
    }
    return stats;
}
