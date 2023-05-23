#include "HistoryModel.h"
#include "BaseMacros.h"

void historyModel_populate(HistoryModel *m,
                           CFMutableArrayRef axisStrs, WeeklyData *weeks, int size) {
    int refIndices[] = {size, size - 26, size - 52, 0};
    refIndices[2] = max(refIndices[2], 0);
    refIndices[1] = max(refIndices[1], 0);
    memcpy(m->nEntries, (int []){size - refIndices[1], size - refIndices[2], size}, 3 * sizeof(int));

    CFLocaleRef locale = CFLocaleCopyCurrent();
    bool ltr = CFLocaleGetLanguageCharacterDirection(CFLocaleGetValue(locale, kCFLocaleLanguageCode))
               == kCFLocaleLanguageDirectionLeftToRight;
    if (ltr) {
        memcpy(m->refIndices, &refIndices[1], 3 * sizeof(int));
    } else {
        for (int i = 0, j = size - 1; i < j; ++i, --j) {
            CFArrayExchangeValuesAtIndices(axisStrs, i, j);
        }
    }

    m->axisStrs = axisStrs;
    m->totals.entries = malloc((unsigned)size * sizeof(CGPoint));
    for (int i = 0; i < 4; ++i) {
        m->lifts.entries[i] = malloc((unsigned)size * sizeof(CGPoint));
        m->types.entries[i] = malloc((unsigned)size * sizeof(CGPoint));
    }
    m->types.entries[4] = malloc((unsigned)size * sizeof(CGPoint));

    int increment = ltr ? 1 : -1, innerLimits[] = {-1, 0, 1};
    int totalWorkouts[3] = {0}, totalByType[3][4] = {0}, totalByExercise[3][4] = {0};
    int maxWorkouts[3] = {0}, maxTime[3] = {0}, maxWeight[3] = {0};

    for (int section = 3, entryIdx = ltr ? 0 : size - 1; section > 0; --section) {
        int jEnd = innerLimits[section - 1];
        WeeklyData *wEnd = &weeks[refIndices[section - 1]];
        for (WeeklyData *w = &weeks[refIndices[section]]; w < wEnd; ++w, entryIdx += increment) {
            for (int j = 2; j > jEnd; --j) {
                totalWorkouts[j] += w->totalWorkouts;
                maxWorkouts[j] = max(maxWorkouts[j], w->totalWorkouts);
                maxTime[j] = max(maxTime[j], w->cumulativeDuration[3]);
            }
            m->totals.entries[entryIdx] = (CGPoint){entryIdx, w->totalWorkouts};

            for (int x = 0; x < 4; ++x) {
                for (int j = 2; j > jEnd; --j) {
                    totalByType[j][x] += w->durationByType[x];
                    totalByExercise[j][x] += w->weights[x];
                    maxWeight[j] = max(maxWeight[j], w->weights[x]);
                }
                m->lifts.entries[x][entryIdx] = (CGPoint){entryIdx, w->weights[x]};
            }

            m->types.entries[0][entryIdx] = (CGPoint){entryIdx, 0};
            for (int x = 1; x < 5; ++x) {
                m->types.entries[x][entryIdx] = (CGPoint){entryIdx, w->cumulativeDuration[x - 1]};
            }
        }
    }

    float inverseEntries[] = {1.f / m->nEntries[0], 1.f / m->nEntries[1], 1.f / m->nEntries[2]};
    for (int i = 0; i < 3; ++i) {
        m->totals.avgs[i] = totalWorkouts[i] * inverseEntries[i];
        m->maxes[ChartTotals][i] = maxWorkouts[i] < 7 ? 7 : 1.1f * maxWorkouts[i];
        m->maxes[ChartTypes][i] = 1.1f * maxTime[i];
        m->maxes[ChartLifts][i] = 1.1f * maxWeight[i];

        for (int j = 0; j < 4; ++j) {
            m->types.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = totalByExercise[i][j] * inverseEntries[i];
        }
    }

    if (isMetric(locale)) {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < 4; ++j) {
                m->lifts.entries[j][i].y *= ToKg;
            }
        }
        for (int i = 0; i < 3; ++i) {
            m->maxes[ChartLifts][i] *= ToKg;
            for (int j = 0; j < 4; ++j) {
                m->lifts.avgs[i][j] *= ToKg;
            }
        }
    }
    CFRelease(locale);
    free(weeks);
}
