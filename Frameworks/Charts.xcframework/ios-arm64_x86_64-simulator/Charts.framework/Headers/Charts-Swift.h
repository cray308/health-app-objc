#if 0
#elif defined(__arm64__) && __arm64__
// Generated by Apple Swift version 5.3.2 (swiftlang-1200.0.45 clang-1200.0.32.28)
#ifndef CHARTS_SWIFT_H
#define CHARTS_SWIFT_H
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgcc-compat"

#if !defined(__has_include)
# define __has_include(x) 0
#endif
#if !defined(__has_attribute)
# define __has_attribute(x) 0
#endif
#if !defined(__has_feature)
# define __has_feature(x) 0
#endif
#if !defined(__has_warning)
# define __has_warning(x) 0
#endif

#if __has_include(<swift/objc-prologue.h>)
# include <swift/objc-prologue.h>
#endif

#pragma clang diagnostic ignored "-Wauto-import"
#include <Foundation/Foundation.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(SWIFT_TYPEDEFS)
# define SWIFT_TYPEDEFS 1
# if __has_include(<uchar.h>)
#  include <uchar.h>
# elif !defined(__cplusplus)
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
# endif
typedef float swift_float2  __attribute__((__ext_vector_type__(2)));
typedef float swift_float3  __attribute__((__ext_vector_type__(3)));
typedef float swift_float4  __attribute__((__ext_vector_type__(4)));
typedef double swift_double2  __attribute__((__ext_vector_type__(2)));
typedef double swift_double3  __attribute__((__ext_vector_type__(3)));
typedef double swift_double4  __attribute__((__ext_vector_type__(4)));
typedef int swift_int2  __attribute__((__ext_vector_type__(2)));
typedef int swift_int3  __attribute__((__ext_vector_type__(3)));
typedef int swift_int4  __attribute__((__ext_vector_type__(4)));
typedef unsigned int swift_uint2  __attribute__((__ext_vector_type__(2)));
typedef unsigned int swift_uint3  __attribute__((__ext_vector_type__(3)));
typedef unsigned int swift_uint4  __attribute__((__ext_vector_type__(4)));
#endif

#if !defined(SWIFT_PASTE)
# define SWIFT_PASTE_HELPER(x, y) x##y
# define SWIFT_PASTE(x, y) SWIFT_PASTE_HELPER(x, y)
#endif
#if !defined(SWIFT_METATYPE)
# define SWIFT_METATYPE(X) Class
#endif
#if !defined(SWIFT_CLASS_PROPERTY)
# if __has_feature(objc_class_property)
#  define SWIFT_CLASS_PROPERTY(...) __VA_ARGS__
# else
#  define SWIFT_CLASS_PROPERTY(...)
# endif
#endif

#if __has_attribute(objc_runtime_name)
# define SWIFT_RUNTIME_NAME(X) __attribute__((objc_runtime_name(X)))
#else
# define SWIFT_RUNTIME_NAME(X)
#endif
#if __has_attribute(swift_name)
# define SWIFT_COMPILE_NAME(X) __attribute__((swift_name(X)))
#else
# define SWIFT_COMPILE_NAME(X)
#endif
#if __has_attribute(objc_method_family)
# define SWIFT_METHOD_FAMILY(X) __attribute__((objc_method_family(X)))
#else
# define SWIFT_METHOD_FAMILY(X)
#endif
#if __has_attribute(noescape)
# define SWIFT_NOESCAPE __attribute__((noescape))
#else
# define SWIFT_NOESCAPE
#endif
#if __has_attribute(ns_consumed)
# define SWIFT_RELEASES_ARGUMENT __attribute__((ns_consumed))
#else
# define SWIFT_RELEASES_ARGUMENT
#endif
#if __has_attribute(warn_unused_result)
# define SWIFT_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define SWIFT_WARN_UNUSED_RESULT
#endif
#if __has_attribute(noreturn)
# define SWIFT_NORETURN __attribute__((noreturn))
#else
# define SWIFT_NORETURN
#endif
#if !defined(SWIFT_CLASS_EXTRA)
# define SWIFT_CLASS_EXTRA
#endif
#if !defined(SWIFT_PROTOCOL_EXTRA)
# define SWIFT_PROTOCOL_EXTRA
#endif
#if !defined(SWIFT_ENUM_EXTRA)
# define SWIFT_ENUM_EXTRA
#endif
#if !defined(SWIFT_CLASS)
# if __has_attribute(objc_subclassing_restricted)
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# else
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# endif
#endif
#if !defined(SWIFT_RESILIENT_CLASS)
# if __has_attribute(objc_class_stub)
#  define SWIFT_RESILIENT_CLASS(SWIFT_NAME) SWIFT_CLASS(SWIFT_NAME) __attribute__((objc_class_stub))
#  define SWIFT_RESILIENT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_class_stub)) SWIFT_CLASS_NAMED(SWIFT_NAME)
# else
#  define SWIFT_RESILIENT_CLASS(SWIFT_NAME) SWIFT_CLASS(SWIFT_NAME)
#  define SWIFT_RESILIENT_CLASS_NAMED(SWIFT_NAME) SWIFT_CLASS_NAMED(SWIFT_NAME)
# endif
#endif

