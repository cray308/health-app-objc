#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/runtime.h>

extern Class SettingsVCClass;

typedef struct {
    id planContainer;
    id switchContainer;
    id deleteButton;
    short results[4];
} SettingsVC;

id settingsVC_init(void);
void settingsVC_updateColors(id self, unsigned char darkMode);
void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id btn);

#endif /* SettingsVC_h */
