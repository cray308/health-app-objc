#ifndef Views_VCExt_h
#define Views_VCExt_h

#include "Views.h"

struct VCCache {
    SEL si, nb;
    id (*init)(id, SEL, id);
    id (*navBar)(id, SEL);
};

extern struct VCCache VCTable;
extern Class NavVC;

#define getNavBarAppearanceClass() objc_getClass("UINavigationBarAppearance")

#define createNavVC(r) VCTable.init(alloc(NavVC), VCTable.si, (r))
#define getNavBar(c) VCTable.navBar((c), VCTable.nb)

void initVCData(void);

void setupBar(id bar, Class BarAppearance, id color);

#endif /* Views_VCExt_h */
