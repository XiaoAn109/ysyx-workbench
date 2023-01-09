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
#include<memory/vaddr.h>


enum {
  TK_NOTYPE = 256, TK_HEXNUM, TK_NUM, TK_REG, TK_EQ, TK_NOEQ, TK_AND, TK_OR,
  TK_NEG, TK_POS, TK_DEREF

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
  {"0[xX][0-9a-fA-F]+", TK_HEXNUM}, //numbers(hexadecimal)
  {"[0-9]+", TK_NUM},   //numbers(demical)
  {"\\$[$]?[0-9a-z]+", TK_REG}, //register
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // subtract
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"\\(", '('},         //left parentheses
  {"\\)", ')'},         //right parentheses
  {"==", TK_EQ},        // equal
  {"!=", TK_NOEQ},      //not equal
  {"&&", TK_AND},       //logic and
  {"\\|\\|", TK_OR},    //logic or
  {"!", '!'},           //logic not
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

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            // i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_HEXNUM: case TK_NUM: case TK_REG: {
            if(substr_len > 32) assert(0);
            memset(tokens[nr_token].str, '\0', sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token].str, substr_start, substr_len);
          }
          default: {
            tokens[nr_token].type = rules[i].token_type;
            nr_token ++;
            break;
          }
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

/*
static char *typeshow(int type) {
  static char tmp[20];
  switch (type)
  {
  //case TK_NOTYPE: strcpy(tmp, "Space"); break;
  case TK_NUM: strcpy(tmp, "NUM"); break;
  case TK_HEXNUM: strcpy(tmp, "HEX"); break;
  case TK_REG: strcpy(tmp, "REG"); break;
  case TK_NEG: strcpy(tmp, "NEG"); break;
  case TK_DEREF: strcpy(tmp, "REF"); break;
  case '+': strcpy(tmp, "'+'"); break;
  case '-': strcpy(tmp, "'-'"); break;
  case '*': strcpy(tmp, "'*'"); break;
  case '/': strcpy(tmp, "'/'"); break;
  case '(': strcpy(tmp, "'('"); break;
  case ')': strcpy(tmp, "')'"); break;
  case '!': strcpy(tmp, "NOT"); break;
  case TK_EQ: strcpy(tmp, "EQU"); break;
  case TK_NOEQ: strcpy(tmp, "NEQ"); break;
  case TK_AND: strcpy(tmp, "AND"); break;
  case TK_OR: strcpy(tmp, "OR"); break;
  default: break;
  }
  return tmp;
}

static void print_expr() {
  for(int i = 0; i< nr_token; i ++) {
    if(i == nr_token - 1) printf("+-----+\n");
    else printf("+-----");
  }
  for(int i = 0; i< nr_token; i ++) {
    if(i == nr_token - 1) printf("| [%d] |\n", i);
    else printf("| [%d] ", i);
  }
  for(int i = 0; i< nr_token; i ++) {
    if(i == nr_token - 1) printf("| %s |\n", typeshow(tokens[i].type));
    else printf("| %s ", typeshow(tokens[i].type));
  }
  for(int i = 0; i< nr_token; i ++) {
    if(strcmp(typeshow(tokens[i].type), "NUM") == 0) {
      if(i == nr_token - 1) printf("|  %s  |\n", tokens[i].str);
      else printf("|  %s  ", tokens[i].str);
    }
    else if(strcmp(typeshow(tokens[i].type), "HEX") == 0) {
      if(i == nr_token - 1) printf("| %s |\n", tokens[i].str);
      else printf("| %s ", tokens[i].str);
    }
    else if(strcmp(typeshow(tokens[i].type), "REG") == 0) {
      if(i == nr_token - 1) printf("| %s |\n", tokens[i].str);
      else printf("| %s ", tokens[i].str);
    }
    else {
      if(i == nr_token -1) printf("|     |\n");
      else printf("|     ");
    }
  }
  for(int i = 0; i< nr_token; i ++) {
    if(i == nr_token - 1) printf("+-----+\n");
    else printf("+-----");
  }
}
*/

