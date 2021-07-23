//
//  HistoryViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryViewModel.h"
#import "PersistenceService.h"
#import "AppUserData.h"
#include "CalendarDateHelpers.h"
#import "ChartHelpers.h"
#import "WeeklyData+CoreDataClass.h"
#include "array.h"

#define freeChartDataEntry(x) [(x) release]
#define SmallDataSetCutoff 7

gen_array(chartData, ChartDataEntry*, DSDefault_shallowCopy, freeChartDataEntry)

typedef struct {
    double weekStart;
    double weekEnd;
    int totalWorkouts;
    int durationByType[4];
    int cumulativeDuration[4];
    int weightArray[4];
} WeekDataModel;

gen_array(weekData, WeekDataModel, DSDefault_shallowCopy, DSDefault_shallowDelete)

HistoryViewModel *historyViewModel_init(void) {
    HistoryViewModel *model = malloc(sizeof(HistoryViewModel));
    if (!model) return NULL;
    model->areaChartViewModel = calloc(1, sizeof(HistoryAreaChartViewModel));
    model->liftChartViewModel = calloc(1, sizeof(HistoryLiftChartViewModel));
    model->gradientChartViewModel = calloc(1, sizeof(HistoryGradientChartViewModel));
    if (!(model->areaChartViewModel && model->gradientChartViewModel && model->liftChartViewModel)) {
        if (model->areaChartViewModel) free(model->areaChartViewModel);
        if (model->gradientChartViewModel) free(model->gradientChartViewModel);
        if (model->liftChartViewModel) free(model->liftChartViewModel);
        free(model);
        return NULL;
    }

    {
        HistoryAreaChartViewModel *vm = model->areaChartViewModel;
        for (int i = 0; i < 5; ++i) {
            vm->entries[i] = array_new(chartData);
        }
        vm->legendLabelFormats[0] = CFStringCreateWithCString(NULL, "Strength (Avg: %@)", kCFStringEncodingUTF8);
        vm->legendLabelFormats[1] = CFStringCreateWithCString(NULL, "HIC (Avg: %@)", kCFStringEncodingUTF8);
        vm->legendLabelFormats[2] = CFStringCreateWithCString(NULL, "SE (Avg: %@)", kCFStringEncodingUTF8);
        vm->legendLabelFormats[3] = CFStringCreateWithCString(NULL, "Endurance (Avg: %@)", kCFStringEncodingUTF8);
    }
    {
        HistoryLiftChartViewModel *vm = model->liftChartViewModel;
        for (int i = 0; i < 4; ++i) {
            vm->entries[i] = array_new(chartData);
        }
        vm->legendLabelFormats[0] = CFStringCreateWithCString(NULL, "Squat (Avg: %.1f)", kCFStringEncodingUTF8);
        vm->legendLabelFormats[1] = CFStringCreateWithCString(NULL, "Pull-up (Avg: %.1f)", kCFStringEncodingUTF8);
        vm->legendLabelFormats[2] = CFStringCreateWithCString(NULL, "Bench (Avg: %.1f)", kCFStringEncodingUTF8);
        vm->legendLabelFormats[3] = CFStringCreateWithCString(NULL, "Deadlift (Avg: %.1f)", kCFStringEncodingUTF8);
    }
    {
        HistoryGradientChartViewModel *vm = model->gradientChartViewModel;
        vm->legendLabelFormat = CFStringCreateWithCString(NULL, "Avg Workouts (%.2f)", kCFStringEncodingUTF8);
        vm->entries = array_new(chartData);
    }
    model->data = NULL;
    return model;
}

void historyViewModel_free(HistoryViewModel *model) {
    {
        HistoryAreaChartViewModel *vm = model->areaChartViewModel;
        for (int i = 0; i < 4; ++i) { CFRelease(vm->legendLabelFormats[i]); }
        for (int i = 0; i < 5; ++i) {  array_free(chartData, vm->entries[i]); }
        free(vm);
    }
    {
        HistoryLiftChartViewModel *vm = model->liftChartViewModel;
        for (int i = 0; i < 4; ++i) { CFRelease(vm->legendLabelFormats[i]); }
        for (int i = 0; i < 4; ++i) {  array_free(chartData, vm->entries[i]); }
        free(vm);
    }
    {
        HistoryGradientChartViewModel *vm = model->gradientChartViewModel;
        CFRelease(vm->legendLabelFormat);
        array_free(chartData, vm->entries);
        free(vm);
    }
    free(model);
}

