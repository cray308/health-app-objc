#include "HistoryTabCoordinator.h"
#include "AppUserData.h"
#include "ExerciseManager.h"
#include "HistoryVC.h"
#include "PersistenceService.h"
#include "ViewControllerHelpers.h"

gen_array_source(weekData, HistoryWeekDataModel, DSDefault_shallowCopy, DSDefault_shallowDelete)

extern id createDataSet(int color, int lineWidth, uint8_t options, id fillSet);
extern id createChartData(id *dataSets, int count, uint8_t options);
extern void setLegendLabel(id entry, CFStringRef text);
extern void setupLegendEntries(id *entries, int *colors, int count);

void historyCoordinator_start(HistoryTabCoordinator *this) {
    HistoryViewModel *model = &this->model;
    int chartColors[] = {0, 1, 2, 3};
    id *areaDataSets = model->workoutTypeModel.dataSets;
    setupLegendEntries(model->totalWorkoutsModel.legendEntries, (int []){4}, 1);
    setupLegendEntries(model->workoutTypeModel.legendEntries, chartColors, 4);
    setupLegendEntries(model->liftModel.legendEntries, chartColors, 4);

    model->totalWorkoutsModel.entries = array_new(object);
    model->totalWorkoutsModel.dataSet = createDataSet(5, 1, 3, nil);
    model->workoutTypeModel.dataSets[0] = createDataSet(-1, 1, 0, nil);
    fillStringArray(model->workoutTypeModel.names, CFSTR("workoutTypes%d"), 4);
    fillStringArray(model->liftModel.names, CFSTR("liftTypes%d"), 4);
    fillStringArray(model->formatter.months, CFSTR("months%02d"), 12);
    model->formatter.currString = CFStringCreateCopy(NULL, CFSTR(""));
    model->totalWorkoutsModel.legendFormat = localize(CFSTR("totalWorkoutsLegend"));
    model->liftModel.legendFormat = localize(CFSTR("liftLegend"));
    model->workoutTypeModel.legendFormat = localize(CFSTR("workoutTypeLegend"));

    for (int i = 0; i < 4; ++i) {
        model->workoutTypeModel.entries[i] = array_new(object);
        model->liftModel.entries[i] = array_new(object);

        id boundary = model->workoutTypeModel.dataSets[i];
        model->workoutTypeModel.dataSets[i + 1] = createDataSet(chartColors[i], 1, 1, boundary);
        model->liftModel.dataSets[i] = createDataSet(chartColors[i], 3, 0, nil);
    }
    model->workoutTypeModel.entries[4] = array_new(object);

    model->totalWorkoutsModel.chartData = createChartData((id []){
        model->totalWorkoutsModel.dataSet
    }, 1, 0);
    model->workoutTypeModel.chartData = createChartData((id []){
        areaDataSets[4], areaDataSets[3], areaDataSets[2], areaDataSets[1]
    }, 4, 1);
    model->liftModel.chartData = createChartData(model->liftModel.dataSets, 4, 0);

    model->data = array_new(weekData);
    array_reserve(weekData, model->data, 128);
    setupNavVC(this->navVC, historyVC_init(this));
}

void historyCoordinator_fetchData(HistoryTabCoordinator *this) {
    HistoryViewModel *model = &this->model;
    runInBackground((^{
        array_clear(weekData, model->data);
        struct tm localInfo;
        int count = 0;

        id request = fetchRequest(createPredicate(CFSTR("weekStart < %lld"), userData->weekStart));
        CFArrayRef data = persistenceService_executeFetchRequest(request, &count, true);
        if (data) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                int timeStrength = weekData_getWorkoutTimeForType(d, WorkoutStrength);
                time_t timestamp = weekData_getWeekStart(d);
                localtime_r(&timestamp, &localInfo);
                HistoryWeekDataModel m = {
                    .year = localInfo.tm_year % 100,
                    .month = localInfo.tm_mon,
                    .day = localInfo.tm_mday,
                    .totalWorkouts = weekData_getTotalWorkouts(d),
                    .durationByType = {
                        timeStrength,
                        weekData_getWorkoutTimeForType(d, WorkoutHIC),
                        weekData_getWorkoutTimeForType(d, WorkoutSE),
                        weekData_getWorkoutTimeForType(d, WorkoutEndurance)
                    },
                    .cumulativeDuration = {[0] = timeStrength}
                };

                weekData_getLiftingLimits(d, m.weightArray);
                for (int j = 1; j < 4; ++j)
                    m.cumulativeDuration[j] = m.cumulativeDuration[j - 1] + m.durationByType[j];
                array_push_back(weekData, model->data, m);
            }
        }
    }));
}
