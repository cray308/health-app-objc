#ifndef ColorCache_h
#define ColorCache_h

#include <objc/objc.h>

typedef struct {
    Class cls;
    SEL sc;
    id (*getColor)(Class,SEL,int);
} ColorCache;

typedef ColorCache const *CCacheRef;

#endif /* ColorCache_h */
