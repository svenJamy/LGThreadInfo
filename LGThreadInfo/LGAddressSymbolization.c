//
//  LGAddressSymbolization.c
//  LGThreadInfo
//
//  Created by jamy on 2018/1/28.
//

#include "LGAddressSymbolization.h"
#include <mach-o/arch.h>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <mach-o/nlist.h>
#include <string.h>

#ifdef __LP64__
  #define nlist_struct struct nlist_64
#else
  #define nlist_struct struct nlist
#endif


uintptr_t lg_first_cmd_after_header(const struct mach_header *header) {
  if (MH_MAGIC == header->magic || MH_CIGAM == header->magic) {
    return (uintptr_t)(header + 1);
  } else if (MH_MAGIC_64 == header->magic || MH_CIGAM_64 == header->magic) {
    return (uintptr_t)(((struct mach_header_64 *)header) + 1);
  } else {
    return 0;
  }
}

uint32_t lg_image_index_from_address(uintptr_t address) {
  uint32_t image_count = _dyld_image_count();
  uint32_t image_index = UINT32_MAX;
  
  for (uint32_t index = 0; index < image_count; index++) {
    const struct mach_header *header = _dyld_get_image_header(index);
    if (NULL != header) {
      uintptr_t cmd_ptr = lg_first_cmd_after_header(header);
      if (0 == cmd_ptr) {
        break;
      }
      
      uintptr_t slide = _dyld_get_image_vmaddr_slide(index);
      uintptr_t addr_no_slide = address - slide;
      
      for (uint32_t cmd_cnt = 0; cmd_cnt < header->ncmds; cmd_cnt++) {
        struct load_command *ld_cmd = (struct load_command *)cmd_ptr;
        if (ld_cmd->cmd == LC_SEGMENT) {
          struct segment_command *seg_cmd = (struct segment_command *)ld_cmd;
          if (addr_no_slide >= seg_cmd->vmaddr && addr_no_slide < (seg_cmd->vmaddr + seg_cmd->vmsize)) {
            image_index = index;
            break;
          }
        } else if (ld_cmd->cmd == LC_SEGMENT_64) {
          struct segment_command_64 *seg_cmd = (struct segment_command_64 *)ld_cmd;
          if (addr_no_slide >= seg_cmd->vmaddr && addr_no_slide < (seg_cmd->vmaddr + seg_cmd->vmsize)) {
            image_index = index;
            break;
          }
        } else {
        }
      }
    }
    
    if (image_index != UINT32_MAX) {
      break;
    }
  }
  
  return image_index;
}

uintptr_t lg_segment_base(uint32_t image_index) {
  const struct mach_header *header = _dyld_get_image_header(image_index);
  uintptr_t cmd_ptr = lg_first_cmd_after_header(header);
  uintptr_t seg_base = 0;
  
  for (int32_t cmd_index = 0; cmd_index < header->ncmds; cmd_index++) {
    struct load_command *ld_cmd = (struct load_command *)cmd_ptr;
    if (ld_cmd->cmd == LC_SEGMENT) {
      struct segment_command *seg_cmd = (struct segment_command *)ld_cmd;
      if (strcmp(seg_cmd->segname,  SEG_LINKEDIT) == 0) {
        seg_base = seg_cmd->vmaddr - seg_cmd->fileoff;
        break;
      }
    } else if (ld_cmd->cmd == LC_SEGMENT_64) {
      struct segment_command_64 *seg_cmd = (struct segment_command_64 *)ld_cmd;
      if (strcmp(seg_cmd->segname,  SEG_LINKEDIT) == 0) {
        seg_base = seg_cmd->vmaddr - seg_cmd->fileoff;
        break;
      }
    } else {
    }
  }
  
  return seg_base;
}