#if !defined(SWIFT_PROTOCOL)
# define SWIFT_PROTOCOL(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
# define SWIFT_PROTOCOL_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
#endif

#if !defined(SWIFT_EXTENSION)
# define SWIFT_EXTENSION(M) SWIFT_PASTE(M##_Swift_, __LINE__)
#endif

#if !defined(OBJC_DESIGNATED_INITIALIZER)
# if __has_attribute(objc_designated_initializer)
#  define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
# else
#  define OBJC_DESIGNATED_INITIALIZER
# endif
#endif
#if !defined(SWIFT_ENUM_ATTR)
# if defined(__has_attribute) && __has_attribute(enum_extensibility)
#  define SWIFT_ENUM_ATTR(_extensibility) __attribute__((enum_extensibility(_extensibility)))
# else
#  define SWIFT_ENUM_ATTR(_extensibility)
# endif
#endif
#if !defined(SWIFT_ENUM)
# define SWIFT_ENUM(_type, _name, _extensibility) enum _name : _type _name; enum SWIFT_ENUM_ATTR(_extensibility) SWIFT_ENUM_EXTRA _name : _type
# if __has_feature(generalized_swift_name)
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME, _extensibility) enum _name : _type _name SWIFT_COMPILE_NAME(SWIFT_NAME); enum SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_ENUM_ATTR(_extensibility) SWIFT_ENUM_EXTRA _name : _type
# else
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME, _extensibility) SWIFT_ENUM(_type, _name, _extensibility)
# endif
#endif
#if !defined(SWIFT_UNAVAILABLE)
# define SWIFT_UNAVAILABLE __attribute__((unavailable))
#endif
#if !defined(SWIFT_UNAVAILABLE_MSG)
# define SWIFT_UNAVAILABLE_MSG(msg) __attribute__((unavailable(msg)))
#endif
#if !defined(SWIFT_AVAILABILITY)
# define SWIFT_AVAILABILITY(plat, ...) __attribute__((availability(plat, __VA_ARGS__)))
#endif
#if !defined(SWIFT_WEAK_IMPORT)
# define SWIFT_WEAK_IMPORT __attribute__((weak_import))
#endif
#if !defined(SWIFT_DEPRECATED)
# define SWIFT_DEPRECATED __attribute__((deprecated))
#endif
#if !defined(SWIFT_DEPRECATED_MSG)
# define SWIFT_DEPRECATED_MSG(...) __attribute__((deprecated(__VA_ARGS__)))
#endif
#if __has_feature(attribute_diagnose_if_objc)
# define SWIFT_DEPRECATED_OBJC(Msg) __attribute__((diagnose_if(1, Msg, "warning")))
#else
# define SWIFT_DEPRECATED_OBJC(Msg) SWIFT_DEPRECATED_MSG(Msg)
#endif
#if !defined(IBSegueAction)
# define IBSegueAction
#endif
#if __has_feature(modules)
#if __has_warning("-Watimport-in-framework-header")
#pragma clang diagnostic ignored "-Watimport-in-framework-header"
#endif
@import CoreGraphics;
@import Foundation;
@import ObjectiveC;
@import UIKit;
#endif

#pragma clang diagnostic ignored "-Wproperty-attribute-mismatch"
#pragma clang diagnostic ignored "-Wduplicate-method-arg"
#if __has_warning("-Wpragma-clang-attribute")
# pragma clang diagnostic ignored "-Wpragma-clang-attribute"
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wnullability"

#if __has_attribute(external_source_symbol)
# pragma push_macro("any")
# undef any
# pragma clang attribute push(__attribute__((external_source_symbol(language="Swift", defined_in="Charts",generated_declaration))), apply_to=any(function,enum,objc_interface,objc_category,objc_protocol))
# pragma pop_macro("any")
#endif


SWIFT_CLASS("_TtC6Charts8Animator")
@interface Animator : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end

@class LineChartDataSet;
@class LineChartView;

