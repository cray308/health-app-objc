#ifndef SettingsVC_h
#define SettingsVC_h

#include <objc/objc.h>

#define SettingsVCEncoding "{?=@[4s]}"

extern Class SettingsVCClass;

typedef struct {
    id planPicker;
    short results[4];
} SettingsVC;

void settingsVC_viewDidLoad(id self, SEL _cmd);
void settingsVC_buttonTapped(id self, SEL _cmd, id btn);

#endif /* SettingsVC_h */
