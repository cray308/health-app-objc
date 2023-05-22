#ifndef ViewCache_h
#define ViewCache_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

typedef struct {
    CGFloat top, left, bottom, right;
} HAInsets;

typedef struct {
    Class cls;
    SEL cr, ac, aar, lp;
    id (*init)(Class,SEL,id,long,long,id,long,CGFloat,CGFloat);
    void (*activateC)(id,SEL,bool);
    void (*activateArr)(Class,SEL,CFArrayRef);
    void (*lowerPri)(id,SEL,float);
} ConstraintCache;

typedef struct {
    SEL trans, asv, rsv, glyr, stg, gtg, shd, sbg, sacl, shn, satrs, sace;
    void (*addSub)(id,SEL,id);
    void (*rmSub)(id,SEL);
    id (*layer)(id,SEL);
    void (*setTag)(id,SEL,long);
    long (*getTag)(id,SEL);
    void (*hide)(id,SEL,bool);
    void (*setBG)(id,SEL,id);
    void (*setAcc)(id,SEL,CFStringRef);
    void (*setHint)(id,SEL,CFStringRef);
    void (*setTraits)(id,SEL,uint64_t);
    void (*setIsAcc)(id,SEL,bool);
} ViewCache;

typedef struct {
    SEL asv, gsv, smr, ssp, scsp;
    void (*addSub)(id,SEL,id);
    CFArrayRef (*getSub)(id,SEL);
    void (*setMargins)(id,SEL,HAInsets);
    void (*setSpace)(id,SEL,CGFloat);
    void (*setSpaceAfter)(id,SEL,CGFloat,id);
} StackCache;

typedef struct {
    SEL snl, stxt, gtxt, stc;
    void (*setLines)(id,SEL,long);
    void (*setText)(id,SEL,CFStringRef);
    CFStringRef (*getText)(id,SEL);
    void (*setColor)(id,SEL,id);
} LabelCache;

typedef struct {
    SEL atgt, en, sbtxt, sbc;
    void (*addTarget)(id,SEL,id,SEL,unsigned long);
    void (*setEnabled)(id,SEL,bool);
    void (*setTitle)(id,SEL,CFStringRef,unsigned long);
    void (*setColor)(id,SEL,id,unsigned long);
} ButtonCache;

typedef struct {
    SEL sdg;
    void (*setText)(id,SEL,CFStringRef);
} FieldCache;

typedef struct {
    const ConstraintCache cc;
    const ViewCache view;
    const StackCache stack;
    const LabelCache label;
    const ButtonCache button;
    const FieldCache field;
} VCache;

typedef VCache const *VCacheRef;

#endif /* ViewCache_h */
