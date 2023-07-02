#include "StringUtils.h"

void fillStringArray(CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = formatStr(NULL, format, i);
        arr[i] = localize(key);
        CFRelease(key);
    }
}

CFRange findNumber(CFStringRef s, CFLocaleRef locale, CFStringRef num, CFStringRef substr) {
    if (!substr) {
        CFRange r;
        CFStringFindWithOptionsAndLocale(s, num, (CFRange){0, CFStringGetLength(s)}, 0, locale, &r);
        return r;
    }

    CFRange subrange = CFStringFind(s, substr, 0);
    CFStringRef sub = CFStringCreateWithSubstring(NULL, s, subrange);
    CFRange findRange = {0, CFStringGetLength(sub)}, numrange;
    CFStringFindWithOptionsAndLocale(sub, num, findRange, 0, locale, &numrange);
    CFRelease(sub);
    return (CFRange){subrange.location + numrange.location, numrange.length};
}
