#include "DatabaseManager.h"
#include <sqlite3.h>
#include "BaseMacros.h"

#define LastWeekQuery "SELECT * FROM weeks ORDER BY uid DESC LIMIT 1;"

enum {
    ColumnID,
    ColumnWeekStart,
    ColumnTotalWorkouts,
    ColumnLifts,
    ColumnDuration = 7
};

static void openDatabase(sqlite3 **db) {
    CFURLRef homeUrl = CFCopyHomeDirectoryURL();
    CFURLRef docUrl = CFURLCreateCopyAppendingPathComponent(NULL, homeUrl,
                                                            CFSTR("Documents/HAData.db"), false);
    char dbPath[256];
    CFURLGetFileSystemRepresentation(docUrl, true, (UInt8 *)dbPath, 256);
    sqlite3_open(dbPath, db);
    CFRelease(homeUrl);
    CFRelease(docUrl);
}

static void bindArray(sqlite3_stmt *statement, int const *values, int index) {
    for (int i = 0; i < 4; ++i) {
        sqlite3_bind_int(statement, index + i, values[i]);
    }
}

static void getArray(sqlite3_stmt *select, int *results, int index) {
    for (int i = 0; i < 4; ++i) {
        results[i] = sqlite3_column_int(select, index + i);
    }
}

static void executeStatement(sqlite3 *db, char const *command, int valueToBind) {
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, command, -1, &statement, NULL);
    sqlite3_bind_int(statement, 1, valueToBind);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

#if DEBUG
static void createDummyData(sqlite3 *db) {
    int lifts[] = {300, 20, 185, 235}, i = 0, year = 0, plan = 0;
    time_t endpts[] = {time(NULL) - 126489600, time(NULL) - 2678400};
    for (int x = 0; x < 2; ++x) {
        time_t date = endpts[x];
        struct tm tmInfo;
        localtime_r(&date, &tmInfo);
        if (tmInfo.tm_wday != 1) {
            date = date - WeekSeconds + (((8 - tmInfo.tm_wday) % 7) * DaySeconds);
            localtime_r(&date, &tmInfo);
        }
        endpts[x] = date - (tmInfo.tm_hour * HourSeconds) - (tmInfo.tm_min * 60) - tmInfo.tm_sec;
    }

    sqlite3_stmt *insert;
    sqlite3_prepare_v2(db,
                       "INSERT INTO weeks (start, total_workouts,"
                       "best_squat, best_pullup, best_bench, best_deadlift,"
                       "time_strength, time_se, time_endurance, time_hic"
                       ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);", -1, &insert, NULL);

    while (endpts[0] < endpts[1]) {
        int totalWorkouts = 0, times[4] = {0}, extraTime = 0, extraWorkouts = 1;
        if (!year || i < 24) {
            extraTime = rand() % 40;
            extraWorkouts += rand() % 5;
        }

        if (plan == 0) {
            for (int j = 0; j < 6; ++j) {
                int extra = 10;
                bool didSE = true;
                switch (j) {
                    case 1:
                    case 2:
                    case 5:
                        times[2] += (rand() % 30) + 30 + extraTime;
                        totalWorkouts += extraWorkouts;
                        break;
                    case 4:
                        if ((didSE = ((rand() % 10) >= 5))) extra = 0;
                    case 0:
                    case 3:
                        if (didSE) {
                            times[1] += (rand() % 20) + extra + extraTime;
                            totalWorkouts += extraWorkouts;
                        }
                    default:
                        break;
                }
            }
        } else {
            for (int j = 0; j < 6; ++j) {
                switch (j) {
                    case 0:
                    case 2:
                    case 4:
                        times[0] += (rand() % 20) + 20 + extraTime;
                        totalWorkouts += extraWorkouts;
                        break;
                    case 1:
                    case 3:
                        times[3] += (rand() % 20) + 15 + extraTime;
                        totalWorkouts += extraWorkouts;
                        break;
                    case 5:
                        times[2] += (rand() % 30) + 60 + extraTime;
                        totalWorkouts += extraWorkouts;
                    default:
                        break;
                }
            }
        }

        if (i == 7) {
            plan = 1;
        } else if (i >= 20 && !((i - 20) % 12)) {
            lifts[0] = 300 + (rand() % 80);
            lifts[1] = 20 + (rand() % 20);
            lifts[2] = 185 + (rand() % 50);
            lifts[3] = 235 + (rand() % 50);
        }

        sqlite3_bind_int64(insert, 1, endpts[0]);
        sqlite3_bind_int(insert, 2, totalWorkouts);
        bindArray(insert, lifts, 3);
        bindArray(insert, times, 7);
        sqlite3_step(insert);

        if (++i == 52) {
            i = 0;
            plan = 0;
            year += 1;
        }
        endpts[0] += WeekSeconds;
        sqlite3_reset(insert);
    }
    sqlite3_finalize(insert);
}
#define setupDatabase(p) createDummyData(p);
#else
#define setupDatabase(p)
#endif

void createDB(void) {
    sqlite3 *db;
    openDatabase(&db);
    sqlite3_exec(db,
                 "CREATE TABLE weeks("
                 "uid INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                 "start BIGINT NOT NULL,"
                 "total_workouts DEFAULT 0,"
                 "best_squat NOT NULL,"
                 "best_pullup NOT NULL,"
                 "best_bench NOT NULL,"
                 "best_deadlift NOT NULL,"
                 "time_strength DEFAULT 0,"
                 "time_se DEFAULT 0,"
                 "time_endurance DEFAULT 0,"
                 "time_hic DEFAULT 0);", NULL, NULL, NULL);
    setupDatabase(db)
    sqlite3_close(db);
}