SWIFT_PROTOCOL("_TtP6Charts13FillFormatter_")
@protocol FillFormatter
- (CGFloat)getFillLinePositionWithDataSet:(LineChartDataSet * _Nonnull)dataSet dataProvider:(LineChartView * _Nonnull)dataProvider SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6Charts18AreaChartFormatter")
@interface AreaChartFormatter : NSObject <FillFormatter>
- (nonnull instancetype)initWithBoundaryDataSet:(LineChartDataSet * _Nonnull)boundaryDataSet OBJC_DESIGNATED_INITIALIZER;
- (CGFloat)getFillLinePositionWithDataSet:(LineChartDataSet * _Nonnull)dataSet dataProvider:(LineChartView * _Nonnull)dataProvider SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts13ChartRenderer")
@interface ChartRenderer : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts17AreaChartRenderer")
@interface AreaChartRenderer : ChartRenderer
- (nonnull instancetype)initWithView:(LineChartView * _Nonnull)view OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts13ComponentBase")
@interface ComponentBase : NSObject
@property (nonatomic) BOOL enabled;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end

@protocol AxisValueFormatter;
@class ChartLimitLine;

SWIFT_CLASS("_TtC6Charts8AxisBase")
@interface AxisBase : ComponentBase
@property (nonatomic, strong) id <AxisValueFormatter> _Nonnull valueFormatter;
@property (nonatomic) NSInteger labelCount;
- (void)addLimitLine:(ChartLimitLine * _Nonnull)line;
@property (nonatomic) double axisMaximum;
@end


SWIFT_PROTOCOL("_TtP6Charts18AxisValueFormatter_")
@protocol AxisValueFormatter
- (NSString * _Nonnull)stringForValue:(double)value axis:(AxisBase * _Nullable)axis SWIFT_WARN_UNUSED_RESULT;
@end

@class ChartData;
@class XAxis;
@class Legend;
@class NSCoder;

SWIFT_CLASS("_TtC6Charts13ChartViewBase")
@interface ChartViewBase : UIView
@property (nonatomic, strong) ChartData * _Nullable data;
@property (nonatomic, readonly, strong) XAxis * _Nonnull xAxis;
@property (nonatomic, readonly, strong) Legend * _Nonnull legend;
@property (nonatomic, strong) ChartRenderer * _Nullable renderer;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
- (void)notifyDataSetChanged;
- (void)drawRect:(CGRect)rect;
- (void)animateWithXAxisDuration:(NSTimeInterval)xAxisDuration;
- (void)observeValueForKeyPath:(NSString * _Nullable)keyPath ofObject:(id _Nullable)object change:(NSDictionary<NSKeyValueChangeKey, id> * _Nullable)change context:(void * _Nullable)context;
@end

@class YAxis;
@class UIGestureRecognizer;

SWIFT_CLASS("_TtC6Charts20BarLineChartViewBase")
@interface BarLineChartViewBase : ChartViewBase <UIGestureRecognizerDelegate>
@property (nonatomic, readonly, strong) YAxis * _Nonnull leftAxis;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
- (void)observeValueForKeyPath:(NSString * _Nullable)keyPath ofObject:(id _Nullable)object change:(NSDictionary<NSKeyValueChangeKey, id> * _Nullable)change context:(void * _Nullable)context;
- (void)drawRect:(CGRect)rect;
- (void)notifyDataSetChanged;
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer * _Nonnull)gestureRecognizer SWIFT_WARN_UNUSED_RESULT;
- (BOOL)gestureRecognizer:(UIGestureRecognizer * _Nonnull)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer * _Nonnull)otherGestureRecognizer SWIFT_WARN_UNUSED_RESULT;
@end

@class ChartDataEntry;
@class UIColor;

SWIFT_CLASS("_TtC6Charts12ChartDataSet")
@interface ChartDataSet : NSObject <NSCopying>
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
- (void)replaceEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries;
@property (nonatomic, copy) NSArray<UIColor *> * _Nonnull colors;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6Charts38BarLineScatterCandleBubbleChartDataSet")
@interface BarLineScatterCandleBubbleChartDataSet : ChartDataSet
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
@end

@protocol ValueFormatter;

SWIFT_CLASS("_TtC6Charts9ChartData")
@interface ChartData : NSObject
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithDataSets:(NSArray<ChartDataSet *> * _Nonnull)dataSets OBJC_DESIGNATED_INITIALIZER;
- (void)notifyDataChanged;
- (void)setValueFormatter:(id <ValueFormatter> _Nonnull)formatter;
- (void)setDrawValues:(BOOL)enabled;
@end






SWIFT_CLASS("_TtC6Charts14ChartDataEntry")
@interface ChartDataEntry : NSObject <NSCopying>
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithX:(double)x y:(double)y OBJC_DESIGNATED_INITIALIZER;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
@end


