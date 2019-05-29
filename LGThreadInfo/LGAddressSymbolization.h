//
//  LGAddressSymbolization.h
//  LGThreadInfo
//
//  Created by jamy on 2018/1/28.
//

#ifndef LGAddressSymbolization_h
#define LGAddressSymbolization_h

// https://www.jianshu.com/p/c58001ae3da5 参考

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <dlfcn.h>

/**
 符号化

 @param address 需要符号化的地址
 @param info 储存符号化结果
 @return 是否符号化成功
 */
bool lg_address_symbolization(uintptr_t address, Dl_info *info);

/**
 回溯调用栈进行符号化

 @param backtrack_buffer 回溯线程
 @param frames 栈帧数
 @param symbolsBuffer 储存符号化的结果
 */
void lg_symbolicate(uintptr_t *backtrack_buffer, int frames, Dl_info * symbolsBuffer);
  
#ifdef __cplusplus
}
#endif
  
#endif /* LGAddressSymbolization_h */

