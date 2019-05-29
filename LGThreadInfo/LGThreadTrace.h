//
//  LGThreadTrace.h
//  LGThreadInfo
//
//  Created by jamy on 2018/1/28.
//

#ifndef LGThreadTrace_h
#define LGThreadTrace_h

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/mach.h>
#include <dlfcn.h>
  
/**
 回溯线程调用栈

 @param thread 目标线程
 @param buffer buffer
 @return 回溯的栈帧数
 */
int lg_trace_thread(thread_t thread, uintptr_t *buffer);
  
  
#ifdef __cplusplus
}
#endif

#endif /* LGThreadTrace_h */
