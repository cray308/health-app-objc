//
//  HistoryViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryViewModel.h"
#include "HistoryDataManager.h"
#include "CalendarDateHelpers.h"

gen_array_source(weekData, HistoryWeekDataModel, DSDefault_shallowCopy, DSDefault_shallowDelete)
gen_array_source(chartData, void*, DSDefault_shallowCopy, freeChartDataEntry)

void historyViewModel_init(HistoryViewModel *this) {
    {
        CFStringRef labels[] = {CFSTR("Strength (Avg: %@)"), CFSTR("HIC (Avg: %@)"), CFSTR("SE (Avg: %@)"),
            CFSTR("Endurance (Avg: %@)")};
        HistoryAreaChartViewModel *vm = &this->areaChartViewModel;
        for (int i = 0; i < 4; ++i) {
            vm->entries[i] = array_new(chartData);
            vm->legendLabelFormats[i] = labels[i];
        }
        vm->entries[4] = array_new(chartData);
    }
    {
        CFStringRef labels[] = {CFSTR("Squat (Avg: %.1f)"), CFSTR("Pull-up (Avg: %.1f)"), CFSTR("Bench (Avg: %.1f)"),
            CFSTR("Deadlift (Avg: %.1f)")};
        HistoryLiftChartViewModel *vm = &this->liftChartViewModel;
        for (int i = 0; i < 4; ++i) {
            vm->entries[i] = array_new(chartData);
            vm->legendLabelFormats[i] = labels[i];
        }
    }
    {
        HistoryGradientChartViewModel *vm = &this->gradientChartViewModel;
        vm->legendLabelFormat = CFSTR("Avg Workouts (%.2f)");
        vm->entries = array_new(chartData);
    }
    this->data = array_new(weekData);
    array_reserve(weekData, this->data, 128);
}

void historyViewModel_free(HistoryViewModel *this) {
    for (int i = 0; i < 4; ++i) {
        array_free(chartData, this->liftChartViewModel.entries[i]);
        array_free(chartData, this->areaChartViewModel.entries[i]);
    }
    array_free(chartData, this->areaChartViewModel.entries[4]);
    array_free(chartData, this->gradientChartViewModel.entries);
    array_free(weekData, this->data);
}

void historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index, HistoryXAxisFormatter *formatter) {
    {
        HistoryAreaChartViewModel *vm = &this->areaChartViewModel;
        vm->maxActivityTime = 0;
        memset(vm->totalByType, 0, 4 * sizeof(int));
        for (int i = 0; i < 5; ++i) {
            array_clear(chartData, vm->entries[i]);
        }
    }
    {
        HistoryLiftChartViewModel *vm = &this->liftChartViewModel;
        vm->maxWeight = 0;
        memset(vm->totalByExercise, 0, 4 * sizeof(int));
        for (int i = 0; i < 4; ++i) {
            array_clear(chartData, vm->entries[i]);
        }
    }
    {
        HistoryGradientChartViewModel *vm = &this->gradientChartViewModel;
        vm->avgWorkouts = vm->maxWorkouts = vm->totalWorkouts = 0;
        array_clear(chartData, vm->entries);
    }

    int size = 0;
    if (!(size = (array_size(this->data)))) return;

    int startIndex = 0;
    if (index == 0) {
        startIndex = size - 26;
    } else if (index == 1) {
        startIndex = size - 52;
    }

    if (startIndex < 0) startIndex = 0;

    HistoryWeekDataModel *arr = this->data->arr;
    double referenceTime = arr[startIndex].weekStart;
    formatter->refTime = referenceTime;
    CFDateFormatterSetFormat(formatter->formatter, (size - startIndex) < 7 ? CFSTR("MMM dd") : CFSTR("M/d/yy"));

    for (int i = startIndex; i < size; ++i) {
        HistoryWeekDataModel *e = &arr[i];
        double xValue = (e->weekStart - referenceTime) / DaySeconds;

        {
            HistoryGradientChartViewModel *vm = &this->gradientChartViewModel;
            int workouts = e->totalWorkouts;
            vm->totalWorkouts += workouts;
            if (workouts > vm->maxWorkouts) vm->maxWorkouts = workouts;

            historyDataManager_createNewEntry(vm->entries, xValue, workouts);
        }
        {
            HistoryAreaChartViewModel *vm = &this->areaChartViewModel;
            for (int i = 0; i < 4; ++i) {
                vm->totalByType[i] += e->durationByType[i];
            }

            if (e->cumulativeDuration[3] > vm->maxActivityTime) {
                vm->maxActivityTime = e->cumulativeDuration[3];
            }

            historyDataManager_createNewEntry(vm->entries[0], xValue, 0);
            for (int i = 1; i < 5; ++i) {
                historyDataManager_createNewEntry(vm->entries[i], xValue, e->cumulativeDuration[i - 1]);
            }
        }
        {
            HistoryLiftChartViewModel *vm = &this->liftChartViewModel;
            for (int i = 0; i < 4; ++i) {
                int w = e->weightArray[i];
                vm->totalByExercise[i] += w;
                if (w > vm->maxWeight) vm->maxWeight = w;
                historyDataManager_createNewEntry(vm->entries[i], xValue, w);
            }
        }
    }

    // finish sub-view model calculations
    {
        HistoryGradientChartViewModel *vm = &this->gradientChartViewModel;
        vm->avgWorkouts = (double) vm->totalWorkouts / (size - startIndex);
    }
}
