#ifndef BaseMacros_h
#define BaseMacros_h

#if DEBUG
#include <assert.h>
#define customAssert(x) assert(x);
#else
#define customAssert(x)
#endif

#define _U_ __attribute__((__unused__))

#define ToKg 0.453592f
#define WeekSeconds 604800
#define DaySeconds 86400
#define HourSeconds 3600
#define MaxValidChar 1

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define isCharValid(v) (v <= MaxValidChar)
#define formatStr(l, fmt, ...)\
 CFStringCreateWithFormat(NULL, (CFDictionaryRef)(l), fmt, ##__VA_ARGS__)
#define formatDate(f, d) CFDateFormatterCreateStringWithAbsoluteTime(\
 NULL, (f), ((d) - (long)kCFAbsoluteTimeIntervalSince1970))
#define localize(s) CFCopyLocalizedString(s, )
#define isMetric(l) CFBooleanGetValue(CFLocaleGetValue((l), kCFLocaleUsesMetricSystem))
#define getSavedMassFactor(l) (isMetric(l) ? 2.204623f : 1)
#define getDictValue(d, k, t, r) CFNumberGetValue(CFDictionaryGetValue((d), (k)), (t), (r))

#endif /* BaseMacros_h */
