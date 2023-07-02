#include "HealthDataManager.h"
#include "CocoaHelpers.h"

extern CFStringRef HKQuantityTypeIdentifierBodyMass;
extern CFStringRef HKSampleSortIdentifierStartDate;

static void getBodyWeight(int *weight) {
    Class Store = objc_getClass("HKHealthStore");
    if (!msgV(clsSig(bool), Store, sel_getUid("isHealthDataAvailable"))) return;

    id store = new(Store);
    id massType = msgV(clsSig(id, CFStringRef), objc_getClass("HKObjectType"),
                       sel_getUid("quantityTypeForIdentifier:"), HKQuantityTypeIdentifierBodyMass);
    CFSetRef readTypes = CFSetCreate(NULL, (const void *[]){massType}, 1, NULL);

    ((objSig(void, CFSetRef, CFSetRef, void(^)(bool, id))objc_msgSend)
     (store, sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:"),
      NULL, readTypes, ^(bool granted, id err _U_) {
        if (!granted) {
            releaseObject(store);
            return;
        }

        id des = msgV(objSig(id, CFStringRef, bool), alloc(objc_getClass("NSSortDescriptor")),
                      sel_getUid("initWithKey:ascending:"), HKSampleSortIdentifierStartDate, false);
        CFArrayRef descriptors = CFArrayCreate(NULL, (const void *[]){des}, 1, NULL);

        id query = ((objSig(id, id, id, u_long, CFArrayRef, void(^)(id, CFArrayRef, id))objc_msgSend)
                    (alloc(objc_getClass("HKSampleQuery")),
                     sel_getUid("initWithSampleType:predicate:limit:sortDescriptors:resultsHandler:"),
                     massType, nil, 1, descriptors, ^(id q _U_, CFArrayRef results, id err2 _U_) {
            releaseObject(des);
            releaseObject(store);
            if (!(results && CFArrayGetCount(results))) return;

            id qty = msgV(objSig(id), (id)CFArrayGetValueAtIndex(results, 0), sel_getUid("quantity"));
            id pounds = msgV(clsSig(id), objc_getClass("HKUnit"), sel_getUid("poundUnit"));
            *weight = (int)msgV(objSig(double, id), qty, sel_getUid("doubleValueForUnit:"), pounds);
        }));
        msgV(objSig(void, id), store, sel_getUid("executeQuery:"), query);
        CFRelease(descriptors);
        releaseObject(query);
    }));
    CFRelease(readTypes);
}

void getMassData(int *weight, CFStringRef *unitName) {
    Class Mass = objc_getClass("NSUnitMass");
    id unit = msgV(clsSig(id), Mass, sel_getUid("poundsMass"));
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (isMetric(locale)) unit = msgV(clsSig(id), Mass, sel_getUid("kilograms"));
    CFRelease(locale);
    id formatter = new(objc_getClass("NSMeasurementFormatter"));
    CFStringRef name = msgV(objSig(CFStringRef, id), formatter, sel_getUid("stringFromUnit:"), unit);
    *unitName = CFStringCreateCopy(NULL, name);
    releaseObject(formatter);

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{ getBodyWeight(weight); });
}
