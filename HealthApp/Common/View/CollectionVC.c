#include "CollectionVC.h"
#include "HeaderView.h"
#include "StatusCell.h"
#include "Views.h"

#define invalidateLayout(c)                                                      \
 msgV(objSig(void), (msgV(objSig(id), (c), sel_getUid("collectionViewLayout"))), \
      sel_getUid("invalidateLayout"))

extern CFStringRef NSFontAttributeName;
extern CFStringRef UIContentSizeCategoryDidChangeNotification;

struct CVCache CVTable;
Class CollectionVCClass;

enum {
    ApplicationStateBackground = 2
};

enum {
    SizeClassRegular = 2
};

enum {
    NSStringDrawingUsesLineFragmentOrigin = 1
};

static CFStringRef const HeaderID = CFSTR("HeaderView");
static struct PrivCVData {
    const struct {
        Class cls;
        SEL rc, drsvok;
        void (*registerClass)(id, SEL, Class, CFStringRef);
        id (*dequeueReusableSupplementaryView)(id, SEL, CFStringRef, CFStringRef, id);
    } cv;
    const struct {
        SEL brws;
        generateRectFunctionSignature(boundingRectWithSize, CGSize, long, CFDictionaryRef, id);
    } string;
} pcvc;

#define getBoundingRect(r, o, s, a)                                      \
 callRectMethod(pcvc.string.boundingRectWithSize, r, o, pcvc.string.brws,\
                s, NSStringDrawingUsesLineFragmentOrigin, a, nil)

static void contentSizeChanged(CFNotificationCenterRef, void *,
                               CFStringRef, const void *, CFDictionaryRef);

void initCollectionData(void) {
    Class Path = objc_getClass("NSIndexPath");
    SEL pipfi = sel_getUid("indexPathForItem:inSection:");
    SEL pgs = sel_getUid("section"), pgi = sel_getUid("item");

    Class Collection = objc_getClass("UICollectionView");
    SEL cdrcwri = sel_getUid("dequeueReusableCellWithReuseIdentifier:forIndexPath:");
    SEL cdiaip = sel_getUid("deselectItemAtIndexPath:animated:");
    SEL csvfek = sel_getUid("supplementaryViewForElementKind:atIndexPath:");
    SEL ccfiaip = sel_getUid("cellForItemAtIndexPath:");
    SEL crc = sel_getUid("registerClass:forCellWithReuseIdentifier:");
    SEL cdrsvok = sel_getUid("dequeueReusableSupplementaryViewOfKind:"
                             "withReuseIdentifier:forIndexPath:");

    Class String = objc_getClass("NSString");
    SEL sbrws = sel_getUid("boundingRectWithSize:options:attributes:context:");

    memcpy(&CVTable, &(struct CVCache){
        {
            Path, pipfi, pgs, pgi,
            (id(*)(Class, SEL, long, long))getClassMethodImp(Path, pipfi),
            (long(*)(id, SEL))class_getMethodImplementation(Path, pgs),
            (long(*)(id, SEL))class_getMethodImplementation(Path, pgi)
        },
        {
            cdrcwri, cdiaip, csvfek, ccfiaip,
            (id(*)(id, SEL, CFStringRef, id))class_getMethodImplementation(Collection, cdrcwri),
            (void(*)(id, SEL, id, bool))class_getMethodImplementation(Collection, cdiaip),
            (id(*)(id, SEL, CFStringRef, id))class_getMethodImplementation(Collection, csvfek),
            (id(*)(id, SEL, id))class_getMethodImplementation(Collection, ccfiaip)
        }
    }, sizeof(struct CVCache));
    memcpy(&pcvc, &(struct PrivCVData){
        {
            Collection, crc, cdrsvok,
            (void(*)(id, SEL, Class, CFStringRef))class_getMethodImplementation(Collection, crc),
            ((id(*)(id, SEL, CFStringRef, CFStringRef, id))
             class_getMethodImplementation(Collection, cdrsvok))
        },
        {sbrws, getRectMethodImplementation(String, sbrws, CGSize, long, CFDictionaryRef, id)}
    }, sizeof(struct PrivCVData));
}

#pragma mark - Lifecycle

