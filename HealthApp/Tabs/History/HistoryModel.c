#include "HistoryModel.h"
#include "BaseMacros.h"

void historyModel_populate(HistoryModel *m,
                           CFArrayRef strs, WeekDataModel *results, int size, bool ltr) {
    m->axisStrings = strs;
    int incr = ltr ? 1 : -1;
    int innerLimits[] = {-1, 0, 1};
    int refIndices[] = {size, size - 26, size - 52, 0};
    refIndices[2] = max(refIndices[2], 0);
    refIndices[1] = max(refIndices[1], 0);

    memcpy(m->nEntries, (int[]){size - refIndices[1], size - refIndices[2], size}, 3 * sizeof(int));
    if (ltr) memcpy(m->refIndices, &refIndices[1], 3 * sizeof(int));

    int totalWorkouts[3] = {0};
    int totalByType[3][4] = {{0},{0},{0}}, totalByExercise[3][4] = {{0},{0},{0}};
    short maxWorkouts[3] = {0}, maxTime[3] = {0}, maxWeight[3] = {0};
    m->tw.entries = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[0] = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[1] = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[2] = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[3] = malloc((unsigned)size * sizeof(CGPoint));
    m->wt.entries[0] = malloc((unsigned)size * sizeof(CGPoint));
    m->wt.entries[1] = malloc((unsigned)size * sizeof(CGPoint));
    m->wt.entries[2] = malloc((unsigned)size * sizeof(CGPoint));
    m->wt.entries[3] = malloc((unsigned)size * sizeof(CGPoint));
    m->wt.entries[4] = malloc((unsigned)size * sizeof(CGPoint));

    for (int section = 3, index = ltr ? 0 : size - 1; section > 0; --section) {
        int limit = refIndices[section - 1];
        int jEnd = innerLimits[section - 1];
        for (int i = refIndices[section]; i < limit; ++i, index += incr) {
            WeekDataModel *e = &results[i];

            for (int j = 2; j > jEnd; --j) {
                totalWorkouts[j] += e->totalWorkouts;
                maxWorkouts[j] = max(maxWorkouts[j], e->totalWorkouts);
            }
            m->tw.entries[index] = (CGPoint){index, e->totalWorkouts};

            for (int x = 0; x < 4; ++x) {
                for (int j = 2; j > jEnd; --j) {
                    totalByType[j][x] += e->durationByType[x];
                    totalByExercise[j][x] += e->weightArray[x];
                    maxWeight[j] = max(maxWeight[j], e->weightArray[x]);
                }
                m->lifts.entries[x][index] = (CGPoint){index, e->weightArray[x]};
            }

            for (int j = 2; j > jEnd; --j) {
                maxTime[j] = max(maxTime[j], e->cumulativeDuration[3]);
            }
            m->wt.entries[0][index] = (CGPoint){index, 0};
            for (int x = 1; x < 5; ++x) {
                m->wt.entries[x][index] = (CGPoint){index, e->cumulativeDuration[x - 1]};
            }
        }
    }

    float invEntries[] = {1.f / m->nEntries[0], 1.f / m->nEntries[1], 1.f / m->nEntries[2]};
    for (int i = 0; i < 3; ++i) {
        m->tw.avgs[i] = totalWorkouts[i] * invEntries[i];
        m->maxes[ChartTW][i] = maxWorkouts[i] < 7 ? 7 : 1.1f * maxWorkouts[i];
        m->maxes[ChartWT][i] = 1.1f * maxTime[i];
        m->maxes[ChartLifts][i] = maxWeight[i] * 1.1f;

        for (int j = 0; j < 4; ++j) {
            m->wt.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = totalByExercise[i][j] * invEntries[i];
        }
    }
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (CFBooleanGetValue(CFLocaleGetValue(locale, kCFLocaleUsesMetricSystem))) {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < 4; ++j) m->lifts.entries[j][i].y *= ToKg;
        }
        for (int i = 0; i < 3; ++i) {
            m->maxes[ChartLifts][i] *= ToKg;
            for (int j = 0; j < 4; ++j) m->lifts.avgs[i][j] *= ToKg;
        }
    }
    CFRelease(locale);
    free(results);
}
