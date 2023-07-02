#ifndef StringUtils_h
#define StringUtils_h

#include <CoreFoundation/CoreFoundation.h>
#include "BaseMacros.h"

#define getOneStr(l) formatStr((l), CFSTR("%d"), 1)

void fillStringArray(CFStringRef *arr, CFStringRef format, int count);
CFRange findNumber(CFStringRef s, CFLocaleRef locale, CFStringRef num, CFStringRef substr);

static inline void updateRange(CFMutableStringRef s, CFRange *range,
                               CFStringRef replacement CF_CONSUMED, CFLocaleRef locale CF_CONSUMED) {
    CFStringReplace(s, *range, replacement);
    range->length = CFStringGetLength(replacement);
    CFRelease(replacement);
    CFRelease(locale);
}

#endif /* StringUtils_h */
