//
//  AppDelegate.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppCoordinator.h"

@interface AppDelegate: UIResponder <UIApplicationDelegate>
@end

@interface AppDelegate() {
    @public AppCoordinator coordinator;
    UIWindow *window;
}
@end
