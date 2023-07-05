#ifndef StatusCell_h
#define StatusCell_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

typedef struct {
    id button;
    id header;
    id box;
} StatusCell;

extern Class Cell;
extern Class RootCell;
extern Class BasicCell;
extern Class FullCell;
extern size_t CellSize;
extern CFStringRef const BasicCellID;
extern CFStringRef const FullCellID;

#define getIVC(x) ((StatusCell *)((char *)(x) + CellSize))

void initCellData(void);

id rootCell_initWithFrame(id self, SEL _cmd, CGRect frame);
void rootCell_deinit(id self, SEL _cmd);
id basicCell_initWithFrame(id self, SEL _cmd, CGRect frame);
id fullCell_initWithFrame(id self, SEL _cmd, CGRect frame);

void statusCell_updateAccessibility(StatusCell *v);

#endif /* StatusCell_h */