static void fetchHistory(sqlite3 *db, HistoryModel *historyModel) {
    int count = 0;
    WeeklyData *weeks = malloc(105 * sizeof(WeeklyData));
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFDateFormatterRef formatter = CFDateFormatterCreate(NULL, locale, kCFDateFormatterShortStyle, 0);
    CFRelease(locale);
    CFMutableArrayRef axisStrs = CFArrayCreateMutable(NULL, 105, &kCFTypeArrayCallBacks);
    sqlite3_stmt *select;
    sqlite3_prepare_v2(db, "SELECT * FROM weeks ORDER BY uid;", -1, &select, NULL);
    while (sqlite3_step(select) == SQLITE_ROW) {
        WeeklyData *w = &weeks[count++];

        CFStringRef date = formatDate(formatter, sqlite3_column_int64(select, ColumnWeekStart));
        CFArrayAppendValue(axisStrs, date);
        CFRelease(date);

        w->totalWorkouts = sqlite3_column_int(select, ColumnTotalWorkouts);
        getArray(select, w->weights, ColumnLifts);
        getArray(select, w->durationByType, ColumnDuration);

        w->cumulativeDuration[0] = w->durationByType[0];
        for (int j = 1; j < 4; ++j) {
            w->cumulativeDuration[j] = w->cumulativeDuration[j - 1] + w->durationByType[j];
        }
    }
    sqlite3_finalize(select);
    CFRelease(formatter);

    if (--count) {
        historyModel_populate(historyModel, axisStrs, weeks, count);
    } else {
        CFRelease(axisStrs);
        free(weeks);
    }
}

void runStartupJob(HistoryModel *historyModel, time_t weekStart, int tzDiff) {
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{
        int lifts[4] = {0};
        sqlite3 *db;
        openDatabase(&db);
        sqlite3_stmt *select, *insert;
        sqlite3_prepare_v2(db,
                           "INSERT INTO weeks ("
                           "start, best_squat, best_pullup, best_bench, best_deadlift"
                           ") VALUES (?, ?, ?, ?, ?);", -1, &insert, NULL);
        sqlite3_prepare_v2(db, LastWeekQuery, -1, &select, NULL);
        if (sqlite3_step(select) != SQLITE_ROW) {
            sqlite3_finalize(select);
            sqlite3_bind_int64(insert, 1, weekStart);
            bindArray(insert, lifts, 2);
            sqlite3_step(insert);
            sqlite3_finalize(insert);
            sqlite3_close(db);
            return;
        }

        time_t start = sqlite3_column_int64(select, ColumnWeekStart);
        getArray(select, lifts, ColumnLifts);
        bindArray(insert, lifts, 2);
        sqlite3_finalize(select);

        if (tzDiff) {
            executeStatement(db, "UPDATE weeks SET start = start + ?;", tzDiff);
            start += tzDiff;
        }

        sqlite3_stmt *delete;
        sqlite3_prepare_v2(db, "DELETE FROM weeks WHERE start < ?;", -1, &delete, NULL);
        sqlite3_bind_int64(delete, 1, weekStart - (WeekSeconds * 104) - (DaySeconds << 2));
        sqlite3_step(delete);
        sqlite3_finalize(delete);

        bool addForThisWeek = start != weekStart;
        time_t lastDate = weekStart - HourSeconds;

        for (start = start + WeekSeconds; start < lastDate; start += WeekSeconds) {
            sqlite3_bind_int64(insert, 1, start);
            sqlite3_step(insert);
            sqlite3_reset(insert);
        }

        if (addForThisWeek) {
            sqlite3_bind_int64(insert, 1, weekStart);
            sqlite3_step(insert);
        }

        sqlite3_finalize(insert);
        fetchHistory(db, historyModel);
        sqlite3_close(db);
    });
}

void deleteStoredData(void) {
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{
        sqlite3 *db;
        openDatabase(&db);
        sqlite3_stmt *select;
        sqlite3_prepare_v2(db, LastWeekQuery, -1, &select, NULL);
        sqlite3_step(select);
        int rowId = sqlite3_column_int(select, ColumnID);
        sqlite3_finalize(select);

        executeStatement(db, "DELETE FROM weeks WHERE uid < ?;", rowId);
        sqlite3_exec(db,
                     "UPDATE weeks SET total_workouts = 0,"
                     "time_strength = 0, time_se = 0, time_endurance = 0, time_hic = 0;",
                     NULL, NULL, NULL);
        sqlite3_close(db);
    });
}

void saveWorkoutData(int duration, uint8_t type, int *weights) {
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{
        sqlite3 *db;
        openDatabase(&db);
        sqlite3_stmt *select, *update;
        sqlite3_prepare_v2(db, LastWeekQuery, -1, &select, NULL);
        sqlite3_step(select);
        int rowId = sqlite3_column_int(select, ColumnID), lifts[4];
        getArray(select, lifts, ColumnLifts);
        if (weights) {
            memcpy(lifts, weights, sizeof(int) << 2);
            free(weights);
        }
        sqlite3_finalize(select);

        char *timeName = (char *[]){"time_strength", "time_se", "time_endurance", "time_hic"}[type];
        char buf[256];
        snprintf(buf, 256,
                 "UPDATE weeks SET total_workouts = total_workouts + 1, %s = %s + ?,"
                 "best_squat = ?, best_pullup = ?, best_bench = ?, best_deadlift = ? WHERE uid = ?;",
                 timeName, timeName);
        sqlite3_prepare_v2(db, buf, -1, &update, NULL);
        sqlite3_bind_int(update, 1, duration);
        bindArray(update, lifts, 2);
        sqlite3_bind_int(update, 6, rowId);
        sqlite3_step(update);
        sqlite3_finalize(update);
        sqlite3_close(db);
    });
}
