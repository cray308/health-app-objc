#include <objc/objc.h>

extern Class AppDelegateClass;

typedef struct {
    Class isa;
    id window;
    id children[3];
} AppDelegate;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id application, id options);
int appDelegate_supportedOrientations(AppDelegate *self, SEL _cmd, id application, id window);

void appDel_setWindowTint(id color);
void appDel_updateUserInfo(signed char plan, signed char darkMode, short *weights);
void appDel_deleteAppData(void);
void appDel_updateMaxWeights(short *weights);
