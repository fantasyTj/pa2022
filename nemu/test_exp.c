#include <stdio.h>
#include <assert.h>
// #include "src/monitor/sdb/sdb.h"

void read_exp_by_line();

int main(){
    read_exp_by_line();
    return 0;
}

void read_exp_by_line(){
  FILE *fp = fopen("input.txt", "r");
  if(fp == NULL) assert(0);

  unsigned result;
  char exp_str[512];
  while(fscanf(fp, "%u %[^\n]s",&result, exp_str) == 2){
    printf("result is %u,\n exp is %s\n", result, exp_str);
  }
}