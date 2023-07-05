#ifndef HeaderView_h
#define HeaderView_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

typedef struct {
    id divider;
    id label;
} HeaderView;

extern Class ReusableView;
extern Class HeaderViewClass;
extern size_t ReusableViewSize;

#define getIVR(x) ((HeaderView *)((char *)(x) + ReusableViewSize))

id headerView_initWithFrame(id self, SEL _cmd, CGRect frame);
void headerView_deinit(id self, SEL _cmd);

#endif /* HeaderView_h */
