#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/objc.h>
#include "ColorCache.h"
#include "ViewCache.h"

extern Class SettingsVCClass;

typedef struct {
    id planLabel;
    id planPicker;
    id switchContainer;
    id deleteButton;
    int results[4];
} SettingsVC;

id settingsVC_init(VCacheRef tbl, CCacheRef clr);
void settingsVC_updateColors(id self, bool darkMode);

void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id btn);

#endif /* SettingsVC_h */