@interface ChartDataEntry (SWIFT_EXTENSION(Charts))
- (BOOL)isEqual:(id _Nullable)object SWIFT_WARN_UNUSED_RESULT;
@end






SWIFT_CLASS("_TtC6Charts14ChartLimitLine")
@interface ChartLimitLine : ComponentBase
@property (nonatomic) double limit;
@property (nonatomic, strong) UIColor * _Nonnull lineColor;
- (nonnull instancetype)initWithLimit:(double)limit OBJC_DESIGNATED_INITIALIZER;
@end




@class ViewPortHandler;

SWIFT_PROTOCOL("_TtP6Charts14ValueFormatter_")
@protocol ValueFormatter
- (NSString * _Nonnull)stringForValue:(double)value entry:(ChartDataEntry * _Nonnull)entry dataSetIndex:(NSInteger)dataSetIndex viewPortHandler:(ViewPortHandler * _Nullable)viewPortHandler SWIFT_WARN_UNUSED_RESULT;
@end

@class NSNumberFormatter;

SWIFT_CLASS("_TtC6Charts21DefaultValueFormatter")
@interface DefaultValueFormatter : NSObject <ValueFormatter>
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
- (nonnull instancetype)initWithFormatter:(NSNumberFormatter * _Nonnull)formatter OBJC_DESIGNATED_INITIALIZER;
- (NSString * _Nonnull)stringForValue:(double)value entry:(ChartDataEntry * _Nonnull)entry dataSetIndex:(NSInteger)dataSetIndex viewPortHandler:(ViewPortHandler * _Nullable)viewPortHandler SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_PROTOCOL("_TtP6Charts4Fill_")
@protocol Fill
- (void)fillPathWithContext:(CGContextRef _Nonnull)context rect:(CGRect)rect;
@end



SWIFT_CLASS("_TtC6Charts6Legend")
@interface Legend : ComponentBase
@end


SWIFT_CLASS("_TtC6Charts11LegendEntry")
@interface LegendEntry : NSObject
- (nonnull instancetype)initWithLabel:(NSString * _Nullable)label color:(UIColor * _Nullable)color OBJC_DESIGNATED_INITIALIZER;
@property (nonatomic, copy) NSString * _Nullable label;
@property (nonatomic, strong) UIColor * _Nullable formColor;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts14LegendRenderer")
@interface LegendRenderer : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts13LineChartData")
@interface LineChartData : ChartData
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithDataSets:(NSArray<ChartDataSet *> * _Nonnull)dataSets OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts34LineScatterCandleRadarChartDataSet")
@interface LineScatterCandleRadarChartDataSet : BarLineScatterCandleBubbleChartDataSet
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts21LineRadarChartDataSet")
@interface LineRadarChartDataSet : LineScatterCandleRadarChartDataSet
@property (nonatomic, strong) UIColor * _Nonnull fillColor;
@property (nonatomic, strong) id <Fill> _Nullable fill;
@property (nonatomic) CGFloat fillAlpha;
@property (nonatomic) CGFloat lineWidth;
@property (nonatomic) BOOL drawFilledEnabled;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts16LineChartDataSet")
@interface LineChartDataSet : LineRadarChartDataSet
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
- (void)setCircleColor:(UIColor * _Nonnull)color;
@property (nonatomic) BOOL drawCirclesEnabled;
@property (nonatomic, strong) id <FillFormatter> _Nullable fillFormatter;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6Charts13LineChartView")
@interface LineChartView : BarLineChartViewBase
- (nonnull instancetype)initWithLegendEntries:(NSArray<LegendEntry *> * _Nonnull)legendEntries OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithFrame:(CGRect)frame SWIFT_UNAVAILABLE;
@end




SWIFT_CLASS("_TtC6Charts18LinearGradientFill")
@interface LinearGradientFill : NSObject <Fill>
- (nonnull instancetype)initWithEndColor:(UIColor * _Nonnull)endColor OBJC_DESIGNATED_INITIALIZER;
- (void)fillPathWithContext:(CGContextRef _Nonnull)context rect:(CGRect)rect;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end



SWIFT_CLASS("_TtC6Charts15ViewPortHandler")
@interface ViewPortHandler : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts5XAxis")
@interface XAxis : AxisBase
@end


SWIFT_CLASS("_TtC6Charts5YAxis")
@interface YAxis : AxisBase
@end

#if __has_attribute(external_source_symbol)
# pragma clang attribute pop
#endif
#pragma clang diagnostic pop
#endif

#elif defined(__x86_64__) && __x86_64__
// Generated by Apple Swift version 5.3.2 (swiftlang-1200.0.45 clang-1200.0.32.28)
#ifndef CHARTS_SWIFT_H
#define CHARTS_SWIFT_H
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgcc-compat"