id collectionVC_init(Class Child, CFMutableStringRef *headers, int totalSections, int *itemCounts) {
    id self = new(Child);
    CollectionVC *d = getIVVC(CollectionVC, self);
    d->totalSections = totalSections;
    d->headers = headers;
    d->headerHeights = malloc((unsigned)totalSections * sizeof(int));
    d->itemCounts = itemCounts;
    d->itemHeights = malloc((unsigned)totalSections * sizeof(int *));
    d->titles = malloc((unsigned)totalSections * sizeof(CFStringRef *));
    d->labels = malloc((unsigned)totalSections * sizeof(CFStringRef *));

    for (int i = 0; i < totalSections; ++i) {
        unsigned items = (unsigned)itemCounts[i];
        d->itemHeights[i] = calloc((unsigned)items, sizeof(int));
        d->titles[i] = calloc((unsigned)items, sizeof(CFStringRef));
        d->labels[i] = calloc((unsigned)items, sizeof(CFStringRef));
    }
    return self;
}

void collectionVC_deinit(id self, SEL _cmd) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    for (int i = 0; i < d->totalSections; ++i) {
        int items = d->itemCounts[i];
        for (int j = 0; j < items; ++j) {
            if (d->titles[i][j]) CFRelease(d->titles[i][j]);
            if (d->labels[i][j]) CFRelease(d->labels[i][j]);
        }
        free(d->titles[i]);
        free(d->labels[i]);
        free(d->itemHeights[i]);
        if (d->headers[i]) CFRelease(d->headers[i]);
    }
    free(d->labels);
    free(d->titles);
    free(d->itemHeights);
    free(d->itemCounts);
    free(d->headerHeights);
    free(d->headers);
    releaseView(d->collectionView);
    msgSupV(supSig(void), self, VC, _cmd);
}

static void updateItemWidth(id self, CollectionVC *d, CGSize availableSize) {
    d->itemWidth = (int)availableSize.width;
    d->headerWidth = d->itemWidth - 16;
    id traitCollection = msgV(objSig(id), self, sel_getUid("traitCollection"));
    if (msgV(objSig(long), traitCollection, sel_getUid("horizontalSizeClass")) == SizeClassRegular)
        d->itemWidth /= 2;
    d->itemWidth -= 16;
}

void collectionVC_viewDidLoad(id self, SEL _cmd) {
    msgSupV(supSig(void), self, VC, _cmd);

    CollectionVC *d = getIVVC(CollectionVC, self);
    id view = getView(self);
    CGRect bounds;
    getBounds(bounds, view);
    updateItemWidth(self, d, bounds.size);

    id layout = new(objc_getClass("UICollectionViewFlowLayout"));
    msgV(objSig(void, CGFloat), layout, sel_getUid("setMinimumInteritemSpacing:"), 8);
    msgV(objSig(void, CGFloat), layout, sel_getUid("setMinimumLineSpacing:"), 8);

    d->collectionView = msgV(objSig(id, CGRect, id), alloc(pcvc.cv.cls),
                             sel_getUid("initWithFrame:collectionViewLayout:"), (CGRect){0}, layout);
    useConstraints(d->collectionView);
    setBackgroundColor(d->collectionView, getColor(ColorPrimaryBGGrouped));
    msgV(objSig(void, Class, CFStringRef, CFStringRef), d->collectionView,
         sel_getUid("registerClass:forSupplementaryViewOfKind:withReuseIdentifier:"),
         HeaderViewClass, UICollectionElementKindSectionHeader, HeaderID);
    pcvc.cv.registerClass(d->collectionView, pcvc.cv.rc, BasicCell, BasicCellID);
    pcvc.cv.registerClass(d->collectionView, pcvc.cv.rc, FullCell, FullCellID);

    addSubview(view, d->collectionView);
    pinToSafeArea(d->collectionView, view);
    releaseObject(layout);

    setIDFormatted(d->collectionView, CFSTR("collection_%@"), CFAutorelease(
      CFStringCreateWithCString(NULL, object_getClassName(self), kCFStringEncodingUTF8)))
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), self, contentSizeChanged,
                                    UIContentSizeCategoryDidChangeNotification, NULL,
                                    CFNotificationSuspensionBehaviorDrop);
}

#pragma mark - Public Functions

