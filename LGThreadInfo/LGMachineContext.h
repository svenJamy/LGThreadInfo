//
//  LGMachineContext.h
//  LGThreadInfo
//
//  Created by jamy on 2018/1/28.
//

#ifndef LGMachineContext_h
#define LGMachineContext_h

#ifdef __cplusplus
extern "C" {
#endif
  
#include <mach/mach.h>
#include <stdbool.h>
#include <sys/ucontext.h>

  
kern_return_t lg_mem_copy(const void *const src, void *const dst, const size_t byteSize);
  
/**
 fill thread info status

 @param thread thread
 @param context context
 @return return value description
 */
bool lg_fill_threadstate_to_context(thread_t thread, _STRUCT_MCONTEXT *context);

/**
 get frame point

 @param context context description
 @return frame point ptr
 */
uintptr_t lg_framepoint_from_context(_STRUCT_MCONTEXT *context);
  
  
/**
 get pc

 @param context context description
 @return pc ptr
 */
uintptr_t lg_programcount_from_context(_STRUCT_MCONTEXT *context);
  

/**
 get link register ptr

 @param context context description
 @return link register ptr
 */
uintptr_t lg_linkregister_from_context(_STRUCT_MCONTEXT *context);
  
#ifdef __cplusplus
}
#endif

#endif /* LGMachineContext_h */