#if !defined(__has_include)
# define __has_include(x) 0
#endif
#if !defined(__has_attribute)
# define __has_attribute(x) 0
#endif
#if !defined(__has_feature)
# define __has_feature(x) 0
#endif
#if !defined(__has_warning)
# define __has_warning(x) 0
#endif

#if __has_include(<swift/objc-prologue.h>)
# include <swift/objc-prologue.h>
#endif

#pragma clang diagnostic ignored "-Wauto-import"
#include <Foundation/Foundation.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(SWIFT_TYPEDEFS)
# define SWIFT_TYPEDEFS 1
# if __has_include(<uchar.h>)
#  include <uchar.h>
# elif !defined(__cplusplus)
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
# endif
typedef float swift_float2  __attribute__((__ext_vector_type__(2)));
typedef float swift_float3  __attribute__((__ext_vector_type__(3)));
typedef float swift_float4  __attribute__((__ext_vector_type__(4)));
typedef double swift_double2  __attribute__((__ext_vector_type__(2)));
typedef double swift_double3  __attribute__((__ext_vector_type__(3)));
typedef double swift_double4  __attribute__((__ext_vector_type__(4)));
typedef int swift_int2  __attribute__((__ext_vector_type__(2)));
typedef int swift_int3  __attribute__((__ext_vector_type__(3)));
typedef int swift_int4  __attribute__((__ext_vector_type__(4)));
typedef unsigned int swift_uint2  __attribute__((__ext_vector_type__(2)));
typedef unsigned int swift_uint3  __attribute__((__ext_vector_type__(3)));
typedef unsigned int swift_uint4  __attribute__((__ext_vector_type__(4)));
#endif

#if !defined(SWIFT_PASTE)
# define SWIFT_PASTE_HELPER(x, y) x##y
# define SWIFT_PASTE(x, y) SWIFT_PASTE_HELPER(x, y)
#endif
#if !defined(SWIFT_METATYPE)
# define SWIFT_METATYPE(X) Class
#endif
#if !defined(SWIFT_CLASS_PROPERTY)
# if __has_feature(objc_class_property)
#  define SWIFT_CLASS_PROPERTY(...) __VA_ARGS__
# else
#  define SWIFT_CLASS_PROPERTY(...)
# endif
#endif

#if __has_attribute(objc_runtime_name)
# define SWIFT_RUNTIME_NAME(X) __attribute__((objc_runtime_name(X)))
#else
# define SWIFT_RUNTIME_NAME(X)
#endif
#if __has_attribute(swift_name)
# define SWIFT_COMPILE_NAME(X) __attribute__((swift_name(X)))
#else
# define SWIFT_COMPILE_NAME(X)
#endif
#if __has_attribute(objc_method_family)
# define SWIFT_METHOD_FAMILY(X) __attribute__((objc_method_family(X)))
#else
# define SWIFT_METHOD_FAMILY(X)
#endif
#if __has_attribute(noescape)
# define SWIFT_NOESCAPE __attribute__((noescape))
#else
# define SWIFT_NOESCAPE
#endif
#if __has_attribute(ns_consumed)
# define SWIFT_RELEASES_ARGUMENT __attribute__((ns_consumed))
#else
# define SWIFT_RELEASES_ARGUMENT
#endif
#if __has_attribute(warn_unused_result)
# define SWIFT_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define SWIFT_WARN_UNUSED_RESULT
#endif
#if __has_attribute(noreturn)
# define SWIFT_NORETURN __attribute__((noreturn))
#else
# define SWIFT_NORETURN
#endif
#if !defined(SWIFT_CLASS_EXTRA)
# define SWIFT_CLASS_EXTRA
#endif
#if !defined(SWIFT_PROTOCOL_EXTRA)
# define SWIFT_PROTOCOL_EXTRA
#endif
#if !defined(SWIFT_ENUM_EXTRA)
# define SWIFT_ENUM_EXTRA
#endif
#if !defined(SWIFT_CLASS)
# if __has_attribute(objc_subclassing_restricted)
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# else
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# endif
#endif
#if !defined(SWIFT_RESILIENT_CLASS)
# if __has_attribute(objc_class_stub)
#  define SWIFT_RESILIENT_CLASS(SWIFT_NAME) SWIFT_CLASS(SWIFT_NAME) __attribute__((objc_class_stub))
#  define SWIFT_RESILIENT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_class_stub)) SWIFT_CLASS_NAMED(SWIFT_NAME)
# else
#  define SWIFT_RESILIENT_CLASS(SWIFT_NAME) SWIFT_CLASS(SWIFT_NAME)
#  define SWIFT_RESILIENT_CLASS_NAMED(SWIFT_NAME) SWIFT_CLASS_NAMED(SWIFT_NAME)
# endif
#endif