/*
 Header
 ------------------
 Load commands
 Segment command 1 -------------|
 Segment command 2              |
 ------------------             |
 Data                           |
 Section 1 data |segment 1 <----|
 Section 2 data |          <----|
 Section 3 data |          <----|
 Section 4 data |segment 2
 Section 5 data |
 ...            |
 Section n data |
 */
/*----------Mach Header---------*/

bool lg_address_symbolization(uintptr_t address, Dl_info *info) {
  if (!address) {
    return false;
  }
  info->dli_fbase = NULL;
  info->dli_fname = NULL;
  info->dli_saddr = NULL;
  info->dli_sname = NULL;
  
  // 获取address对应的image index
  uint32_t image_index = lg_image_index_from_address(address);
  if (UINT32_MAX == image_index) {
    return false;
  }

  const struct mach_header *header = _dyld_get_image_header(image_index);
  //动态链接器加载 image 时，image 必须映射到未占用地址的进程的虚拟地址空间。动态链接器通过添加一个值到 image 的基地址来实现，这个值是虚拟内存 slide 数量
  uintptr_t slide = _dyld_get_image_vmaddr_slide(image_index);
  /*-----------ASLR 的偏移量---------*/
  // https://en.wikipedia.org/wiki/Address_space_layout_randomization
  uintptr_t addr_no_slide = address - slide;
  uintptr_t cmd_ptr = lg_first_cmd_after_header(header);
  if (0 == cmd_ptr) {
    return false;
  }
  uintptr_t seg_base = lg_segment_base(image_index) + slide;
  if (0 == seg_base) {
    return false;
  }
  
  info->dli_fbase = (void *)header;
  info->dli_fname = _dyld_get_image_name(image_index);
  
  uintptr_t current_distance = UINT32_MAX;
  uintptr_t first_cmd_ptr = cmd_ptr;
  const nlist_struct *match_symbol = NULL;
  
  for (uint32_t cmd_index = 0; cmd_index < header->ncmds; cmd_index++) {
    struct load_command *load_cmd = (struct load_command *)first_cmd_ptr;
    //Segment 除了 __TEXT 和 __DATA 外还有 __LINKEDIT segment，它里面包含动态链接器的使用的原始数据，比如符号，字符串和重定位表项。
    //LC_SYMTAB 描述了 __LINKEDIT segment 内查找字符串和符号表的位置
    if (load_cmd->cmd == LC_SYMTAB) {
      const struct symtab_command *symtab_cmd = (struct symtab_command *)first_cmd_ptr;
      const nlist_struct *symbol_table = (nlist_struct *)(seg_base + symtab_cmd->symoff);
      const uintptr_t string_table = seg_base + symtab_cmd->stroff;
      
      for (uint32_t isym = 0; isym < symtab_cmd->nsyms; isym++) {
        //如果 n_value 是0，symbol 指向外部对象
        uintptr_t symbol_value = symbol_table[isym].n_value;
        if (symbol_value > 0) {
          // addr是方法的指令地址，应该大于函数的入口地址
          uintptr_t symbol_distance = addr_no_slide - symbol_value;
          if (symbol_distance < addr_no_slide && symbol_distance <= current_distance) {
            current_distance = symbol_distance;
            match_symbol = symbol_table + isym;
          }
        }
      }
      
      if (NULL != match_symbol) {
        info->dli_saddr = (void *)(match_symbol->n_value + slide);
        info->dli_sname = (char *)(string_table + (intptr_t)match_symbol->n_un.n_strx);
        if (*info->dli_sname == '_') {
          info->dli_sname++;
        }
      }
      
    }
    first_cmd_ptr+= load_cmd->cmdsize;
  }
  
  return true;
}

void lg_symbolicate(uintptr_t *backtrack_buffer, int frames, Dl_info * symbolsBuffer) {
  for (int i = 0; i < frames; i++) {
    Dl_info *info = &symbolsBuffer[i];
    uintptr_t address = backtrack_buffer[i];
    lg_address_symbolization(address, info);
  }
}
