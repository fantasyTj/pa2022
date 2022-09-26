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
#include <memory/paddr.h> // can i add this .h file?

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_NEQ, TK_DEF, TK_NEG, TK_REG, TK_AND, TK_HEX,

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
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // and
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"\\(", '('},         // left parenthesis
  {"\\)", ')'},         // right parenthesis
  {"[1-9][0-9]*", TK_NUM},   // number
  {"0x[0-9]+", TK_HEX}, // hex number
  {"\\$[a-z0-9]{1,3}", TK_REG},  // register
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

static Token tokens[64] __attribute__((used)) = {};
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

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          /*Assume that we won't encounter situations such as number overflow, so just simplify it*/
          // case TK_NUM:{ /* This may be modifided for the change of strategy for overflow number */
          //   tokens[nr_token].type = TK_NUM;
          //   int occ_count = (substr_len - 1) / 31 + 1;
          //   int pad_count = occ_count * 31 - substr_len;
            
          //   for(int i = occ_count; i > 0; i--){
          //     if(i == occ_count){
          //       tokens[nr_token].type = occ_count;
          //       char tmp_str[32] = "0";
          //       for(int j = 0; j < pad_count; j++) tmp_str[j] = '0';
          //       strncat(tmp_str, substr_start, 31 - pad_count);
          //       strncpy(tokens[nr_token].str, tmp_str, 31);
          //       substr_start += 31 - pad_count;
          //       // printf("num_str is: %s\n",tokens[nr_token].str);
          //       nr_token += 1;
          //     }
          //     else{
          //       tokens[nr_token].type = occ_count;
          //       strncpy(tokens[nr_token].str, substr_start, 31);
          //       substr_start += 31;
          //       nr_token += 1;
          //     }
          //   }
          // }; break;
          case '+': case '-': case '*': case '/': case '(': case ')':{
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = rules[i].token_type;
            nr_token += 1;
          } break;
          case TK_EQ: case TK_NEQ: case TK_AND: case TK_HEX: case TK_REG: case TK_NUM:{
            tokens[nr_token].type = rules[i].token_type;
            Assert(substr_len < 31, "Can't be stored in 32-bit str! (str is %.*s)\n", substr_len, substr_start);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
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

int prio_check(char op){
  if(op == '+' || op == '-') return 5;
  else if(op == '*' || op == '/') return 7;
  else if(op == TK_AND) return 3;
  else return 1;
}

bool checkparenthesis(int p, int q){
  if(tokens[q].type != ')' || p == q) return false;
  else{
    int count = 1;
    int left_pos = q - 1;
    while(left_pos >= 0 ){
      switch(tokens[left_pos].type){
        case '(': count--; break;
        case ')': count++; break;
        default: break;
      }
      if(count == 0) break;
      left_pos--;
    }
    printf("left_pos is %d\n", left_pos);
    return (p == left_pos)?(true):(false);
  }
}

bool checkop(char op){
  if(op == '+'||op == '-'||op == '*'||op == '/'||op == TK_AND||op == TK_EQ||op == TK_NEQ){
    return true;
  }else return false;
}

word_t eval(int p, int q, bool *success){
  Assert(p <= q, "Eval Error!\n");
  // printf("p is %d, q is %d\t", p, q);
  // if(p > q){ /* (Maybe) Can't solve the situation when minus located in the begin */
  //   printf("stream 1\n");
  //   if(p < nr_token && tokens[p].type == '-') {
  //     return 0;
  //   }
  //   printf("Wrong expression\n");
  //   assert(0);
  // }
  if(p == q && (tokens[p].type == TK_NUM || tokens[p].type == TK_HEX)){
  // else if((tokens[q].type == tokens[p].type) && (p + tokens[p].type - 1 == q)){
    // printf("stream 2\n");
    // int num;
    // int occ_count = tokens[p].type;
    // char num_str[occ_count * 32];
    // for(int j = 0; j < occ_count; j++){
    //   if(j == 0) strncpy(num_str, tokens[p + j].str, 31);
    //   else strncat(num_str, tokens[p + j].str, 31);
    // }
    // sscanf(num_str, "%d", &num);
    // *success = true;
    // return num;
    word_t num;
    if(tokens[p].type == TK_NUM) sscanf(tokens[p].str, "%d", &num);
    else sscanf(tokens[p].str, "%x", &num);
    return num;
  }else if(checkparenthesis(p, q)){
    // printf("stream 3\n");
    *success = true;
    return eval(p+1, q-1, success);
  }else{
    // printf("stream 4\n");
    int position = q;
    int main_pos = -1;
    while(position >= p){ /* find the "main" position */
      if(tokens[position].type == ')'){
        int para_count = 1;
        do{
          position = position - 1;
          if(tokens[position].type == '(') para_count -= 1;
          if(tokens[position].type == ')') para_count += 1;
        }while(para_count > 0);
        position = position - 1;
      }else if(checkop(tokens[position].type)){
        if(main_pos == -1) main_pos = position;
        else if(prio_check(tokens[main_pos].type) > prio_check(tokens[position].type)) main_pos = position;
        position--;
      }else position--;
      // else{
      //   position = position - tokens[position].type;
      // }
    }
    if(main_pos == -1){
      if(tokens[p].type == TK_DEF){
        word_t def_data;
        word_t addr;
        if(q == p + 1){
          if(tokens[q].type == TK_NUM) sscanf(tokens[q].str, "%d", &addr);
          else sscanf(tokens[q].str, "%x", &addr);
        }else{
          Assert(checkparenthesis(p + 1, q), "Defer Error!\n");
          addr = eval(p + 1, q, success);
        }
        def_data = paddr_read(addr, 4);
        return def_data;
      }else if(tokens[p].type == TK_NEG){
        int flag = -1;
        int tmp_pos = p + 1;
        int tmp_res;
        while(tokens[tmp_pos].type == TK_NEG){
          flag *= -1;
          tmp_pos += 1;
        }
        tmp_res = flag * eval(tmp_pos, q, success);
        return tmp_res;
      }
    }else{
      word_t result;
      word_t left = eval(p, main_pos - 1, success);
      int right = eval(main_pos + 1, q, success);
      *success = true;
      switch(tokens[main_pos].type){
        case '+':{
          if(right < 0) result = left - (word_t)(-right);
          else result = left + (word_t)right;
        }break;
        case '-':{
          if(right < 0) result = left + (word_t)(-right);
          else result = left - (word_t)right;
        }break;
        case '*': result = left * (word_t)right;break;
        case '/': {
          Assert(right != 0, "ZeroDevisionError!\n");
          result = left / (word_t)right;
        } break;
        case TK_AND: result = left && right;break;
        case TK_EQ: result = (left == right);break;
        case TK_NEQ: result = (left != right);break;
        default: assert(0);
      }
    return result;
    }
  }
  return 0;
}

// char op_type = (main_pos != -1)?(tokens[main_pos].type):(0);
    // int main_pos_left = main_pos, main_pos_right = main_pos;
    // if(tokens[main_pos].type == '-'){ /* Handle continuous minus type */
    //   int flag = -1, front_flag = -1;
    //   while(main_pos_left > 0 && flag){
    //     front_flag = flag;
    //     switch(tokens[--main_pos_left].type){
    //       case '+': flag *= 1; break;
    //       case '-': flag *= (-1); break;
    //       default: flag *= 0; break;
    //     }
    //   }
    //   front_flag = (flag != 0)?(flag):(front_flag);
    //   main_pos_left = (flag != 0)?(main_pos_left):(main_pos_left + 1);
    //   op_type = (front_flag == -1)?('-'):('+'); 
    // }

word_t expr(char *e, bool *success) {

  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // for(int i = 0; i < nr_token; i++){
  //   printf("i: %d, %s\n", i, tokens[i].str);
  // }
  // printf("here\n");
  /* TODO: Insert codes to evaluate the expression. */
  /*pretreat*/
  for(int i = 0; i < nr_token; i++){
    if(tokens[i].type == '*' && (i == 0 || (tokens[i-1].type != TK_NUM || tokens[i-1].type != TK_HEX || tokens[i-1].type != ')'))){
      tokens[i].type = TK_DEF;
    }
    if(tokens[i].type == '-' && (i == 0 || (tokens[i-1].type != TK_NUM || tokens[i-1].type != TK_HEX || tokens[i-1].type != ')'))){
      tokens[i].type = TK_NEG;
    }
    if(tokens[i].type == TK_REG){
      bool success;
      tokens[i].type = TK_NUM;
      word_t tmp_num = isa_reg_str2val(tokens[i].str, &success);
      Assert(success == true, "Register %s not found!\n", tokens[i].str);
      sprintf(tokens[i].str, "%d", tmp_num);
    }
  }

  word_t result;
  result = eval(0, nr_token-1, success);
  Assert(*success == true, "Eval Error!\n");

  return result;
}
