#ifndef CollectionVC_h
#define CollectionVC_h

#include "Views.h"

extern CFStringRef UICollectionElementKindSectionHeader;

struct CVCache {
    const struct {
        Class cls;
        SEL ipfi, gs, gi;
        id (*forItem)(Class, SEL, long, long);
        long (*section)(id, SEL);
        long (*item)(id, SEL);
    } path;
    const struct {
        SEL drcwri, diaip, svfek, cfiaip;
        id (*dequeue)(id, SEL, CFStringRef, id);
        void (*deselect)(id, SEL, id, bool);
        id (*supplementary)(id, SEL, CFStringRef, id);
        id (*cell)(id, SEL, id);
    } cv;
};

typedef struct {
    id collectionView;
    CFMutableStringRef *headers;
    CFStringRef **titles;
    CFStringRef **labels;
    int *headerHeights;
    int *itemCounts;
    int **itemHeights;
    int totalSections;
    int firstSection;
    int headerWidth;
    int itemWidth;
} CollectionVC;

extern struct CVCache CVTable;
extern Class CollectionVCClass;

#define makeIndexPath(i, s) CVTable.path.forItem(CVTable.path.cls, CVTable.path.ipfi, i, s)
#define getSection(p) CVTable.path.section((p), CVTable.path.gs)
#define getItem(p) CVTable.path.item((p), CVTable.path.gi)

#define dequeueReusableCell(v, i, p) CVTable.cv.dequeue((v), CVTable.cv.drcwri, i, p)
#define deselectItem(v, p) CVTable.cv.deselect((v), CVTable.cv.diaip, p, false)
#define supplementaryView(v, p)\
 CVTable.cv.supplementary((v), CVTable.cv.svfek, UICollectionElementKindSectionHeader, p)
#define cellForItem(v, p) CVTable.cv.cell((v), CVTable.cv.cfiaip, p)

void initCollectionData(void);

id collectionVC_init(Class Child, CFMutableStringRef *headers, int totalSections, int *itemCounts);
void collectionVC_deinit(id self, SEL _cmd);
void collectionVC_viewDidLoad(id self, SEL _cmd);

void collectionVC_calculateHeights(CollectionVC *d, int section);

long collectionVC_numberOfSectionsInCollectionView(id self, SEL _cmd);
long collectionVC_numberOfItemsInSection(id self, SEL _cmd, id collectionView, long section);
id collectionVC_viewForSupplementaryElement(id self, SEL _cmd,
                                            id collectionView, CFStringRef kind, id indexPath);
CGSize collectionVC_sizeForItem(id self, SEL _cmd, id collectionView, id layout, id indexPath);
HAInsets collectionVC_insetForSection(id self, SEL _cmd,id collectionView, id layout, long section);
CGSize collectionVC_referenceSizeForHeader(id self, SEL _cmd,
                                           id collectionView, id layout, long section);

void collectionVC_viewWillTransitionToSize(id self, SEL _cmd, CGSize size, id coordinator);

static inline void reloadSections(id collectionView, u_long section) {
    id sections = msgV(clsSig(id, u_long), objc_getClass("NSIndexSet"),
                       sel_getUid("indexSetWithIndex:"), section);
    msgV(objSig(void, id), collectionView, sel_getUid("reloadSections:"), sections);
}

#endif /* CollectionVC_h */
