#ifndef Views_VCExt_h
#define Views_VCExt_h

#include "Views.h"

extern Class DMNavVC;

id getAppWindow(void);

bool initVCData(bool modern, unsigned char darkMode, void (*cascade)(bool));

void setupTabVC(id vc, Class TabAppear);
void handleTintChange(id window, bool darkMode);

#endif /* Views_VCExt_h */