void collectionVC_calculateHeights(CollectionVC *d, int section) {
    const void *keys[] = {NSFontAttributeName};
    CFDictionaryRef headerAttrs = CFDictionaryCreate(NULL, keys, (const void *[]){
        getPreferredFont(UIFontTextStyleTitle3)
    }, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef titleAttrs = CFDictionaryCreate(NULL, keys, (const void *[]){
        getPreferredFont(UIFontTextStyleHeadline)
    }, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef labelAttrs = CFDictionaryCreate(NULL, keys, (const void *[]){
        getPreferredFont(UIFontTextStyleSubheadline)
    }, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CGSize headerSize = {d->headerWidth, 1000}, titleSize = {d->itemWidth - 36, 1000};
    CGSize labelSize = {d->itemWidth - 8, 1000};
    CGRect rect;

    bool allSections = section < 0;
    int lastSection = allSections ? d->totalSections : section + 1;

    for (int i = allSections ? d->firstSection : section; i < lastSection; ++i) {
        if (d->headers[i] && allSections) {
            getBoundingRect(rect, (id)d->headers[i], headerSize, headerAttrs);
            d->headerHeights[i] = (int)ceil(rect.size.height) + 20;
        }

        int items = d->itemCounts[i];
        for (int j = 0; j < items; ++j) {
            getBoundingRect(rect, (id)d->titles[i][j], titleSize, titleAttrs);
            int height = (int)ceil(rect.size.height) + 30;
            if (d->labels[i][j]) {
                getBoundingRect(rect, (id)d->labels[i][j], labelSize, labelAttrs);
                height += (int)ceil(rect.size.height) + 8;
            }
            d->itemHeights[i][j] = height;
        }
    }

    CFRelease(headerAttrs);
    CFRelease(titleAttrs);
    CFRelease(labelAttrs);
}

#pragma mark - Collection Data Source

long collectionVC_numberOfSectionsInCollectionView(id self, SEL _cmd _U_) {
    return getIVVC(CollectionVC, self)->totalSections;
}

long collectionVC_numberOfItemsInSection(id self, SEL _cmd _U_,
                                         id collectionView _U_, long section) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    return section < d->firstSection ? 0 : d->itemCounts[section];
}

id collectionVC_viewForSupplementaryElement(id self, SEL _cmd _U_,
                                            id collectionView, CFStringRef kind _U_, id indexPath) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    id header = pcvc.cv.dequeueReusableSupplementaryView(
      collectionView, pcvc.cv.drsvok, UICollectionElementKindSectionHeader, HeaderID, indexPath);
    HeaderView *v = getIVR(header);
    long section = getSection(indexPath);
    setAlpha(v->divider, section != d->firstSection);
    setText(v->label, d->headers[section]);
    setIDFormatted(v->label, CFSTR("header_%ld"), section)
    return header;
}

#pragma mark - Collection Delegate Flow Layout

CGSize collectionVC_sizeForItem(id self, SEL _cmd _U_,
                                id collectionView _U_, id layout _U_, id indexPath) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    return (CGSize){d->itemWidth, d->itemHeights[getSection(indexPath)][getItem(indexPath)]};
}

HAInsets collectionVC_insetForSection(id self, SEL _cmd _U_,
                                      id collectionView _U_, id layout _U_, long section) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    return section < d->firstSection ? (HAInsets){0} : (HAInsets){10, 8, 10, 8};
}

CGSize collectionVC_referenceSizeForHeader(id self, SEL _cmd _U_,
                                           id collectionView _U_, id layout _U_, long section) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    return section < d->firstSection ? (CGSize){0} : (CGSize){0, d->headerHeights[section]};
}

#pragma mark - Size Changes

void contentSizeChanged(CFNotificationCenterRef center _U_, void *self,
                        CFStringRef name _U_, const void *object _U_, CFDictionaryRef userInfo _U_) {
    CollectionVC *d = getIVVC(CollectionVC, self);
    collectionVC_calculateHeights(d, -1);
    invalidateLayout(d->collectionView);
}

void collectionVC_viewWillTransitionToSize(id self, SEL _cmd, CGSize size, id coordinator) {
    msgSupV(supSig(void, CGSize, id), self, VC, _cmd, size, coordinator);
    CollectionVC *d = getIVVC(CollectionVC, self);

    ((objSig(void, ObjectBlock, ObjectBlock)objc_msgSend)
     (coordinator, sel_getUid("animateAlongsideTransition:completion:"), nil, ^(id context _U_) {
        invalidateLayout(d->collectionView);
        long state = msgV(objSig(long), getSharedApplication(), sel_getUid("applicationState"));
        if (state != ApplicationStateBackground) {
            updateItemWidth(self, d, size);
            collectionVC_calculateHeights(d, -1);
            invalidateLayout(d->collectionView);
        }
    }));
}
