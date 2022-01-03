#ifndef AppCoordinator_h
#define AppCoordinator_h

#include <objc/objc.h>

enum {
    LoadedVC_Home = 0x1,
    LoadedVC_History = 0x2,
    LoadedVC_Settings = 0x4
};

typedef struct {
    unsigned char loadedViewControllers;
    id children[3];
} AppCoordinator;

extern AppCoordinator *appCoordinator;

void *appCoordinator_start(id tabVC, void (**fetchHandler)(void*));
void appCoordinator_updateUserInfo(signed char plan, signed char darkMode, short *weights);
void appCoordinator_deleteAppData(void);
void appCoordinator_updateMaxWeights(short *weights);

#endif /* AppCoordinator_h */
