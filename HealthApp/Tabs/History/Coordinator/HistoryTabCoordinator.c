#include "HistoryTabCoordinator.h"
#include "AppUserData.h"
#include "ExerciseManager.h"
#include "HistoryVC.h"
#include "PersistenceService.h"
#include "SwiftBridging.h"
#include "ViewControllerHelpers.h"

gen_array_source(pt, CGPoint, DSDefault_shallowCopy, DSDefault_shallowDelete)

struct WeekDataModel {
    struct HistTimeData timeData;
    int totalWorkouts;
    int durationByType[4];
    int cumulativeDuration[4];
    short weightArray[4];
};

extern id createDataSet(int color, id fillSet);
extern id createChartData(CFArrayRef dataSets, int lineWidth, uint8_t options);
extern void setLegendLabel(id entry, CFStringRef text);

static void populateData(HistoryViewModel *m, struct WeekDataModel *results, int size);

static inline void createNewEntry(Array_pt *arr, int x, int y) {
    CGPoint point = {x, y};
    array_push_back(pt, arr, point);
}

void historyCoordinator_start(HistoryTabCoordinator *this) {
    HistoryViewModel *model = &this->model;
    int chartColors[] = {0, 1, 2, 3};
    id *areaDataSets = model->workoutTypes.dataSets;

    model->totalWorkouts.entries = array_new(pt);
    model->totalWorkouts.dataSet = createDataSet(5, nil);
    model->workoutTypes.dataSets[0] = createDataSet(-1, nil);
    fillStringArray(model->workoutTypes.names, CFSTR("workoutTypes%d"), 4);
    fillStringArray(model->lifts.names, CFSTR("liftTypes%d"), 4);
    fillStringArray(model->formatter.months, CFSTR("months%02d"), 12);
    model->formatter.currString = CFStringCreateCopy(NULL, CFSTR(""));
    model->totalWorkouts.legendFormat = localize(CFSTR("totalWorkoutsLegend"));
    model->lifts.legendFormat = localize(CFSTR("liftLegend"));
    model->workoutTypes.legendFormat = localize(CFSTR("workoutTypeLegend"));

    for (int i = 0; i < 4; ++i) {
        model->workoutTypes.entries[i] = array_new(pt);
        model->lifts.entries[i] = array_new(pt);

        id boundary = model->workoutTypes.dataSets[i];
        model->workoutTypes.dataSets[i + 1] = createDataSet(chartColors[i], boundary);
        model->lifts.dataSets[i] = createDataSet(chartColors[i], nil);
    }
    model->workoutTypes.entries[4] = array_new(pt);

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void **)(id []){model->totalWorkouts.dataSet},
                                       1, &(CFArrayCallBacks){0});
    model->totalWorkouts.chartData = createChartData(dataArr, 1, 3);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)(id []){
        areaDataSets[4], areaDataSets[3], areaDataSets[2], areaDataSets[1]
    }, 4, &(CFArrayCallBacks){0});
    model->workoutTypes.chartData = createChartData(dataArr, 1, 5);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)model->lifts.dataSets, 4, &(CFArrayCallBacks){0});
    model->lifts.chartData = createChartData(dataArr, 3, 0);
    CFRelease(dataArr);

    setupNavVC(this->navVC, historyVC_init(this));
}

void historyCoordinator_fetchData(HistoryTabCoordinator *this) {
    HistoryViewModel *model = &this->model;
    runInBackground((^{
        struct tm localInfo;
        int count = 0;

        id request = fetchRequest(createPredicate(CFSTR("weekStart < %lld"), userData->weekStart));
        CFArrayRef data = persistenceService_executeFetchRequest(request, &count, true);
        if (data) {
            struct WeekDataModel *results = malloc(sizeof(struct WeekDataModel) << 7);
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                struct WeekDataModel *r = &results[i];
                time_t timestamp = (time_t) weekData_getWeekStart(d);
                localtime_r(&timestamp, &localInfo);
                r->timeData.year = localInfo.tm_year % 100;
                r->timeData.month = localInfo.tm_mon;
                r->timeData.day = localInfo.tm_mday;
                r->totalWorkouts = weekData_getTotalWorkouts(d);
                r->durationByType[0] = weekData_getWorkoutTimeForType(d, WorkoutStrength);
                r->durationByType[1] = weekData_getWorkoutTimeForType(d, WorkoutHIC);
                r->durationByType[2] = weekData_getWorkoutTimeForType(d, WorkoutSE);
                r->durationByType[3] = weekData_getWorkoutTimeForType(d, WorkoutEndurance);
                weekData_getLiftingLimits(d, r->weightArray);

                r->cumulativeDuration[0] = results[i].durationByType[0];
                for (int j = 1; j < 4; ++j) {
                    r->cumulativeDuration[j] = r->cumulativeDuration[j - 1] + r->durationByType[j];
                }

                memcpy(&model->formatter.data[i], &r->timeData, sizeof(struct HistTimeData));
            }
            populateData(model, results, count);
        }
    }));
}

