#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/runtime.h>

extern Class SettingsVCClass;
extern Ivar SettingsVCDataRef;

typedef struct __settingsVCData {
    id picker;
    id switchView;
} SettingsVCData;

id settingsVC_init(void);
void settingsVC_updateWeightFields(id self);
void settingsVC_updateColors(id self);
void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id btn);

#endif /* SettingsVC_h */
