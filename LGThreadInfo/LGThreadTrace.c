//
//  LGThreadTrace.c
//  LGThreadInfo
//
//  Created by jamy on 2018/1/28.
//

#include "LGThreadTrace.h"
#include "LGMachineContext.h"
#include "LGAddressSymbolization.h"

typedef struct {
  const struct LGStackFrame *const previous;
  const uintptr_t return_address;
} LGStackFrame;


/**
 //回溯栈的算法
 栈帧布局参考：
 https://en.wikipedia.org/wiki/Call_stack
 http://www.cs.cornell.edu/courses/cs412/2008sp/lectures/lec20.pdf
 http://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/
 */

int lg_trace_thread(thread_t thread, uintptr_t *buffer) {
  _STRUCT_MCONTEXT mcontext;
  if (!lg_fill_threadstate_to_context(thread, &mcontext)) {
    return 0;
  }
  
  int i = 0;
  uintptr_t pc = lg_programcount_from_context(&mcontext);
  buffer[i++] = pc;
  
  uintptr_t linkregister = lg_linkregister_from_context(&mcontext);
  if (linkregister) {
    buffer[i++] = linkregister;
  }
  
  LGStackFrame stackFrame = {0};
  uintptr_t framepointer = lg_framepoint_from_context(&mcontext);
  if (0 == framepointer || lg_mem_copy((void *)framepointer, &stackFrame, sizeof(stackFrame))) {
    return i;
  }
  
  for (int depath = 0; depath < 32; depath++) {
    buffer[i] = stackFrame.return_address;
    if (stackFrame.return_address == 0) {
      break;
    }
    if (stackFrame.previous == NULL) {
      break;
    }
    if (lg_mem_copy((void*)stackFrame.previous, &stackFrame, sizeof(stackFrame)) != KERN_SUCCESS) {
      break;
    }
  }
  
  return 0;
}


int lg_stack_thread(thread_t thread) {
  uintptr_t tracebuffer[32] = {0};
  int frames = lg_trace_thread(thread, tracebuffer);
  if (frames == 0) {
    return 0;
  }
  //DL_info 用来保存解析的结果
  Dl_info symbolicated[frames];
  lg_symbolicate(tracebuffer, frames, symbolicated);
  
  return 0;
}