void historyViewModel_fetchData(HistoryViewModel *model) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();

    if (!model->data) {
        model->data = array_new(weekData);
        array_reserve(weekData, model->data, 128);
    } else {
        array_clear(weekData, model->data);
    }

    size_t count = 0;
    NSFetchRequest *fetchRequest = WeeklyData.fetchRequest;
    fetchRequest.predicate = [NSPredicate predicateWithFormat:@"weekStart > %f AND weekStart < %f", date_twoYears(calendar), appUserDataShared->weekStart - 2];
    NSSortDescriptor *descriptor = [[NSSortDescriptor alloc] initWithKey:@"weekStart" ascending:true];
    fetchRequest.sortDescriptors = @[descriptor];
    [descriptor release];
    CFRelease(calendar);

    NSArray<WeeklyData *> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!(data && (count = (data.count)) != 0)) return;

    for (size_t i = 0; i < count; ++i) {
        WeeklyData *d = data[i];
        WeekDataModel m = {
            .weekStart = d.weekStart, .weekEnd = d.weekEnd, .totalWorkouts = d.totalWorkouts, .weightArray = {d.bestSquat, d.bestPullup, d.bestBench, d.bestDeadlift}, .durationByType = {d.timeStrength, d.timeHIC, d.timeSE, d.timeEndurance}
        };

        m.cumulativeDuration[0] = m.durationByType[0];
        m.cumulativeDuration[1] = m.cumulativeDuration[0] + m.durationByType[1];
        m.cumulativeDuration[2] = m.cumulativeDuration[1] + m.durationByType[2];
        m.cumulativeDuration[3] = m.cumulativeDuration[2] + m.durationByType[3];

        array_push_back(weekData, model->data, m);
    }
}

void historyViewModel_formatDataForTimeRange(HistoryViewModel *model, int index) {
    {
        HistoryAreaChartViewModel *vm = model->areaChartViewModel;
        vm->maxActivityTime = 0;
        memset(vm->totalByType, 0, 4 * sizeof(int));
        for (int i = 0; i < 5; ++i) {
            array_clear(chartData, vm->entries[i]);
        }
    }
    {
        HistoryLiftChartViewModel *vm = model->liftChartViewModel;
        vm->maxWeight = 0;
        memset(vm->totalByExercise, 0, 4 * sizeof(int));
        for (int i = 0; i < 4; ++i) {
            array_clear(chartData, vm->entries[i]);
        }
    }
    {
        HistoryGradientChartViewModel *vm = model->gradientChartViewModel;
        vm->avgWorkouts = vm->maxWorkouts = vm->totalWorkouts = 0;
        array_clear(chartData, vm->entries);
    }

    int size = 0;
    if (!(size = (array_size(model->data)))) return;

    int startIndex = 0;
    if (index == 0) {
        startIndex = size - 26;
    } else if (index == 1) {
        startIndex = size - 52;
    }

    if (startIndex < 0) startIndex = 0;

    WeekDataModel *arr = model->data->arr;
    double referenceTime = arr[startIndex].weekStart;
    NSString *format = (size - startIndex) < 7 ? @"MMM dd" : @"M/d/yy";
    [sharedHistoryXAxisFormatter update:referenceTime dateFormat:format];

    for (int i = startIndex; i < size; ++i) {
        WeekDataModel *e = &arr[i];
        double xValue = (e->weekStart - referenceTime) / DaySeconds;

        {
            HistoryGradientChartViewModel *viewModel = model->gradientChartViewModel;
            int workouts = e->totalWorkouts;
            viewModel->totalWorkouts += workouts;
            if (workouts > viewModel->maxWorkouts) viewModel->maxWorkouts = workouts;

            array_push_back(chartData, viewModel->entries, [[ChartDataEntry alloc] initWithX:xValue y:workouts]);
        }
        {
            HistoryAreaChartViewModel *viewModel = model->areaChartViewModel;
            for (int i = 0; i < 4; ++i) {
                viewModel->totalByType[i] += e->durationByType[i];
            }

            if (e->cumulativeDuration[3] > viewModel->maxActivityTime) viewModel->maxActivityTime = e->cumulativeDuration[3];

            array_push_back(chartData, viewModel->entries[0], [[ChartDataEntry alloc] initWithX:xValue y:0]);
            for (int i = 1; i < 5; ++i) {
                array_push_back(chartData, viewModel->entries[i], [[ChartDataEntry alloc] initWithX:xValue y:e->cumulativeDuration[i - 1]]);
            }
        }
        {
            HistoryLiftChartViewModel *viewModel = model->liftChartViewModel;
            for (int i = 0; i < 4; ++i) {
                int w = e->weightArray[i];
                viewModel->totalByExercise[i] += w;
                if (w > viewModel->maxWeight) viewModel->maxWeight = w;
                array_push_back(chartData, viewModel->entries[i], [[ChartDataEntry alloc] initWithX:xValue y:w]);
            }
        }
    }

    // finish sub-view model calculations
    {
        HistoryGradientChartViewModel *viewModel = model->gradientChartViewModel;
        viewModel->avgWorkouts = (double) viewModel->totalWorkouts / (size - startIndex);
    }
}