void populateData(HistoryViewModel *m, struct WeekDataModel *results, int size) {
    int refIndices[] = {size - 26, size - 52, 0};
    if (refIndices[1] < 0) refIndices[1] = 0;
    if (refIndices[0] < 0) refIndices[0] = 0;

    memcpy(m->nEntries, (int[]){size - refIndices[0], size - refIndices[1], size}, 3 * sizeof(int));

    int totalWorkouts[3] = {0}, maxWorkouts[3] = {0}, maxTime[3] = {0}, maxWeight[3] = {0};
    int totalByType[3][4] = {{0},{0},{0}}, totalByExercise[3][4] = {{0},{0},{0}};

    for (int i = 0; i < refIndices[1]; ++i) {
        struct WeekDataModel *e = &results[i];

        totalWorkouts[2] += e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts[2]) maxWorkouts[2] = e->totalWorkouts;
        createNewEntry(m->totalWorkouts.entries, i, e->totalWorkouts);

        for (int j = 0; j < 4; ++j) {
            totalByType[2][j] += e->durationByType[j];

            totalByExercise[2][j] += e->weightArray[j];
            if (e->weightArray[j] > maxWeight[2]) maxWeight[2] = e->weightArray[j];
            createNewEntry(m->lifts.entries[j], i, e->weightArray[j]);
        }

        if (e->cumulativeDuration[3] > maxTime[2]) maxTime[2] = e->cumulativeDuration[3];
        createNewEntry(m->workoutTypes.entries[0], i, 0);
        for (int j = 1; j < 5; ++j) {
            createNewEntry(m->workoutTypes.entries[j], i, e->cumulativeDuration[j - 1]);
        }
    }

    for (int i = refIndices[1]; i < refIndices[0]; ++i) {
        struct WeekDataModel *e = &results[i];

        totalWorkouts[2] += e->totalWorkouts;
        totalWorkouts[1] += e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts[2]) maxWorkouts[2] = e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts[1]) maxWorkouts[1] = e->totalWorkouts;
        createNewEntry(m->totalWorkouts.entries, i, e->totalWorkouts);

        for (int j = 0; j < 4; ++j) {
            totalByType[2][j] += e->durationByType[j];
            totalByType[1][j] += e->durationByType[j];

            totalByExercise[2][j] += e->weightArray[j];
            totalByExercise[1][j] += e->weightArray[j];
            if (e->weightArray[j] > maxWeight[2]) maxWeight[2] = e->weightArray[j];
            if (e->weightArray[j] > maxWeight[1]) maxWeight[1] = e->weightArray[j];
            createNewEntry(m->lifts.entries[j], i, e->weightArray[j]);
        }

        if (e->cumulativeDuration[3] > maxTime[2]) maxTime[2] = e->cumulativeDuration[3];
        if (e->cumulativeDuration[3] > maxTime[1]) maxTime[1] = e->cumulativeDuration[3];
        createNewEntry(m->workoutTypes.entries[0], i, 0);
        for (int j = 1; j < 5; ++j) {
            createNewEntry(m->workoutTypes.entries[j], i, e->cumulativeDuration[j - 1]);
        }
    }

    for (int i = refIndices[0]; i < size; ++i) {
        struct WeekDataModel *e = &results[i];

        totalWorkouts[2] += e->totalWorkouts;
        totalWorkouts[1] += e->totalWorkouts;
        totalWorkouts[0] += e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts[2]) maxWorkouts[2] = e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts[1]) maxWorkouts[1] = e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts[0]) maxWorkouts[0] = e->totalWorkouts;
        createNewEntry(m->totalWorkouts.entries, i, e->totalWorkouts);

        for (int j = 0; j < 4; ++j) {
            totalByType[2][j] += e->durationByType[j];
            totalByType[1][j] += e->durationByType[j];
            totalByType[0][j] += e->durationByType[j];

            totalByExercise[2][j] += e->weightArray[j];
            totalByExercise[1][j] += e->weightArray[j];
            totalByExercise[0][j] += e->weightArray[j];
            if (e->weightArray[j] > maxWeight[2]) maxWeight[2] = e->weightArray[j];
            if (e->weightArray[j] > maxWeight[1]) maxWeight[1] = e->weightArray[j];
            if (e->weightArray[j] > maxWeight[0]) maxWeight[0] = e->weightArray[j];
            createNewEntry(m->lifts.entries[j], i, e->weightArray[j]);
        }

        if (e->cumulativeDuration[3] > maxTime[2]) maxTime[2] = e->cumulativeDuration[3];
        if (e->cumulativeDuration[3] > maxTime[1]) maxTime[1] = e->cumulativeDuration[3];
        if (e->cumulativeDuration[3] > maxTime[0]) maxTime[0] = e->cumulativeDuration[3];
        createNewEntry(m->workoutTypes.entries[0], i, 0);
        for (int j = 1; j < 5; ++j) {
            createNewEntry(m->workoutTypes.entries[j], i, e->cumulativeDuration[j - 1]);
        }
    }

    for (int i = 0; i < 3; ++i) {
        m->totalWorkouts.avgs[i] = (float) totalWorkouts[i] / m->nEntries[i];
        m->totalWorkouts.maxes[i] = maxWorkouts[i] < 7 ? 7 : 1.1 * maxWorkouts[i];
        m->workoutTypes.maxes[i] = 1.1 * maxTime[i];
        m->lifts.maxes[i] = 1.1 * maxWeight[i];
        m->totalWorkouts.dataArrays[i] = &m->totalWorkouts.entries->arr[refIndices[i]];
        m->workoutTypes.dataArrays[i][0] = &m->workoutTypes.entries[0]->arr[refIndices[i]];

        for (int j = 0; j < 4; ++j) {
            m->workoutTypes.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = (float) totalByExercise[i][j] / m->nEntries[i];
            m->workoutTypes.dataArrays[i][j + 1] = &m->workoutTypes.entries[j + 1]->arr[refIndices[i]];
            m->lifts.dataArrays[i][j] = &m->lifts.entries[j]->arr[refIndices[i]];
        }
    }
    free(results);
}

