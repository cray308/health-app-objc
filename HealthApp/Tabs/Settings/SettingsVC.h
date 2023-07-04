#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/objc.h>

extern Class SettingsVCClass;

typedef struct {
    id planControl;
    int results[4];
} SettingsVC;

void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id button);

#endif /* SettingsVC_h */
