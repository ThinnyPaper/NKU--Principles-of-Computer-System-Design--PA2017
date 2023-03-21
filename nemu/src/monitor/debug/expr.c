#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_HEX, TK_DEC, TK_REG, TK_EQ, TK_NEQ, TK_AND, TK_OR, TK_RS, TK_LS, TK_GEQ, TK_LEQ, TK_DEREF, TK_NEG

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},                          // spaces
  {"0x[0-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},      // hex number
  {"0|[1-9][0-9]*", TK_DEC},                  // dec number
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG},
  {"\\+", '+'},                               // add
  {"-", '-'},                                 // sub
  {"\\*", '*'},                               // mul
  {"\\/", '/'},                               // div
  {"\\(", '('},                               // lparen
  {"\\)", ')'},                               // rparen
  {"==", TK_EQ},                              // equal
  {"!=", TK_NEQ},                             // not equal
  {"&&", TK_AND},                             // and
  {"\\|\\|", TK_OR},                          // or
  {">>", TK_RS},                              // right shift
  {"<<", TK_LS},                              // left shift
  {"<=", TK_LEQ},                             // less equal
  {">=", TK_GEQ},                             // great equal
  {"<", '<'},                                 // less 
  {">", '>'}                                  // great
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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

Token tokens[32];
int nr_token;

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
          case TK_NOTYPE: 
            break;
				
          default:{
            if(substr_len >= 32) {
              printf("Token length is too long!\n");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            memset(tokens[nr_token].str, 0, 32);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
            //printf("nr_token=%d\n",nr_token);
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

bool check_parentheses(int p,int q){
  // True: 括号包围的子式，且matched
  // False: 非括号包围的式
  // assert(0): unmatched

  int cnt = 0;
  for(int i = p;i < q; i++){
    if(tokens[i].type == '('){
      cnt++;
    }
    else if(tokens[i].type==')'){
      cnt--;
    }
    if(cnt<=0){
      if(cnt<0){  //brankets unmatched
        printf("( and ) unmatched!\n");
        assert(0);
      }
      else{ 
        return false;
      }  //not in the (<expr>) format
    }
  }
  if(cnt != 1 || tokens[q].type!=')'){
    printf("( and ) unmatched!\n");
    assert(0);
  }
  return true;
}





uint32_t get_token_value(Token token) {
  uint32_t ret_value = 0;
    if(token.type == TK_DEC) {
      sscanf(token.str, "%d", &ret_value);
      return ret_value;
    }
    else if(token.type == TK_HEX) {
      sscanf(token.str, "0x%08x", &ret_value);
      return ret_value;
    }
    else if(token.type == TK_REG) {
      char reg[4] = {0};
      sscanf(token.str, "$%s", reg);
      printf("reg:%s\n",reg);
      for(int i = 0; i < 8; i++) {
        printf("reg=%s, regsl[i]=%s\n",reg,regsl[i]);
        if(strcasecmp(reg, regsl[i]) == 0) {
          printf("in\n");
          return cpu.gpr[i]._32;
        }
        else if(strcasecmp(reg, regsw[i]) == 0) {
          return cpu.gpr[i]._16;
        }
        else if(strcasecmp(reg, regsb[i]) == 0) {
          return cpu.gpr[i % 4]._8[i / 4];
        }
        else if(strcasecmp(reg, "eip") == 0) {
          return cpu.eip;
        }
        else {
          printf("Illegal reg!\n");
          assert(0);
        }
      }
    }
    else {
      printf("Illegal value token!\n");
      assert(0);
    }
}

int op_priority(int type) {
  switch(type){
    case TK_NOTYPE:
    case ')':
      return 0;
    case TK_OR:
      return 1;
    case TK_AND:
      return 2;
    case TK_EQ:
    case TK_NEQ:
      return 3;
    case '<':
    case '>':
    case TK_LEQ:
    case TK_GEQ:
      return 4;
    case TK_LS:
    case TK_RS:
      return 5;
    case '+':
    case '-':
      return 6;
    case '*':
    case '/':
      return 7;
    case TK_NEG:
    case TK_DEREF:
      return 8;
    case '(':
      return 9;
  }
}

bool is_number_token(int type) {
  if(type == TK_DEC || type == TK_HEX || type == TK_REG) {
    return true;
  }
  return false;
}

int find_dominant(int p, int q){
  int d=p;
  while(is_number_token(tokens[d].type)) d++;
  
  for(int i=p; p<=q; p++){
    if(is_number_token(tokens[i].type)) continue;
    else if(tokens[i].type == '('){
      int cnt=1;
      while(cnt!=0){
        p++;
        if(tokens[i].type=='(') cnt++;
        else if(tokens[i].type==')') cnt--;
        if(cnt<0){printf("( and ) unmatched!\n"); assert(0);}
      }
    }
    else if(tokens[i].type ==')') p++;
    else{
      if(op_priority(tokens[i].type) <= op_priority(tokens[d].type)) d=i;
    }
  }
  //printf("d=%d\n",d);
  return d;
}



int eval(int p, int q, bool* success) {
  if( success == false){
    return 0;
  }
  //printf("p=%d,q=%d\n",p,q);
  if(p > q) {
    printf("Bad Expression!\n");
    success = false;
    assert(0);
    return 0;
  }
  // only one token
  else if(p == q) {
    return (int)get_token_value(tokens[p]);
  }
  else if(check_parentheses(p, q) == true){
    return eval(p + 1, q - 1, success);
  }
  else {
    //find dominant
    int op = find_dominant(p,q);
    int val1, val2;
    if(tokens[op].type==TK_NEG || tokens[op].type==TK_DEREF){
      val2 = tokens[op].type==TK_NEG ? -eval(op+1, q, success) : vaddr_read(eval(op+1, q, success), 4);
      if(op==p){
        return val2;
      }
      else{
        op--;
        val1 = eval(p, op-1, success);
      }
    }
    else{
      val1 = eval(p, op-1, success);
      val2 = eval(op+1, q, success);
    }

    switch(tokens[op].type) {
      case '+': return(val1 + val2); break;
      case '-': return(val1 - val2); break;
      case '*': return(val1 * val2); break;
      case '/': return(val1 / val2); break;
      case '<': return(val1 < val2); break;
      case '>': return(val1 > val2); break;
      case TK_EQ: return(val1 == val2); break;
      case TK_NEQ: return(val1 != val2); break;
      case TK_AND: return(val1 && val2); break;
      case TK_OR: return(val1 || val2); break;
      case TK_LS: return(val1 << val2); break;
      case TK_RS: return(val1 >> val2); break;
      case TK_LEQ: return(val1 <= val2); break;
      case TK_GEQ: return(val1 >= val2); break;
      default: success = false; return 0;
    }
  }
}





uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // deal with one operand operator
  for(int i = 0; i < nr_token; i++) {
    if(tokens[i].type == '*' || tokens[i].type == '-') {
      if(i == 0 || (tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')')) {
        tokens[i].type = tokens[i].type == '*' ? TK_DEREF : TK_NEG;
      }
    }
  }
  // return the calculated result
  return eval(0, nr_token - 1, success);
}
