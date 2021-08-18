//
//  HistoryViewModel.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryViewModel.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"
#include "Exercise.h"
#include "PersistenceService.h"

static inline void createNewEntry(Array_chartData *arr, int x, int y) {
    id entry = createChartEntry(x, y);
    array_push_back(chartData, arr, entry);
}

gen_array_source(weekData, HistoryWeekDataModel, DSDefault_shallowCopy, DSDefault_shallowDelete)
gen_array_source(chartData, id, DSDefault_shallowCopy, releaseObj)

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

void historyViewModel_fetchData(HistoryViewModel *this) {
    struct tm localInfo;
    array_clear(weekData, this->data);
    int count = 0;

    id request = objc_staticMethod(objc_getClass("WeeklyData"), sel_getUid("fetchRequest"));
    id predicate = ((id(*)(Class,SEL,CFStringRef,...))objc_msgSend)
    (objc_getClass("NSPredicate"), sel_getUid("predicateWithFormat:"),
     CFSTR("weekStart > %lld AND weekStart < %lld"), date_twoYears, appUserDataShared->weekStart);
    id descriptor = ((id(*)(id,SEL,CFStringRef,bool))objc_msgSend)
    (allocClass("NSSortDescriptor"),
     sel_getUid("initWithKey:ascending:"), CFSTR("weekStart"), true);

    ((void(*)(id,SEL,id))objc_msgSend)(request, sel_getUid("setPredicate:"), predicate);
    ((void(*)(id,SEL,id))objc_msgSend)(request, sel_getUid("setSortDescriptors:"),
                                       createArray((id []){descriptor}, 1));

    id data = persistenceService_executeFetchRequest(request, &count);
    releaseObj(descriptor);
    if (!data) return;

    for (int i = 0; i < count; ++i) {
        id d = getObjectAtIndex(data, i);
        int timeStrength = getWorkoutTimeForType(d, WorkoutTypeStrength);
        time_t timestamp = ((int64_t(*)(id,SEL))objc_msgSend)(d, sel_getUid("weekStart"));
        localtime_r(&timestamp, &localInfo);
        HistoryWeekDataModel m = {
            .year = localInfo.tm_year % 100,
            .month = localInfo.tm_mon,
            .day = localInfo.tm_mday,
            .totalWorkouts = getTotalWorkouts(d),
            .weightArray = {
                getLiftingLimitForType(d, LiftTypeSquat),
                getLiftingLimitForType(d, LiftTypePullup),
                getLiftingLimitForType(d, LiftTypeBench),
                getLiftingLimitForType(d, LiftTypeDeadlift)
            },
            .durationByType = {
                timeStrength,
                getWorkoutTimeForType(d, WorkoutTypeHIC),
                getWorkoutTimeForType(d, WorkoutTypeSE),
                getWorkoutTimeForType(d, WorkoutTypeEndurance)
            },
            .cumulativeDuration = {[0] = timeStrength}
        };

        for (int j = 1; j < 4; ++j) {
            m.cumulativeDuration[j] = m.cumulativeDuration[j - 1] + m.durationByType[j];
        }
        array_push_back(weekData, this->data, m);
    }
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

            createNewEntry(vm->entries, i, workouts);
        }
        {
            HistoryAreaChartViewModel *vm = &this->areaChartViewModel;
            for (int j = 0; j < 4; ++j) {
                vm->totalByType[j] += e->durationByType[j];
            }

            if (e->cumulativeDuration[3] > vm->maxActivityTime) {
                vm->maxActivityTime = e->cumulativeDuration[3];
            }

            createNewEntry(vm->entries[0], i, 0);
            for (int j = 1; j < 5; ++j) {
                createNewEntry(vm->entries[j], i, e->cumulativeDuration[j - 1]);
            }
        }
        {
            HistoryLiftChartViewModel *vm = &this->liftChartViewModel;
            for (int j = 0; j < 4; ++j) {
                int w = e->weightArray[j];
                vm->totalByExercise[j] += w;
                if (w > vm->maxWeight) vm->maxWeight = w;
                createNewEntry(vm->entries[j], i, w);
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
