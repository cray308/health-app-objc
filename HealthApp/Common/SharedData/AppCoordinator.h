#ifndef AppCoordinator_h
#define AppCoordinator_h

#include <objc/objc.h>

typedef struct {
    id children[3];
} AppCoordinator;

extern AppCoordinator *appCoordinator;

void *appCoordinator_start(id tabVC, bool legacy, bool scrollEdge, void (**fetchHandler)(void*));
void appCoordinator_updateUserInfo(signed char plan, signed char darkMode, short *weights);
void appCoordinator_deleteAppData(void);
void appCoordinator_updateMaxWeights(short *weights);

#endif /* AppCoordinator_h */
