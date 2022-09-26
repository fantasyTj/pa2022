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
#include "local-include/reg.h"

const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void isa_reg_display()
{

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      int idx = 4 * i + j;
      if (cpu.gpr[idx] == 0)
      {
        printf("%s\t0x%d\t\t", regs[idx], cpu.gpr[idx]);
      }
      else
        printf("%s\t%#x\t\t", regs[idx], cpu.gpr[idx]);
    }
    putchar('\n');
  }
}

word_t isa_reg_str2val(const char *s, bool *success)
{
  if (s[1] != '0') s += 1;
  int idx;
  for (idx = 0; idx < 32; idx++)
  {
    if (strcmp(s, regs[idx]) == 0)
      break;
  }
  printf("idx is %d\n", idx);
  Assert(idx < 32, "Register %s not found!\n", s);
  *success = true;
  return cpu.gpr[idx];
}
