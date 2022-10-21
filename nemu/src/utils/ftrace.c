// ftrace

// #ifdef CONFIG_FTRACE

#include <elf.h>
#include <common.h>

#define MAX_FTRACE_CAPACITY 128

extern uint32_t symtab_size;
extern uint32_t symtab_off;
extern Elf32_Sym symtab[128];

extern uint32_t strtab_size;
extern uint32_t strtab_off;
extern char strtab[512];

// complete ftrace
typedef struct Func{
  char func_name[32];
  paddr_t func_addr;
} Func;
static Func fc[100];
static uint16_t nr_func = 0;

typedef struct Ftarce{
  uint8_t call_or_ret; // call -> 0, ret -> 1
  char func_name[32];
  paddr_t func_addr;
  paddr_t caller_addr;
} Ftarce;
static uint16_t nr_ftrace = 0;
static Ftarce ftc[MAX_FTRACE_CAPACITY];

void init_ftrace(){
    uint32_t symnum = symtab_size / sizeof(Elf32_Sym);
    for(uint32_t i = 0; i < symnum; i++){
        if(ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC){
          fc[nr_func].func_addr = symtab[i].st_value;
          strcpy(fc[nr_func].func_name, strtab + symtab[i].st_name);
          nr_func++; 
      }
    }
}

void ftrace(paddr_t pc, paddr_t dnpc){
    // check "call"
    for(uint16_t i = 0; i < nr_func; i++){
        if(dnpc == fc[i].func_addr){
            ftc[nr_ftrace].call_or_ret = 0; // call
            ftc[nr_ftrace].caller_addr = pc;
            ftc[nr_ftrace].func_addr = dnpc;
            strcpy(ftc[nr_ftrace].func_name, fc[i].func_name);
            nr_ftrace++;
            return;
        }
    }

    for(uint16_t i = 0; i < nr_ftrace; i++){
        if(dnpc == ftc[i].caller_addr + 4){
            ftc[nr_ftrace].call_or_ret = 1; // ret
            ftc[nr_ftrace].caller_addr = pc;
            ftc[nr_ftrace].func_addr = ftc[i].func_addr;
            strcpy(ftc[nr_ftrace].func_name, ftc[i].func_name);
            nr_ftrace++;
            return;
        }
    }
}

void print_ftrace(){
    uint16_t indent = 0;
    for(uint16_t i = 0; i < nr_ftrace; i++){
        if(ftc[i].call_or_ret == 0){ // call
            printf(FMT_PADDR ":", ftc[i].caller_addr);
            for(uint16_t j = 0; j < indent; j++) putchar(' ');
            printf("call[%s@" FMT_PADDR "]\n", ftc[i].func_name, ftc[i].func_addr);
            indent++;
        }else{
            indent--;
            printf(FMT_PADDR ":", ftc[i].caller_addr);
            for(uint16_t j = 0; j < indent; j++) putchar(' ');
            printf("ret[%s@" FMT_PADDR "]\n", ftc[i].func_name, ftc[i].func_addr);
        }
    }
}

// #endif