void historyCoordinator_clearData(HistoryTabCoordinator *this, bool callVC) {
    HistoryViewModel *model = &this->model;

    memset(model->nEntries, 0, 3 * sizeof(int));
    memset(model->totalWorkouts.dataArrays, 0, 3 * sizeof(id*));
    memset(model->totalWorkouts.avgs, 0, 3 * sizeof(float));
    memset(model->totalWorkouts.maxes, 0, 3 * sizeof(float));
    memset(model->lifts.maxes, 0, 3 * sizeof(float));
    memset(model->workoutTypes.maxes, 0, 3 * sizeof(float));

    for (int i = 0; i < 3; ++i) {
        memset(model->workoutTypes.dataArrays[i], 0, 5 * sizeof(id*));
        memset(model->lifts.dataArrays[i], 0, sizeof(id*) << 2);
        memset(model->workoutTypes.avgs[i], 0, sizeof(int) << 2);
        memset(model->lifts.avgs[i], 0, sizeof(float) << 2);
    }

    array_clear(pt, model->totalWorkouts.entries);
    for (int i = 0; i < 4; ++i) {
        array_clear(pt, model->workoutTypes.entries[i]);
        array_clear(pt, model->lifts.entries[i]);
    }
    array_clear(pt, model->workoutTypes.entries[4]);

    replaceDataSetEntries(model->totalWorkouts.dataSet, model->totalWorkouts.entries->arr, 0);
    for (int i = 0; i < 4; ++i) {
        replaceDataSetEntries(model->workoutTypes.dataSets[i], model->workoutTypes.entries[0]->arr, 0);
        replaceDataSetEntries(model->lifts.dataSets[i], model->lifts.entries[0]->arr, 0);
    }
    replaceDataSetEntries(model->workoutTypes.dataSets[4], model->workoutTypes.entries[0]->arr, 0);

    if (callVC)
        historyVC_refresh(getFirstVC(this->navVC));
}