static bool check_parentheses(int p, int q) {
  int count = 0;
  for(int i = p; i <= q; i ++) {
    if(tokens[i].type == '(') count ++;
    if(tokens[i].type == ')') count --;
    Assert(count >= 0, "ERROR: EXPR is illegal!");
  }
  Assert(count == 0, "ERROR: EXPR is illegal!");
  if((tokens[p].type == '(') && (tokens[q].type == ')')) {
    for(int i = p + 1; i < q; i ++) {
      if(tokens[i].type == '(') count ++;
      if(tokens[i].type == ')') count --;
      if(count < 0) return false;
    }
    return true;
  }
  else return false;
}

static int dominant_op(int p, int q) {
  int count = 0;
  int op = -1;
  int priority = 0;
  for(int i = p; i <= q; i ++) {
    if(tokens[i].type == '(') count ++;
    if(tokens[i].type == ')') count --;
    if(count == 0) {
      if(tokens[i].type == TK_OR) {
        if(priority <= 5) {
          op = i;
          priority = 5;
        }
      }
      if(tokens[i].type == TK_AND) {
        if(priority <= 4) {
          op = i;
          priority = 4;
        }
      }
      if(tokens[i].type == TK_EQ || tokens[i].type == TK_NOEQ) {
        if(priority <= 3) {
          op = i;
          priority = 3;
        }
      }
      if(tokens[i].type == '+' || tokens[i].type == '-') {
        if(priority <= 2) {
          op = i;
          priority = 2;
        }
      }
      if(tokens[i].type == '*' || tokens[i].type == '/') {
        if(priority <= 1) {
          op = i;
          priority = 1;
        }
      }
    }
    else if (count < 0) assert(0);
  }
  return op;
}

static uint64_t eval(int p, int q) {
  Assert(p <= q, "ERROR: EXPR is illegal!");
  if(p == q) {
    uint64_t num = 0;
    bool tmp = false;
    switch(tokens[p].type) {
      case TK_HEXNUM: sscanf(tokens[p].str, "%lx", &num); break;
      case TK_NUM: sscanf(tokens[p].str, "%lu", &num); break;
      case TK_REG: {
        num = isa_reg_str2val(tokens[p].str, &tmp);
        if(!tmp) assert(0) ;
        break;
        }
        default: assert(0);
    }
    return num;
  }
  else if(check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  else {
    int op = dominant_op(p, q);
    //single operand
    if(op == -1) {
      if(tokens[p].type == '!') return !eval(p + 1, q);
      if(tokens[p].type == TK_NEG) return -eval(p + 1, q);
      if(tokens[p].type == TK_POS) return eval(p + 1, q);
      if(tokens[p].type == TK_DEREF) {
        return vaddr_read(eval(p + 1, q), 4);
      }
    }
    //double operands
    uint64_t val1 = eval(p, op - 1);
    uint64_t val2 = eval(op + 1, q);
    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': {
      Assert(val2 != 0, "ERROR: Division by zero!");
      return val1 / val2;
    }
    case TK_AND: return val1 && val2;
    case TK_OR: return val1 || val2;
    case TK_EQ: return val1 == val2 ? 1 : 0;
    case TK_NOEQ: return val1 != val2 ? 1 : 0;
    default: assert(0);
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  for(int i = 0; i < nr_token; i ++) {
    if(tokens[i].type == '+' && (i==0 || (tokens[i-1].type != TK_HEXNUM && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_REG && tokens[i-1].type != ')'))) {
      tokens[i].type = TK_POS;
    }
    if(tokens[i].type == '-' && (i==0 || (tokens[i-1].type != TK_HEXNUM && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_REG && tokens[i-1].type != ')'))) {
      tokens[i].type = TK_NEG;
    }
    if(tokens[i].type == '*' && (i==0 || (tokens[i-1].type != TK_HEXNUM && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_REG && tokens[i-1].type != ')'))) {
      tokens[i].type = TK_DEREF;
    }
  }
  uint64_t ans = eval(0, nr_token - 1);
  *success = true;
  //printf("EXPR is surrounded by parentheses ? %d\n", check_parentheses(0,nr_token - 1));
  //printf("dominant operation's position is %d\n", dominant_op(0, nr_token - 1));
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  //print_expr();
  return ans;
}
