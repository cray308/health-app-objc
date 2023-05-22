#ifndef BaseMacros_h
#define BaseMacros_h

#if DEBUG
#include <assert.h>
#define customAssert(x) assert(x);
extern void NSLog(id format, ...);
#else
#define customAssert(x)
#endif

#define _U_ __attribute__((__unused__))

#define ToKg 0.453592f
#define WeekSeconds 604800

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define formatStr(l, fmt, ...)\
 CFStringCreateWithFormat(NULL, (CFDictionaryRef)(l), fmt, ##__VA_ARGS__)
#define localize(s) CFCopyLocalizedString(s, )
#define isMetric(l) CFBooleanGetValue(CFLocaleGetValue((l), kCFLocaleUsesMetricSystem))
#define getSavedMassFactor(l) (isMetric(l) ? 2.204623f : 1)

#endif /* BaseMacros_h */
