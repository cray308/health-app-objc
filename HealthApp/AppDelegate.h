#include <objc/objc.h>

extern Class AppDelegateClass;

typedef struct {
    Class isa;
    id window;
    id children[3];
} AppDelegate;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id application, id options);
int appDelegate_supportedOrientations(AppDelegate *self, SEL _cmd, id application, id window);

id appDel_setWindowTint(id color);
