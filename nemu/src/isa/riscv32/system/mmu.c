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
#include <memory/vaddr.h>

#define _HIGH_T(addr) ((uintptr_t)addr>>22)
#define _LOW_T(addr) (((uintptr_t)addr & (0x003ff000)) >> 12)
#define _PNN_MASK (0xfffff000)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  // printf("vaddr is 0x%x\n", vaddr);
  // if((vaddr&(~_PNN_MASK))+len > 0x1000) {
  //   return MEM_RET_CROSS_PAGE;
  // }
  Assert((vaddr&(~_PNN_MASK))+len <= 0x1000, "When %d, at addr 0x%x, with len %d, encounter CROSS_PAGE", type, vaddr, len);
  vaddr_t ptr = cpu.csr.satp << 12;
  vaddr_t first_page = (ptr&_PNN_MASK) | ((_HIGH_T(vaddr))<<2);
  uint32_t first_val = paddr_read(first_page, 4);
  // printf("firstval is 0x%x\n", first_val);
  // printf("vaddr is 0x%x, first_val is 0x%x\n", vaddr, first_val);
  vaddr_t second_page = (first_val&_PNN_MASK) | ((_LOW_T(vaddr))<<2);
  // printf("secondpage is 0x%x\n", second_page);
  uint32_t second_val = paddr_read(second_page, 4);
  // if(vaddr <= 0x80000000) {
  // }
  // printf("enter is 0x%x, res is 0x%x\n", vaddr, second_val);
  // printf("page is 0x%x, second_val is 0x%x\n", second_page, second_val);
  return MEM_RET_OK | (second_val&_PNN_MASK);
}