#if !defined(SWIFT_PROTOCOL)
# define SWIFT_PROTOCOL(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
# define SWIFT_PROTOCOL_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
#endif

#if !defined(SWIFT_EXTENSION)
# define SWIFT_EXTENSION(M) SWIFT_PASTE(M##_Swift_, __LINE__)
#endif

#if !defined(OBJC_DESIGNATED_INITIALIZER)
# if __has_attribute(objc_designated_initializer)
#  define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
# else
#  define OBJC_DESIGNATED_INITIALIZER
# endif
#endif
#if !defined(SWIFT_ENUM_ATTR)
# if defined(__has_attribute) && __has_attribute(enum_extensibility)
#  define SWIFT_ENUM_ATTR(_extensibility) __attribute__((enum_extensibility(_extensibility)))
# else
#  define SWIFT_ENUM_ATTR(_extensibility)
# endif
#endif
#if !defined(SWIFT_ENUM)
# define SWIFT_ENUM(_type, _name, _extensibility) enum _name : _type _name; enum SWIFT_ENUM_ATTR(_extensibility) SWIFT_ENUM_EXTRA _name : _type
# if __has_feature(generalized_swift_name)
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME, _extensibility) enum _name : _type _name SWIFT_COMPILE_NAME(SWIFT_NAME); enum SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_ENUM_ATTR(_extensibility) SWIFT_ENUM_EXTRA _name : _type
# else
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME, _extensibility) SWIFT_ENUM(_type, _name, _extensibility)
# endif
#endif
#if !defined(SWIFT_UNAVAILABLE)
# define SWIFT_UNAVAILABLE __attribute__((unavailable))
#endif
#if !defined(SWIFT_UNAVAILABLE_MSG)
# define SWIFT_UNAVAILABLE_MSG(msg) __attribute__((unavailable(msg)))
#endif
#if !defined(SWIFT_AVAILABILITY)
# define SWIFT_AVAILABILITY(plat, ...) __attribute__((availability(plat, __VA_ARGS__)))
#endif
#if !defined(SWIFT_WEAK_IMPORT)
# define SWIFT_WEAK_IMPORT __attribute__((weak_import))
#endif
#if !defined(SWIFT_DEPRECATED)
# define SWIFT_DEPRECATED __attribute__((deprecated))
#endif
#if !defined(SWIFT_DEPRECATED_MSG)
# define SWIFT_DEPRECATED_MSG(...) __attribute__((deprecated(__VA_ARGS__)))
#endif
#if __has_feature(attribute_diagnose_if_objc)
# define SWIFT_DEPRECATED_OBJC(Msg) __attribute__((diagnose_if(1, Msg, "warning")))
#else
# define SWIFT_DEPRECATED_OBJC(Msg) SWIFT_DEPRECATED_MSG(Msg)
#endif
#if !defined(IBSegueAction)
# define IBSegueAction
#endif
#if __has_feature(modules)
#if __has_warning("-Watimport-in-framework-header")
#pragma clang diagnostic ignored "-Watimport-in-framework-header"
#endif
@import CoreGraphics;
@import Foundation;
@import ObjectiveC;
@import UIKit;
#endif

#pragma clang diagnostic ignored "-Wproperty-attribute-mismatch"
#pragma clang diagnostic ignored "-Wduplicate-method-arg"
#if __has_warning("-Wpragma-clang-attribute")
# pragma clang diagnostic ignored "-Wpragma-clang-attribute"
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wnullability"

#if __has_attribute(external_source_symbol)
# pragma push_macro("any")
# undef any
# pragma clang attribute push(__attribute__((external_source_symbol(language="Swift", defined_in="Charts",generated_declaration))), apply_to=any(function,enum,objc_interface,objc_category,objc_protocol))
# pragma pop_macro("any")
#endif


SWIFT_CLASS("_TtC6Charts8Animator")
@interface Animator : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end

@class LineChartDataSet;
@class LineChartView;

