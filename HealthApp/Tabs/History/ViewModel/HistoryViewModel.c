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

static id getCustomColor(CFStringRef name) {
    return ((id(*)(Class,SEL,CFStringRef))objc_msgSend)(objc_getClass("UIColor"),
                                                        sel_getUid("colorNamed:"), name);
}

static inline void createNewEntry(Array_chartData *arr, int x, int y) {
    array_push_back(chartData, arr, createChartEntry(x, y));
}

gen_array_source(weekData, HistoryWeekDataModel, DSDefault_shallowCopy, DSDefault_shallowDelete)
gen_array_source(chartData, id, DSDefault_shallowCopy, releaseObj)

void historyViewModel_init(HistoryViewModel *this) {
    id colors[] = {
        getCustomColor(CFSTR("chartBlue")), getCustomColor(CFSTR("chartGreen")),
        getCustomColor(CFSTR("chartOrange")), getCustomColor(CFSTR("chartPink"))
    };
    id *areaDataSets = this->workoutTypeViewModel.dataSets;
    setupLegendEntries(this->totalWorkoutsViewModel.legendEntries,
                       (id []){createColor("systemTealColor")}, 1);
    setupLegendEntries(this->workoutTypeViewModel.legendEntries, colors, 4);
    setupLegendEntries(this->liftViewModel.legendEntries, colors, 4);

    this->totalWorkoutsViewModel.entries = array_new(chartData);
    this->totalWorkoutsViewModel.dataSet = createDataSet(createColor("systemRedColor"));
    this->workoutTypeViewModel.dataSets[0] = createEmptyDataSet();

    for (int i = 0; i < 4; ++i) {
        this->workoutTypeViewModel.entries[i] = array_new(chartData);
        this->liftViewModel.entries[i] = array_new(chartData);

        this->workoutTypeViewModel.dataSets[i + 1] = createDataSet(colors[i]);
        this->liftViewModel.dataSets[i] = createDataSet(colors[i]);

        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("workoutTypes%d"), i);
        this->workoutTypeViewModel.names[i] = localize(key);
        CFRelease(key);

        key = CFStringCreateWithFormat(NULL, NULL, CFSTR("liftTypes%d"), i);
        this->liftViewModel.names[i] = localize(key);
        CFRelease(key);
    }

    this->workoutTypeViewModel.entries[4] = array_new(chartData);

    this->totalWorkoutsViewModel.chartData = createChartData((id []){
        this->totalWorkoutsViewModel.dataSet
    }, 1);
    this->workoutTypeViewModel.chartData = createChartData((id []){
        areaDataSets[4], areaDataSets[3], areaDataSets[2], areaDataSets[1]
    }, 4);
    this->liftViewModel.chartData = createChartData(this->liftViewModel.dataSets, 4);

    this->workoutTypeViewModel.durationStr = CFStringCreateCopy(NULL, CFSTR(""));

    for (int i = 0; i < 12; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("months%02d"), i);
        this->formatter.months[i] = localize(key);
        CFRelease(key);
    }
    this->formatter.currString = CFStringCreateCopy(NULL, CFSTR(""));

    this->data = array_new(weekData);
    array_reserve(weekData, this->data, 128);
}

void historyViewModel_fetchData(HistoryViewModel *this) {
    ((void(*)(id,SEL,void(^)(void)))objc_msgSend)(backgroundContext, sel_getUid("performBlock:"), ^{
        array_clear(weekData, this->data);
        struct tm localInfo;
        int count = 0;

        id request = fetchRequest();
        id predicate = createPredicate(CFSTR("weekStart > %lld AND weekStart < %lld"),
                                       date_twoYears, userData->weekStart);
        id descriptors[] = {createSortDescriptor()};
        CFArrayRef array = CFArrayCreate(NULL, (const void **)descriptors, 1, &kCocoaArrCallbacks);

        setPredicate(request, predicate);
        setDescriptors(request, array);

        CFArrayRef data = persistenceService_executeFetchRequest(request, &count);
        releaseObj(descriptors[0]);
        CFRelease(array);
        if (data) {
            for (int i = 0; i < count; ++i) {
                const id d = (const id) CFArrayGetValueAtIndex(data, i);
                int timeStrength = weekData_getWorkoutTimeForType(d, WorkoutTypeStrength);
                time_t timestamp = weekData_getWeekStart(d);
                localtime_r(&timestamp, &localInfo);
                HistoryWeekDataModel m = {
                    .year = localInfo.tm_year % 100,
                    .month = localInfo.tm_mon,
                    .day = localInfo.tm_mday,
                    .totalWorkouts = weekData_getTotalWorkouts(d),
                    .weightArray = {
                        weekData_getLiftingLimitForType(d, LiftTypeSquat),
                        weekData_getLiftingLimitForType(d, LiftTypePullup),
                        weekData_getLiftingLimitForType(d, LiftTypeBench),
                        weekData_getLiftingLimitForType(d, LiftTypeDeadlift)
                    },
                    .durationByType = {
                        timeStrength,
                        weekData_getWorkoutTimeForType(d, WorkoutTypeHIC),
                        weekData_getWorkoutTimeForType(d, WorkoutTypeSE),
                        weekData_getWorkoutTimeForType(d, WorkoutTypeEndurance)
                    },
                    .cumulativeDuration = {[0] = timeStrength}
                };

                for (int j = 1; j < 4; ++j)
                m.cumulativeDuration[j] = m.cumulativeDuration[j - 1] + m.durationByType[j];
                array_push_back(weekData, this->data, m);
            }
        }
    });
}

void historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index) {
    this->isSmall = true;
    this->totalWorkoutsViewModel.avgWorkouts = 0;
    this->totalWorkoutsViewModel.yMax = 0;
    this->workoutTypeViewModel.yMax = 0;
    this->liftViewModel.yMax = 0;
    memset(this->liftViewModel.totalByExercise, 0, 4 * sizeof(int));
    memset(this->workoutTypeViewModel.totalByType, 0, 4 * sizeof(int));
    array_clear(chartData, this->totalWorkoutsViewModel.entries);
    for (int i = 0; i < 4; ++i) {
        array_clear(chartData, this->workoutTypeViewModel.entries[i]);
        array_clear(chartData, this->liftViewModel.entries[i]);
    }
    array_clear(chartData, this->workoutTypeViewModel.entries[4]);

    int size = 0;
    if (!(size = (this->data->size))) return;

    int startIndex = 0;
    if (index == 0) {
        startIndex = size - 26;
    } else if (index == 1) {
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
        createNewEntry(this->totalWorkoutsViewModel.entries, i, workouts);

        for (int j = 0; j < 4; ++j) {
            this->workoutTypeViewModel.totalByType[j] += e->durationByType[j];

            int weight = e->weightArray[j];
            this->liftViewModel.totalByExercise[j] += weight;
            if (weight > maxWeight)
                maxWeight = weight;
            createNewEntry(this->liftViewModel.entries[j], i, weight);
        }

        if (e->cumulativeDuration[3] > maxActivityTime)
            maxActivityTime = e->cumulativeDuration[3];
        createNewEntry(this->workoutTypeViewModel.entries[0], i, 0);
        for (int j = 1; j < 5; ++j)
            createNewEntry(this->workoutTypeViewModel.entries[j], i, e->cumulativeDuration[j - 1]);
    }

    this->totalWorkoutsViewModel.avgWorkouts = (double) totalWorkouts / nEntries;
    this->totalWorkoutsViewModel.yMax = maxWorkouts < 7 ? 7 : 1.1 * maxWorkouts;
    this->workoutTypeViewModel.yMax = 1.1 * maxActivityTime;
    this->liftViewModel.yMax = 1.1 * maxWeight;

    CFStringRef totalWorkoutLegend = localize(CFSTR("totalWorkoutsLegend"));
    CFStringRef liftLegend = localize(CFSTR("liftLegend"));
    CFStringRef typeLegend = localize(CFSTR("workoutTypeLegend"));
    char buf[10];

    CFStringRef label = CFStringCreateWithFormat(NULL, NULL, totalWorkoutLegend,
                                                 this->totalWorkoutsViewModel.avgWorkouts);
    setLegendLabel(this->totalWorkoutsViewModel.legendEntries[0], label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        double liftAverage = (double) this->liftViewModel.totalByExercise[i] / nEntries;
        int typeAverage = this->workoutTypeViewModel.totalByType[i] / nEntries;
        if (typeAverage > 59) {
            sprintf(buf, "%d h %d m", typeAverage / 60, typeAverage % 60);
        } else {
            sprintf(buf, "%d m", typeAverage);
        }
        label = CFStringCreateWithFormat(NULL, NULL, typeLegend,
                                         this->workoutTypeViewModel.names[i], buf);
        setLegendLabel(this->workoutTypeViewModel.legendEntries[i], label);
        CFRelease(label);

        label = CFStringCreateWithFormat(NULL, NULL, liftLegend,
                                         this->liftViewModel.names[i], liftAverage);
        setLegendLabel(this->liftViewModel.legendEntries[i], label);
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

CFStringRef workoutTypeViewModel_getDuration(HistoryWorkoutTypeChartViewModel *this, int minutes) {
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
