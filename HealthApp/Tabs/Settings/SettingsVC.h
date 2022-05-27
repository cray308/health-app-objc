#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/objc.h>

#define SettingsVCEncoding "{?=@@{?=@@}@[4s]}"

extern Class SwitchViewClass;
extern Class SettingsVCClass;

typedef struct {
    id label;
    id sv;
} SwitchView;

typedef struct {
    id planLabel;
    id planPicker;
    struct SwitchPair {
        id view;
        SwitchView *d;
    } dmSwitch;
    id deleteButton;
    short results[4];
} SettingsVC;

void switchView_didChange(id self, SEL _cmd);
bool switchView_activate(id self, SEL _cmd);

void settingsVC_updateColors(id self, bool darkMode);

void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id btn);

#endif /* SettingsVC_h */
