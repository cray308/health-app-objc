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
    for (int i = 0; i < 4; ++i) {
        this->areaChartViewModel.entries[i] = array_new(chartData);
        this->liftChartViewModel.entries[i] = array_new(chartData);
    }
    this->areaChartViewModel.entries[4] = array_new(chartData);
    this->gradientChartViewModel.entries = array_new(chartData);

    memcpy(this->areaChartViewModel.names, (char [][10]){"Strength", "HIC", "SE", "Endurance"}, 40);
    memcpy(this->liftChartViewModel.names,
           (char [][9]){"Squat", "Pull-up", "Bench", "Deadlift"}, 36);

    this->data = array_new(weekData);
    array_reserve(weekData, this->data, 128);
}

XAxisFormatType historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index) {
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
    if (!(size = (this->data->size))) return FormatShort;

    int startIndex = 0;
    if (index == 0) {
        startIndex = size - 26;
    } else if (index == 1) {
        startIndex = size - 52;
    }

    if (startIndex < 0) startIndex = 0;

    HistoryWeekDataModel *arr = this->data->arr;

    for (int i = startIndex; i < size; ++i) {
        HistoryWeekDataModel *e = &arr[i];

        {
            HistoryGradientChartViewModel *vm = &this->gradientChartViewModel;
            int workouts = e->totalWorkouts;
            vm->totalWorkouts += workouts;
            if (workouts > vm->maxWorkouts) vm->maxWorkouts = workouts;

            historyDataManager_createNewEntry(vm->entries, i, workouts);
        }
        {
            HistoryAreaChartViewModel *vm = &this->areaChartViewModel;
            for (int j = 0; j < 4; ++j) {
                vm->totalByType[j] += e->durationByType[j];
            }

            if (e->cumulativeDuration[3] > vm->maxActivityTime) {
                vm->maxActivityTime = e->cumulativeDuration[3];
            }

            historyDataManager_createNewEntry(vm->entries[0], i, 0);
            for (int j = 1; j < 5; ++j) {
                historyDataManager_createNewEntry(vm->entries[j], i, e->cumulativeDuration[j - 1]);
            }
        }
        {
            HistoryLiftChartViewModel *vm = &this->liftChartViewModel;
            for (int j = 0; j < 4; ++j) {
                int w = e->weightArray[j];
                vm->totalByExercise[j] += w;
                if (w > vm->maxWeight) vm->maxWeight = w;
                historyDataManager_createNewEntry(vm->entries[j], i, w);
            }
        }
    }

    // finish sub-view model calculations
    {
        HistoryGradientChartViewModel *vm = &this->gradientChartViewModel;
        vm->avgWorkouts = (double) vm->totalWorkouts / (size - startIndex);
    }
    return (size - startIndex) < 7 ? FormatShort : FormatLong;
}
