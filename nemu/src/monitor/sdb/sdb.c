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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h> // can i add this .h file?

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single cloth execution", cmd_si},
  { "info", "Print the register status(r) or print the watchpoint(w) information", cmd_info},
  { "x", "Scan memory from EXPR by N", cmd_x},
  { "p", "Expression evaluation", cmd_p},
  { "w", "Set watchpoints", cmd_w},
  { "d", "Delete watchpoints", cmd_d}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){

  char *arg = strtok(NULL, " ");
  int n;

  if(arg == NULL) n = 1;
  else{
    sscanf(arg, "%d", &n);
  }
  
  cpu_exec(n);

  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " ");

  if(arg == NULL){
    printf("Lack expected argument. [r]\\[w]\n");
    return 0;
  }

  switch(*arg){
    case 'r': isa_reg_display(); break;
    case 'w': wp_display();break;
    default: printf("Unknown argument: '%c' , only 'r' or 's' is legal.\n", *arg); break;
  }

  return 0;
}

static int cmd_x(char *args){
  
  char *arg1 = strtok(NULL, " ");
  char *arg2 = strtok(NULL, " ");

  if(arg1 == NULL || arg2 == NULL){
    printf("Lack argument(s). Please command as x N EXPR !\n");
    return 0;
  }

  int n;
  sscanf(arg1, "%d", &n);

  paddr_t l_addr;
  word_t data;

  switch(*arg2){
    case '$':{
      bool success = true;
      l_addr = isa_reg_str2val(arg2, &success);
    } break;
    case '0':{
      sscanf(arg2, "%x", &l_addr);
      break;
    }
    default: printf("Wrong EXPR format!\n");
  }

  /* there is an intersting phenomenon that when i call the pmem_read(l_addr, 1) for 4 times,
  // each time it will return b7 02 00 80. However, when i call pmem_read(l_addr, 4) for 1 time,
  // it will return 800002b7 just the same as nemu presents when calling "si".
  // i think it is because array pmem[]'s elements is type of uint32_t, when i dereference it as uint8_t,
  // this machine with little-endian just presents the exact value it stores as b7 02 00 80, rather than "decorate" it as 800002b7.  
  */

  for(; n>0; n--){
        printf(FMT_WORD ": ", l_addr);
        for(int i = 3; i >= 0; i--){
          // data = pmem_read(l_addr, 1);
          data = vaddr_read(l_addr + i, 1);
          printf("%02x ", data);
        }
        l_addr += 4;
        putchar('\n');
      }

  return 0;

}

void read_exp_by_line(){
  FILE *fp = fopen("input.txt", "r");
  if(fp == NULL) assert(0);

  unsigned result;
  word_t cal_result;
  char exp_str[512];
  bool success;
  while(fscanf(fp, "%u %[^\n]s",&result, exp_str) == 2){
    // printf("result is %u,\n exp is %s\n", result, exp_str);
    cal_result = expr(exp_str, &success);
    if(cal_result == result) printf("succ\n");
    else printf("str fail: %s\n", exp_str);
  }
  fclose(fp);
}

static int cmd_p(char *args){
  // read_exp_by_line();

  char *arg = strtok(NULL, "");
  if(arg == NULL){
    printf("Lack expected argument(s)!\n");
    return 0;
  }
  // printf("arg is %s\n", arg);
  bool success = false;
  word_t result;
  result = expr(arg, &success);

  if(success == true) printf("Result: %u\n", result);
  else printf("Invalid token(s)!\n");

  return 0;
}

static int cmd_w(char *args){
  char *arg = strtok(NULL, "");
  if(arg == NULL){
    printf("Lack expected argument(s)!\n");
    return 0;
  }
  
  bool success;
  add_wp(arg, &success);
  if(success == false) printf("Add watchpoint error!\n");

  return 0;
}

static int cmd_d(char *args){
  char *arg = strtok(NULL, "");
  if(arg == NULL){
    printf("Lack expected argument(s)!\n");
    return 0;
  }

  word_t NO;
  sscanf(arg, "%u", &NO);
  delete_wp(NO);

  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
