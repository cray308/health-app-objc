#ifndef BaseMacros_h
#define BaseMacros_h

#if DEBUG
#include <assert.h>
#define customAssert(x) assert(x);
#else
#define customAssert(x)
#endif

#define _U_ __attribute__((__unused__))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define ToKg 0.453592f
#define ToAppleTime(x) ((x) - (long)kCFAbsoluteTimeIntervalSince1970)

#define formatStr(l, fmt, ...) CFStringCreateWithFormat(NULL, (CFDictionaryRef)(l), fmt, ##__VA_ARGS__)
#define localize(s) CFCopyLocalizedString(s, )

#endif /* BaseMacros_h */
