//
//  LGMachineContext.c
//  LGThreadInfo
//
//  Created by jamy on 2018/1/28.
//

#include "LGMachineContext.h"

kern_return_t lg_mem_copy(const void *const src, void *const dst, const size_t byteSize) {
  vm_size_t bytesCopied = 0;
  return vm_read_overwrite(mach_task_self(), (vm_address_t)src, (vm_size_t)byteSize, (vm_address_t)dst, &bytesCopied);
}

#if defined(__arm__)

bool lg_fill_threadstate_to_context(thread_t thread, _STRUCT_MCONTEXT *context) {
  mach_msg_type_number_t status_count = ARM_THREAD_STATE_COUNT;
  kern_return_t res = thread_get_state(thread, ARM_THREAD_STATE, (thread_state_t)&context->__ss, &status_count);
  return KERN_SUCCESS == res;
}

uintptr_t lg_framepoint_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__r[7];
}

uintptr_t lg_programcount_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__pc;
}

uintptr_t lg_linkregister_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__lr;
}

#endif

#if defined(__arm64__)

bool lg_fill_threadstate_to_context(thread_t thread, _STRUCT_MCONTEXT *context) {
  mach_msg_type_number_t status_count = ARM_THREAD_STATE64_COUNT;
  kern_return_t res = thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&context->__ss, &status_count);
  return KERN_SUCCESS == res;
}

uintptr_t lg_framepoint_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__fp;
}

uintptr_t lg_programcount_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__pc;
}

uintptr_t lg_linkregister_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__lr;
}

#endif

#if defined(__i386__)

bool lg_fill_threadstate_to_context(thread_t thread, _STRUCT_MCONTEXT *context) {
  mach_msg_type_number_t status_count = x86_THREAD_STATE32_COUNT;
  kern_return_t res = thread_get_state(thread, x86_THREAD_STATE32, (thread_state_t)&context->__ss, &status_count);
  return KERN_SUCCESS == res;
}

uintptr_t lg_framepoint_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__ebp;
}

uintptr_t lg_programcount_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__eip;
}

uintptr_t lg_linkregister_from_context(_STRUCT_MCONTEXT *context) {
  return 0;
}

#endif

#if defined(__x84_64__)

bool lg_fill_threadstate_to_context(thread_t thread, _STRUCT_MCONTEXT *context) {
  mach_msg_type_number_t status_count = x86_THREAD_STATE64_COUNT;
  kern_return_t res = thread_get_state(thread, x86_THREAD_STATE64, (thread_state_t)&context->__ss, &status_count);
  return KERN_SUCCESS == res;
}

uintptr_t lg_framepoint_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__rbp;
}

uintptr_t lg_programcount_from_context(_STRUCT_MCONTEXT *context) {
  return context->__ss.__rip;
}

uintptr_t lg_linkregister_from_context(_STRUCT_MCONTEXT *context) {
  return 0;
}

#endif

