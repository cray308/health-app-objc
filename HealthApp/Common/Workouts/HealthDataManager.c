#include "HealthDataManager.h"
#include "CocoaHelpers.h"

extern CFStringRef HKQuantityTypeIdentifierBodyMass;
extern CFStringRef HKSampleSortIdentifierStartDate;

static void getBodyWeight(short *weight) {
    Class storeClass = objc_getClass("HKHealthStore");
    if (!clsF0(bool, storeClass, sel_getUid("isHealthDataAvailable"))) return;

    id store = new(storeClass);
    id weightType = clsF1(id, CFStringRef, objc_getClass("HKSampleType"),
                          sel_getUid("quantityTypeForIdentifier:"), HKQuantityTypeIdentifierBodyMass);
    CFSetRef set = CFSetCreate(NULL, (const void *[]){weightType}, 1, NULL);

    msg3(void, CFSetRef, CFSetRef, void(^)(bool,id), store,
         sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:"), NULL, set, ^(bool g, id er _U_) {
        if (!g) {
            releaseO(store);
            return;
        }

        CFArrayRef arr = createSortDescriptors(HKSampleSortIdentifierStartDate, false);
        id _r = alloc(objc_getClass("HKSampleQuery"));
        id req = (((id(*)(id,SEL,id,id,u_long,CFArrayRef,void(^)(id,CFArrayRef,id)))objc_msgSend)
                  (_r, sel_getUid("initWithSampleType:predicate:limit:sortDescriptors:resultsHandler:"),
                   weightType, nil, 1, arr, ^(id q _U_, CFArrayRef data, id err2 _U_) {
            if (data && CFArrayGetCount(data)) {
                id unit = clsF0(id, objc_getClass("HKUnit"), sel_getUid("poundUnit"));
                id qty = msg0(id, (id)CFArrayGetValueAtIndex(data, 0), sel_getUid("quantity"));
                *weight = (short)msg1(double, id, qty, sel_getUid("doubleValueForUnit:"), unit);
            }
            releaseO(store);
        }));
        msg1(void, id, store, sel_getUid("executeQuery:"), req);
        CFRelease(arr);
        releaseO(req);
    });
    CFRelease(set);
}

void getHealthData(short *weight, float *multiplier, CFStringRef *unitRef) {
    Class UnitMass = objc_getClass("NSUnitMass");
    id unit = clsF0(id, UnitMass, sel_getUid("poundsMass"));
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (CFBooleanGetValue(CFLocaleGetValue(locale, kCFLocaleUsesMetricSystem))) {
        unit = clsF0(id, UnitMass, sel_getUid("kilograms"));
        *multiplier = 2.204623f;
    }
    id formatter = new(objc_getClass("NSMeasurementFormatter"));
    CFStringRef _unitStr = msg1(CFStringRef, id, formatter, sel_getUid("stringFromUnit:"), unit);
    *unitRef = CFStringCreateCopy(NULL, _unitStr);
    releaseO(formatter);
    CFRelease(locale);

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{ getBodyWeight(weight); });
}
