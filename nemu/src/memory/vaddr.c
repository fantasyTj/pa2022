/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

#define TEMP_MASK (~0xfffff000)

word_t vaddr_ifetch(vaddr_t addr, int len) {
  if(isa_mmu_check(addr, len, MEM_TYPE_IFETCH) == MMU_DIRECT) {
    return paddr_read(addr, len);
  }else {
    vaddr_t trans_res = isa_mmu_translate(addr, len, MEM_TYPE_IFETCH);
    // printf("res is 0x%x\n", trans_res | (addr&TEMP_MASK));
    // assert(trans_res == (addr&(~TEMP_MASK)));
    return paddr_read(trans_res | (addr&TEMP_MASK), len);
  }
}

word_t vaddr_read(vaddr_t addr, int len) {
  // printf("read_addr is 0x%x\n", addr);
  if(isa_mmu_check(addr, len, MEM_TYPE_READ) == MMU_DIRECT) {
    return paddr_read(addr, len);
  }else {
    return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_READ) | (addr&TEMP_MASK), len);
  }
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  if(isa_mmu_check(addr, len, MEM_TYPE_WRITE) == MMU_DIRECT) {
    paddr_write(addr, len, data);
  }else {
    paddr_write(isa_mmu_translate(addr, len, MEM_TYPE_WRITE) | (addr&TEMP_MASK), len, data);
  }
}
