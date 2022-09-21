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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
  {"\\(", '('},         // left parenthesis
  {"\\)", ')'},         // right parenthesis
  {"[1-9][0-9]*", TK_NUM},   // number
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_EQ: break;
          case TK_NUM:{
            tokens[nr_token].type = TK_NUM;
            int occ_count = (substr_len - 1) / 31 + 1;
            int pad_count = occ_count * 31 - substr_len;
            
            for(int i = occ_count; i > 0; i--){
              if(i == occ_count){
                for(int j = pad_count; j > 0; j--) tokens[nr_token].str[pad_count - j] = '0';
                strncat(tokens[nr_token].str, substr_start, 31 - pad_count);
                substr_start += 31 - pad_count;
                // tokens[nr_token].str[31] = '\0';
                nr_token += 1;
              }
              else{
                strncpy(tokens[nr_token].str, substr_start, 31);
                substr_start += 31;
                // tokens[nr_token].str[31] = '\0';
                nr_token += 1;
              }
            }
          }; break;
          case '+': case '-': case '*': case '/': case '(': case ')':{
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = rules[i].token_type;
            nr_token += 1;
          } break;
          default: break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

void gen_expr(char *to_eval){
  
  for(int i = 0; i < 32; i++){
    // switch(tokens[i].type){
    //   case '+': case '-': case '*': case '/': case '(': case ')':{
    //     strncat(to_eval, tokens[i].str, 1);
    //   } break;
    //   default:{
    //     int occ_count = tokens[i].type;
    //     for(int j = occ_count; occ_count > 0; occ_count--){
    //       strncat(to_eval, tokens[j].str, 31);
    //     }
    //   }
    // }
    strncat(to_eval, tokens[i].str, 31);
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // for(int i = 0; i < 32; i++){
  //   printf("%d: %s\n", i, tokens[i].str);
  // }
  // *success = true;

  char to_eval[32 * 32];
  gen_expr(to_eval);
  printf("%s\n", to_eval);
  *success = true;

  return 0;
}