SWIFT_PROTOCOL("_TtP6Charts13FillFormatter_")
@protocol FillFormatter
- (CGFloat)getFillLinePositionWithDataSet:(LineChartDataSet * _Nonnull)dataSet dataProvider:(LineChartView * _Nonnull)dataProvider SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6Charts18AreaChartFormatter")
@interface AreaChartFormatter : NSObject <FillFormatter>
- (nonnull instancetype)initWithBoundaryDataSet:(LineChartDataSet * _Nonnull)boundaryDataSet OBJC_DESIGNATED_INITIALIZER;
- (CGFloat)getFillLinePositionWithDataSet:(LineChartDataSet * _Nonnull)dataSet dataProvider:(LineChartView * _Nonnull)dataProvider SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts13ChartRenderer")
@interface ChartRenderer : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts17AreaChartRenderer")
@interface AreaChartRenderer : ChartRenderer
- (nonnull instancetype)initWithView:(LineChartView * _Nonnull)view OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts13ComponentBase")
@interface ComponentBase : NSObject
@property (nonatomic) BOOL enabled;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end

@protocol AxisValueFormatter;
@class ChartLimitLine;

SWIFT_CLASS("_TtC6Charts8AxisBase")
@interface AxisBase : ComponentBase
@property (nonatomic, strong) id <AxisValueFormatter> _Nonnull valueFormatter;
@property (nonatomic) NSInteger labelCount;
- (void)addLimitLine:(ChartLimitLine * _Nonnull)line;
@property (nonatomic) double axisMaximum;
@end


SWIFT_PROTOCOL("_TtP6Charts18AxisValueFormatter_")
@protocol AxisValueFormatter
- (NSString * _Nonnull)stringForValue:(double)value axis:(AxisBase * _Nullable)axis SWIFT_WARN_UNUSED_RESULT;
@end

@class ChartData;
@class XAxis;
@class Legend;
@class NSCoder;

SWIFT_CLASS("_TtC6Charts13ChartViewBase")
@interface ChartViewBase : UIView
@property (nonatomic, strong) ChartData * _Nullable data;
@property (nonatomic, readonly, strong) XAxis * _Nonnull xAxis;
@property (nonatomic, readonly, strong) Legend * _Nonnull legend;
@property (nonatomic, strong) ChartRenderer * _Nullable renderer;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
- (void)notifyDataSetChanged;
- (void)drawRect:(CGRect)rect;
- (void)animateWithXAxisDuration:(NSTimeInterval)xAxisDuration;
- (void)observeValueForKeyPath:(NSString * _Nullable)keyPath ofObject:(id _Nullable)object change:(NSDictionary<NSKeyValueChangeKey, id> * _Nullable)change context:(void * _Nullable)context;
@end

@class YAxis;
@class UIGestureRecognizer;

SWIFT_CLASS("_TtC6Charts20BarLineChartViewBase")
@interface BarLineChartViewBase : ChartViewBase <UIGestureRecognizerDelegate>
@property (nonatomic, readonly, strong) YAxis * _Nonnull leftAxis;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
- (void)observeValueForKeyPath:(NSString * _Nullable)keyPath ofObject:(id _Nullable)object change:(NSDictionary<NSKeyValueChangeKey, id> * _Nullable)change context:(void * _Nullable)context;
- (void)drawRect:(CGRect)rect;
- (void)notifyDataSetChanged;
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer * _Nonnull)gestureRecognizer SWIFT_WARN_UNUSED_RESULT;
- (BOOL)gestureRecognizer:(UIGestureRecognizer * _Nonnull)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer * _Nonnull)otherGestureRecognizer SWIFT_WARN_UNUSED_RESULT;
@end

@class ChartDataEntry;
@class UIColor;

SWIFT_CLASS("_TtC6Charts12ChartDataSet")
@interface ChartDataSet : NSObject <NSCopying>
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
- (void)replaceEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries;
@property (nonatomic, copy) NSArray<UIColor *> * _Nonnull colors;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6Charts38BarLineScatterCandleBubbleChartDataSet")
@interface BarLineScatterCandleBubbleChartDataSet : ChartDataSet
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
@end

@protocol ValueFormatter;

SWIFT_CLASS("_TtC6Charts9ChartData")
@interface ChartData : NSObject
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithDataSets:(NSArray<ChartDataSet *> * _Nonnull)dataSets OBJC_DESIGNATED_INITIALIZER;
- (void)notifyDataChanged;
- (void)setValueFormatter:(id <ValueFormatter> _Nonnull)formatter;
- (void)setDrawValues:(BOOL)enabled;
@end






SWIFT_CLASS("_TtC6Charts14ChartDataEntry")
@interface ChartDataEntry : NSObject <NSCopying>
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithX:(double)x y:(double)y OBJC_DESIGNATED_INITIALIZER;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
@end


@interface ChartDataEntry (SWIFT_EXTENSION(Charts))
- (BOOL)isEqual:(id _Nullable)object SWIFT_WARN_UNUSED_RESULT;
@end






