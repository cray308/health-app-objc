#ifndef StatusCell_h
#define StatusCell_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

struct CellCache {
    size_t classSize;
    SEL gbv, gsbv;
    id (*getBG)(id, SEL);
    id (*getSelected)(id, SEL);
};

typedef struct {
    id button;
    id header;
    id box;
} StatusCell;

extern struct CellCache CellTable;
extern Class Cell;
extern Class RootCell;
extern Class BasicCell;
extern Class FullCell;
extern CFStringRef const BasicCellID;
extern CFStringRef const FullCellID;

#define getIVC(x) ((StatusCell *)((char *)(x) + CellTable.classSize))

#define getBackgroundView(c) CellTable.getBG((c), CellTable.gbv)
#define getSelectedBackgroundView(c) CellTable.getSelected((c), CellTable.gsbv)

void initCellData(void);

id rootCell_initWithFrame(id self, SEL _cmd, CGRect frame);
void rootCell_deinit(id self, SEL _cmd);
id basicCell_initWithFrame(id self, SEL _cmd, CGRect frame);
id fullCell_initWithFrame(id self, SEL _cmd, CGRect frame);

void statusCell_updateAccessibility(StatusCell *v);

#endif /* StatusCell_h */
