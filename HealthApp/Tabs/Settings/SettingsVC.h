#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/objc.h>

extern Class SwitchViewClass;
extern Class SettingsVCClass;

typedef struct {
    id label;
    id button;
} SwitchView;

typedef struct {
    id planLabel;
    id planControl;
    struct {
        id view;
        SwitchView *data;
    } darkModeSwitch;
    id deleteButton;
    int results[4];
} SettingsVC;

void switchView_changedValue(id self, SEL _cmd);
bool switchView_accessibilityActivate(id self, SEL _cmd);

void settingsVC_updateColors(id self, bool darkMode);

void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id button);

#endif /* SettingsVC_h */
