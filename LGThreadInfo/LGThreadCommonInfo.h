//
//  LGThreadCommonInfo.h
//  LGThreadInfo
//
//  Created by jamy on 2018/1/29.
//

#import <Foundation/Foundation.h>
#import <sys/utsname.h>
#import <CoreGraphics/CoreGraphics.h>
#include <mach/mach.h>

NS_ASSUME_NONNULL_BEGIN

@interface LGThreadCommonInfo : NSObject

+ (uint64_t)memoryFootprint;

+ (NSString *)deviceName;

+ (CGFloat)totalCPUUsage;

+ (CGFloat)cpuUsageOfThread:(thread_t)thread;

+ (NSString *)getCPUType;

@end

NS_ASSUME_NONNULL_END