SWIFT_CLASS("_TtC6Charts14ChartLimitLine")
@interface ChartLimitLine : ComponentBase
@property (nonatomic) double limit;
@property (nonatomic, strong) UIColor * _Nonnull lineColor;
- (nonnull instancetype)initWithLimit:(double)limit OBJC_DESIGNATED_INITIALIZER;
@end




@class ViewPortHandler;

SWIFT_PROTOCOL("_TtP6Charts14ValueFormatter_")
@protocol ValueFormatter
- (NSString * _Nonnull)stringForValue:(double)value entry:(ChartDataEntry * _Nonnull)entry dataSetIndex:(NSInteger)dataSetIndex viewPortHandler:(ViewPortHandler * _Nullable)viewPortHandler SWIFT_WARN_UNUSED_RESULT;
@end

@class NSNumberFormatter;

SWIFT_CLASS("_TtC6Charts21DefaultValueFormatter")
@interface DefaultValueFormatter : NSObject <ValueFormatter>
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
- (nonnull instancetype)initWithFormatter:(NSNumberFormatter * _Nonnull)formatter OBJC_DESIGNATED_INITIALIZER;
- (NSString * _Nonnull)stringForValue:(double)value entry:(ChartDataEntry * _Nonnull)entry dataSetIndex:(NSInteger)dataSetIndex viewPortHandler:(ViewPortHandler * _Nullable)viewPortHandler SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_PROTOCOL("_TtP6Charts4Fill_")
@protocol Fill
- (void)fillPathWithContext:(CGContextRef _Nonnull)context rect:(CGRect)rect;
@end



SWIFT_CLASS("_TtC6Charts6Legend")
@interface Legend : ComponentBase
@end


SWIFT_CLASS("_TtC6Charts11LegendEntry")
@interface LegendEntry : NSObject
- (nonnull instancetype)initWithLabel:(NSString * _Nullable)label color:(UIColor * _Nullable)color OBJC_DESIGNATED_INITIALIZER;
@property (nonatomic, copy) NSString * _Nullable label;
@property (nonatomic, strong) UIColor * _Nullable formColor;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts14LegendRenderer")
@interface LegendRenderer : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts13LineChartData")
@interface LineChartData : ChartData
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithDataSets:(NSArray<ChartDataSet *> * _Nonnull)dataSets OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts34LineScatterCandleRadarChartDataSet")
@interface LineScatterCandleRadarChartDataSet : BarLineScatterCandleBubbleChartDataSet
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts21LineRadarChartDataSet")
@interface LineRadarChartDataSet : LineScatterCandleRadarChartDataSet
@property (nonatomic, strong) UIColor * _Nonnull fillColor;
@property (nonatomic, strong) id <Fill> _Nullable fill;
@property (nonatomic) CGFloat fillAlpha;
@property (nonatomic) CGFloat lineWidth;
@property (nonatomic) BOOL drawFilledEnabled;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6Charts16LineChartDataSet")
@interface LineChartDataSet : LineRadarChartDataSet
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEntries:(NSArray<ChartDataEntry *> * _Nonnull)entries OBJC_DESIGNATED_INITIALIZER;
- (void)setCircleColor:(UIColor * _Nonnull)color;
@property (nonatomic) BOOL drawCirclesEnabled;
@property (nonatomic, strong) id <FillFormatter> _Nullable fillFormatter;
- (id _Nonnull)copyWithZone:(struct _NSZone * _Nullable)zone SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6Charts13LineChartView")
@interface LineChartView : BarLineChartViewBase
- (nonnull instancetype)initWithLegendEntries:(NSArray<LegendEntry *> * _Nonnull)legendEntries OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithFrame:(CGRect)frame SWIFT_UNAVAILABLE;
@end




SWIFT_CLASS("_TtC6Charts18LinearGradientFill")
@interface LinearGradientFill : NSObject <Fill>
- (nonnull instancetype)initWithEndColor:(UIColor * _Nonnull)endColor OBJC_DESIGNATED_INITIALIZER;
- (void)fillPathWithContext:(CGContextRef _Nonnull)context rect:(CGRect)rect;
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end



SWIFT_CLASS("_TtC6Charts15ViewPortHandler")
@interface ViewPortHandler : NSObject
- (nonnull instancetype)init SWIFT_UNAVAILABLE;
+ (nonnull instancetype)new SWIFT_UNAVAILABLE_MSG("-init is unavailable");
@end


SWIFT_CLASS("_TtC6Charts5XAxis")
@interface XAxis : AxisBase
@end


SWIFT_CLASS("_TtC6Charts5YAxis")
@interface YAxis : AxisBase
@end

#if __has_attribute(external_source_symbol)
# pragma clang attribute pop
#endif
#pragma clang diagnostic pop
#endif

#endif