bool historyViewModel_shouldShowCharts(HistoryViewModel *model) {
    return model->data->size > 0;
}

void historyViewModel_applyUpdatesForTotalWorkouts(HistoryGradientChartViewModel *model, LineChartView *view, LineChartData *data,
                                                   LineChartDataSet *dataSet, ChartLimitLine *limitLine, NSArray<ChartLegendEntry*> *legendEntries) {
    int entryCount = array_size(model->entries);
    bool isSmallDataSet = entryCount < SmallDataSetCutoff;
    dataSet.drawCirclesEnabled = isSmallDataSet;
    [dataSet replaceEntries:[NSArray arrayWithObjects:model->entries->arr count:entryCount]];
    [data setDrawValues:isSmallDataSet];

    view.leftAxis.axisMaximum = max(1.1 * model->maxWorkouts, 7);
    view.xAxis.forceLabelsEnabled = isSmallDataSet;
    view.xAxis.labelCount = isSmallDataSet ? entryCount : (SmallDataSetCutoff - 1);
    limitLine.limit = model->avgWorkouts;

    CFStringRef label = CFStringCreateWithFormat(NULL, NULL, model->legendLabelFormat, model->avgWorkouts);
    legendEntries[0].label = (__bridge NSString*) label;
    CFRelease(label);
    view.legend.enabled = true;

    view.data = data;
    [view.data notifyDataChanged];
    [view notifyDataSetChanged];
    [view animateWithXAxisDuration:isSmallDataSet ? 1.5 : 2.5];
}

void historyViewModel_applyUpdatesForDurations(HistoryAreaChartViewModel *model, LineChartView *view, LineChartData *data,
                                               LineChartDataSet **dataSets, NSArray<ChartLegendEntry*> *legendEntries) {
    int entryCount = array_size(model->entries[0]);
    bool isSmallDataSet = entryCount < SmallDataSetCutoff;

    [dataSets[0] replaceEntries:[NSArray arrayWithObjects:model->entries[0]->arr count:entryCount]];
    for (int i = 1; i < 5; ++i) {
        dataSets[i].drawCirclesEnabled = isSmallDataSet;
        [dataSets[i] replaceEntries:[NSArray arrayWithObjects:model->entries[i]->arr count:entryCount]];

        int average = model->totalByType[i - 1] / entryCount;
        CFStringRef suffix;
        if (average > 59) {
            suffix = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d h %d m"), average / 60, average % 60);
        } else {
            suffix = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d m"), average);
        }

        CFStringRef label = CFStringCreateWithFormat(NULL, NULL, model->legendLabelFormats[i - 1], suffix);
        legendEntries[i - 1].label = (__bridge NSString*) label;
        CFRelease(label);
        CFRelease(suffix);
    }
    [data setDrawValues:isSmallDataSet];

    view.leftAxis.axisMaximum = 1.1 * model->maxActivityTime;
    view.xAxis.forceLabelsEnabled = isSmallDataSet;
    view.xAxis.labelCount = isSmallDataSet ? entryCount : (SmallDataSetCutoff - 1);
    view.legend.enabled = true;

    view.data = data;
    [view.data notifyDataChanged];
    [view notifyDataSetChanged];
    [view animateWithXAxisDuration:isSmallDataSet ? 1.5 : 2.5];
}

void historyViewModel_applyUpdatesForLifts(HistoryLiftChartViewModel *model, LineChartView *view, LineChartData *data,
                                           LineChartDataSet **dataSets, NSArray<ChartLegendEntry*> *legendEntries) {
    int entryCount = array_size(model->entries[0]);
    bool isSmallDataSet = entryCount < SmallDataSetCutoff;

    for (int i = 0; i < 4; ++i) {
        dataSets[i].drawCirclesEnabled = isSmallDataSet;
        [dataSets[i] replaceEntries:[NSArray arrayWithObjects:model->entries[i]->arr count:entryCount]];
        double average = (double) model->totalByExercise[i] / (double) entryCount;
        CFStringRef label = CFStringCreateWithFormat(NULL, NULL, model->legendLabelFormats[i], average);
        legendEntries[i].label = (__bridge NSString*) label;
        CFRelease(label);
    }
    [data setDrawValues:isSmallDataSet];

    view.leftAxis.axisMaximum = 1.1 * model->maxWeight;
    view.xAxis.forceLabelsEnabled = isSmallDataSet;
    view.xAxis.labelCount = isSmallDataSet ? entryCount : (SmallDataSetCutoff - 1);
    view.legend.enabled = true;

    view.data = data;
    [view.data notifyDataChanged];
    [view notifyDataSetChanged];
    [view animateWithXAxisDuration:isSmallDataSet ? 1.5 : 2.5];
}
