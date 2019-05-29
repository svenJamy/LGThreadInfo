//
//  LGThreadCommonInfo.m
//  LGThreadInfo
//
//  Created by jamy on 2018/1/29.
//

#import "LGThreadCommonInfo.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/machine.h>

@implementation LGThreadCommonInfo

// phys_footprint is right than resident_size
+ (uint64_t)memoryFootprint {
  task_vm_info_data_t vmInfo;
  mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
  kern_return_t result = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) &vmInfo, &count);
  if (result != KERN_SUCCESS)
    return 0;
  return vmInfo.phys_footprint;
}

+ (NSString *)deviceName {
  struct utsname systemInfo;
  uname(&systemInfo);
  
  return [NSString stringWithCString:systemInfo.machine
                            encoding:NSUTF8StringEncoding];
}

// https://stackoverflow.com/questions/8223348/ios-get-cpu-usage-from-application/8382889#8382889
+ (CGFloat)totalCPUUsage {
  kern_return_t kr;
  task_info_data_t tinfo;
  mach_msg_type_number_t task_info_count;
  
  task_info_count = TASK_INFO_MAX;
  kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
  if (kr != KERN_SUCCESS) {
    return -1;
  }
  
  task_basic_info_t      basic_info;
  thread_array_t         thread_list;
  mach_msg_type_number_t thread_count;
  
  uint32_t stat_thread = 0; // Mach threads
  
  basic_info = (task_basic_info_t)tinfo;
  
  // get threads in the task
  kr = task_threads(mach_task_self(), &thread_list, &thread_count);
  if (kr != KERN_SUCCESS) {
    return -1;
  }
  if (thread_count > 0)
    stat_thread += thread_count;
  
  float tot_cpu = 0;
  int j;
  
  for (j = 0; j < (int)thread_count; j++) {
    tot_cpu += [self cpuUsageOfThread:thread_list[j]];
  } // for each thread
  
  kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count * sizeof(thread_t));
  assert(kr == KERN_SUCCESS);
  
  return tot_cpu;
}

+ (CGFloat)cpuUsageOfThread:(thread_t)thread {
  mach_msg_type_number_t thread_info_count = THREAD_INFO_MAX;
  thread_info_data_t     thinfo;
  thread_basic_info_t basic_info_th;
  
  kern_return_t kr = thread_info(thread, THREAD_BASIC_INFO,
                   (thread_info_t)thinfo, &thread_info_count);
  if (kr != KERN_SUCCESS) {
    return -1;
  }
  
  basic_info_th = (thread_basic_info_t)thinfo;
  
  if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
    return basic_info_th->cpu_usage / (float)TH_USAGE_SCALE * 100.0;
  }
  
  return 0;
}

// https://stackoverflow.com/questions/19859388/how-can-i-get-the-ios-device-cpu-architecture-in-runtime/19859698#19859698
+ (NSString *)getCPUType {
  size_t size;
  cpu_type_t type;
  cpu_subtype_t subtype;
  size = sizeof(type);
  sysctlbyname("hw.cputype", &type, &size, NULL, 0);
  
  size = sizeof(subtype);
  sysctlbyname("hw.cpusubtype", &subtype, &size, NULL, 0);
  
  NSString *archType = nil;
  // values for cputype and cpusubtype defined in mach/machine.h
  if (CPU_TYPE_ARM == type) {
    archType = @"ARM-32";
  } else if (CPU_TYPE_ARM64 == type) {
    archType = @"ARM-64";
  } else if (CPU_TYPE_I386 == type) {
    archType = @"i386";
  } else if (CPU_TYPE_X86_64 == type) {
    archType = @"x86-64";
  }
  
  return archType;
}

@end
