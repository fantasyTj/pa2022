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

#include "sdb.h"

#define NR_WP 32
#define WP_EXPR_LEN 128

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char store_expr[WP_EXPR_LEN];
  word_t value;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
  WP *tmp_p;
  Assert(free_ != NULL,"No more free watchpoints! Maximum capacity is %u\n", NR_WP);
  tmp_p = free_;
  free_ = free_->next;
  return tmp_p;
}

void free_wp(WP *wp){
  wp->next = (free_ == NULL)?(NULL):(free_->next);
  free_ = wp;
}

void add_wp(char *inp_expr, bool *success){
  WP* added_wp = new_wp();
  printf("here\n");
  added_wp->next = head;
  printf("here2\n");
  head = added_wp;
  printf("here3\n");
  head->NO = (head->next == NULL)?(1):(head->next->NO + 1);
  printf("here4\n");
  Assert(strlen(inp_expr) < WP_EXPR_LEN, "Too long expression for watchpoint! %u limited!\n", WP_EXPR_LEN);
  strcpy(head->store_expr, inp_expr);
  head->store_expr[strlen(inp_expr)] = '\0';
  
  head->value = expr(inp_expr, success);
  Assert(*success == true, "Invalid watchpoint expression: %s!\n", inp_expr);
}

void delete_wp(word_t inp_NO){
  WP* tmp_p;
  tmp_p = head;
  if(tmp_p->NO == inp_NO){
    head = head->next;
    free_wp(tmp_p);
  }else{
    int flag = 0;
    while(tmp_p->next != NULL){
      if(tmp_p->next->NO == inp_NO){
        flag = 1;
        break;
      }
      tmp_p = tmp_p->next;
    }
    if(flag == 0) printf("watchpoint %d is not found!\n", inp_NO);
    else{
      WP *tmp_next = tmp_p->next;
      tmp_p->next = tmp_p->next->next;
      free_wp(tmp_next);
    }
  }
}


void wp_display(){
  WP *tmp_p = head;
  if(tmp_p == NULL){
    printf("No watchpoints\n");
  }else{
    printf("%.5s\t%s\n", "NUM", "WHAT");
    while(tmp_p != NULL){
      printf("%5d\t%s\n", tmp_p->NO, tmp_p->store_expr);
      tmp_p = tmp_p->next;
    }
  }
}

void scan_wp(){
  WP *tmp_p = head;
  bool success;
  while(tmp_p != NULL){
    word_t tmp_val;
    tmp_val = expr(tmp_p->store_expr, &success);
    Assert(success == true, "Scan watchpoint error when evaluating!\n");
    if(tmp_val != tmp_p->value){
      nemu_state.state = NEMU_STOP;
      printf("Watchpoint %d: %s\nOld value: %u\nNew value: %u", tmp_p->NO, tmp_p->store_expr, tmp_p->value, tmp_val);
      tmp_p->value = tmp_val;
    }
  }
}