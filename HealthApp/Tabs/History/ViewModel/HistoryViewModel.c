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
#include "SwiftBridging.h"

typedef enum {
    HistoryTimeRange6Months,
    HistoryTimeRange1Year
} HistoryTimeRange;

static inline void createNewEntry(Array_object *arr, int x, int y) {
    array_push_back(object, arr, createChartEntry(x, y));
}

gen_array_source(weekData, HistoryWeekDataModel, DSDefault_shallowCopy, DSDefault_shallowDelete)

void historyViewModel_init(HistoryViewModel *this) {
    id chartColors[4];
    for (int i = 0; i < 4; ++i) {
        CFStringRef name = CFStringCreateWithFormat(NULL, NULL, CFSTR("chartColor%d"), i);
        chartColors[i] = staticMethodWithString(objc_getClass("UIColor"),
                                                sel_getUid("colorNamed:"), name);
        CFRelease(name);
    }
    id *areaDataSets = this->workoutTypeModel.dataSets;
    setupLegendEntries(this->totalWorkoutsModel.legendEntries,
                       (id []){createColor("systemTealColor")}, 1);
    setupLegendEntries(this->workoutTypeModel.legendEntries, chartColors, 4);
    setupLegendEntries(this->liftModel.legendEntries, chartColors, 4);

    this->totalWorkoutsModel.entries = array_new(object);
    this->totalWorkoutsModel.dataSet = createDataSet(createColor("systemRedColor"));
    this->workoutTypeModel.dataSets[0] = createEmptyDataSet();
    fillStringArray(this->workoutTypeModel.names, CFSTR("workoutTypes%d"), 4);
    fillStringArray(this->liftModel.names, CFSTR("liftTypes%d"), 4);
    fillStringArray(this->formatter.months, CFSTR("months%02d"), 12);
    this->totalWorkoutsModel.legendFormat = localize(CFSTR("totalWorkoutsLegend"));
    this->liftModel.legendFormat = localize(CFSTR("liftLegend"));
    this->workoutTypeModel.legendFormat = localize(CFSTR("workoutTypeLegend"));

    for (int i = 0; i < 4; ++i) {
        this->workoutTypeModel.entries[i] = array_new(object);
        this->liftModel.entries[i] = array_new(object);

        this->workoutTypeModel.dataSets[i + 1] = createDataSet(chartColors[i]);
        this->liftModel.dataSets[i] = createDataSet(chartColors[i]);
    }

    this->workoutTypeModel.entries[4] = array_new(object);

    this->totalWorkoutsModel.chartData = createChartData((id []){
        this->totalWorkoutsModel.dataSet
    }, 1);
    this->workoutTypeModel.chartData = createChartData((id []){
        areaDataSets[4], areaDataSets[3], areaDataSets[2], areaDataSets[1]
    }, 4);
    this->liftModel.chartData = createChartData(this->liftModel.dataSets, 4);

    this->workoutTypeModel.durationStr = CFStringCreateCopy(NULL, CFSTR(""));
    this->formatter.currString = CFStringCreateCopy(NULL, CFSTR(""));

    this->data = array_new(weekData);
    array_reserve(weekData, this->data, 128);
}

void historyViewModel_fetchData(HistoryViewModel *this) {
    runInBackground((^{
        array_clear(weekData, this->data);
        struct tm localInfo;
        int count = 0;

        id request = fetchRequest(createPredicate(CFSTR("weekStart < %lld"), userData->weekStart));
        CFArrayRef data = persistenceService_executeFetchRequest(request, &count, true);
        if (data) {
            for (int i = 0; i < count; ++i) {
                const id d = (const id) CFArrayGetValueAtIndex(data, i);
                int timeStrength = weekData_getWorkoutTimeForType(d, WorkoutStrength);
                time_t timestamp = weekData_getWeekStart(d);
                localtime_r(&timestamp, &localInfo);
                HistoryWeekDataModel m = {
                    .year = localInfo.tm_year % 100,
                    .month = localInfo.tm_mon,
                    .day = localInfo.tm_mday,
                    .totalWorkouts = weekData_getTotalWorkouts(d),
                    .weightArray = {
                        weekData_getLiftingLimitForType(d, LiftSquat),
                        weekData_getLiftingLimitForType(d, LiftPullup),
                        weekData_getLiftingLimitForType(d, LiftBench),
                        weekData_getLiftingLimitForType(d, LiftDeadlift)
                    },
                    .durationByType = {
                        timeStrength,
                        weekData_getWorkoutTimeForType(d, WorkoutHIC),
                        weekData_getWorkoutTimeForType(d, WorkoutSE),
                        weekData_getWorkoutTimeForType(d, WorkoutEndurance)
                    },
                    .cumulativeDuration = {[0] = timeStrength}
                };

                for (int j = 1; j < 4; ++j)
                m.cumulativeDuration[j] = m.cumulativeDuration[j - 1] + m.durationByType[j];
                array_push_back(weekData, this->data, m);
            }
        }
    }));
}

void historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index) {
    this->isSmall = true;
    this->totalWorkoutsModel.avgWorkouts = 0;
    this->totalWorkoutsModel.yMax = 0;
    this->workoutTypeModel.yMax = 0;
    this->liftModel.yMax = 0;
    memset(this->liftModel.totalByExercise, 0, 4 * sizeof(int));
    memset(this->workoutTypeModel.totalByType, 0, 4 * sizeof(int));
    array_clear(object, this->totalWorkoutsModel.entries);
    for (int i = 0; i < 4; ++i) {
        array_clear(object, this->workoutTypeModel.entries[i]);
        array_clear(object, this->liftModel.entries[i]);
    }
    array_clear(object, this->workoutTypeModel.entries[4]);

    int size = 0;
    if (!(size = (this->data->size))) return;

    int startIndex = 0;
    if (index == HistoryTimeRange6Months) {
        startIndex = size - 26;
    } else if (index == HistoryTimeRange1Year) {
        startIndex = size - 52;
    }

    if (startIndex < 0)
        startIndex = 0;
    int nEntries = size - startIndex;
    if (nEntries >= 7)
        this->isSmall = false;

    HistoryWeekDataModel *arr = this->data->arr;
    int totalWorkouts = 0, maxWorkouts = 0, maxActivityTime = 0, maxWeight = 0;

    for (int i = startIndex; i < size; ++i) {
        HistoryWeekDataModel *e = &arr[i];

        int workouts = e->totalWorkouts;
        totalWorkouts += workouts;
        if (workouts > maxWorkouts)
            maxWorkouts = workouts;
        createNewEntry(this->totalWorkoutsModel.entries, i, workouts);

        for (int j = 0; j < 4; ++j) {
            this->workoutTypeModel.totalByType[j] += e->durationByType[j];

            int weight = e->weightArray[j];
            this->liftModel.totalByExercise[j] += weight;
            if (weight > maxWeight)
                maxWeight = weight;
            createNewEntry(this->liftModel.entries[j], i, weight);
        }

        if (e->cumulativeDuration[3] > maxActivityTime)
            maxActivityTime = e->cumulativeDuration[3];
        createNewEntry(this->workoutTypeModel.entries[0], i, 0);
        for (int j = 1; j < 5; ++j)
            createNewEntry(this->workoutTypeModel.entries[j], i, e->cumulativeDuration[j - 1]);
    }

    this->totalWorkoutsModel.avgWorkouts = (float) totalWorkouts / nEntries;
    this->totalWorkoutsModel.yMax = maxWorkouts < 7 ? 7 : 1.1 * maxWorkouts;
    this->workoutTypeModel.yMax = 1.1 * maxActivityTime;
    this->liftModel.yMax = 1.1 * maxWeight;

    char buf[10];
    CFStringRef label = CFStringCreateWithFormat(NULL, NULL, this->totalWorkoutsModel.legendFormat,
                                                 this->totalWorkoutsModel.avgWorkouts);
    setLegendLabel(this->totalWorkoutsModel.legendEntries[0], label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        float liftAverage = (float) this->liftModel.totalByExercise[i] / nEntries;
        int typeAverage = this->workoutTypeModel.totalByType[i] / nEntries;
        if (typeAverage > 59) {
            sprintf(buf, "%d h %d m", typeAverage / 60, typeAverage % 60);
        } else {
            sprintf(buf, "%d m", typeAverage);
        }
        label = CFStringCreateWithFormat(NULL, NULL, this->workoutTypeModel.legendFormat,
                                         this->workoutTypeModel.names[i], buf);
        setLegendLabel(this->workoutTypeModel.legendEntries[i], label);
        CFRelease(label);

        label = CFStringCreateWithFormat(NULL, NULL, this->liftModel.legendFormat,
                                         this->liftModel.names[i], liftAverage);
        setLegendLabel(this->liftModel.legendEntries[i], label);
        CFRelease(label);
    }
}

CFStringRef historyViewModel_getXAxisLabel(HistoryViewModel *this, int index) {
    CFRelease(this->formatter.currString);
    const HistoryWeekDataModel *model = &this->data->arr[index];
    this->formatter.currString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%d/%d"),
                                                          this->formatter.months[model->month],
                                                          model->day, model->year);
    return this->formatter.currString;
}

CFStringRef workoutTypeViewModel_getDuration(WorkoutTypeChartViewModel *this, int minutes) {
    CFRelease(this->durationStr);
    if (!minutes) {
        this->durationStr = CFStringCreateCopy(NULL, CFSTR(""));
    } else if (minutes < 60) {
        this->durationStr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%dm"), minutes);
    } else {
        this->durationStr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%dh %dm"),
                                                     minutes / 60, minutes % 60);
    }
    return this->durationStr;
}
