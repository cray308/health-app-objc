#include <objc/objc.h>

extern Class AppDelegateClass;

typedef struct {
    Class isa;
    id window;
    id children[3];
} AppDelegate;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id application, id options);
int appDelegate_supportedOrientations(AppDelegate *self, SEL _cmd, id application, id window);

id appDel_getWindow(void);
void appDel_updateUserInfo(unsigned char plan, unsigned char darkMode, short *weights);
void appDel_deleteAppData(void);
void appDel_updateMaxWeights(short *weights);
