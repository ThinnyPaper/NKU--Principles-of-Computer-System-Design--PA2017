#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,

  /* TODO: Add more token types */
  TK_ADD,TK_SUB,TK_TIMES,TK_DIVIDE,TK_NEG,TK_PTR,
  TK_LBRACKET,TK_RBRACKET,
  TK_REG,TK_INT,TK_HEX,
  TK_AND,TK_OR,TK_NEQ,TK_EQ,TK_NOT,
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},         // add
  {"\\-", TK_SUB},         // sub
  {"\\*", TK_TIMES},       // times
  {"\\/", TK_DIVIDE},      // divide
  {"\\(", TK_LBRACKET},    // lbracket
  {"\\)", TK_RBRACKET},    // rbracket
  {"\\$e(([abc]x)|([bsi]p)|([sd]i))",TK_REG}, //register
  {"0[Xx][0-9a-fA-F]+",TK_HEX}, //hexical
  {"[1-9]?[0-9]+",TK_INT}, //int
  {"&&",TK_AND},         //and
  {"\\|\\|",TK_OR}, 	//or
  {"!=",TK_NEQ},	//neq
  {"==", TK_EQ},         // equal
  {"!",TK_NOT},		//NOT
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
int nr_token; //the num of tokens that have already been recognized.

uint32_t s2i(char* s){
	uint32_t i=0,sum=0;
	while(s[i]!='\0'){
		sum=sum*10+s[i]-'0';
		i++;
	}
	//printf("got num:%u\n",sum);
	return sum;
}
uint32_t h2i(char *s){
	uint32_t n;	
	sscanf(s,"0x%X",&n);
	//printf("%s-%u",s,n);
	return n;
}


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
	//judge sub & negative
	if(rules[i].token_type==TK_SUB){
		if(!nr_token||(tokens[nr_token-1].type!=TK_REG&&tokens[nr_token-1].type!=TK_INT&&tokens[nr_token-1].type!=TK_HEX)){
			rules[i].token_type=TK_NEG;
		}
	}
	//judge times & *pointer 
	if(rules[i].token_type==TK_TIMES){
		if(!nr_token||(tokens[nr_token-1].type!=TK_REG&&tokens[nr_token-1].type!=TK_INT&&tokens[nr_token-1].type!=TK_HEX)){
			rules[i].token_type=TK_PTR;
		}
	}

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	tokens[nr_token].type=rules[i].token_type;
	assert(substr_len<32);
	for(int i=0;i<substr_len;i++){
	    tokens[nr_token].str[i]=substr_start[i];	
	}
	tokens[nr_token++].str[substr_len]='\0';
	
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

bool match_parentheses(){
  int cnt=0;
  for(int i=0;i<nr_token;i++){
    if(tokens[i].type==TK_LBRACKET)cnt++;
    if(tokens[i].type==TK_RBRACKET)cnt--;
    if(cnt<0)return false;
  }
  return !cnt;
}

bool check_parentheses(int p,int q){
  if(tokens[p].type==TK_LBRACKET&&tokens[q].type==TK_RBRACKET)return true;
  return false;
}

//Get the Value of register according its name string.
uint32_t getRegValue(char* name){
	for(int i=0,j=0;i<8;i++){
		for(j=0;j<3;j++){
			if(name[j+1]!=regsl[i][j])break;
		}
		if(j==3)return cpu.gpr[i]._32;
	}
	return cpu.eip;
}

uint32_t eval(int p,int q){
  //calculate the value of expression
  if(p>q){
	Assert(0,"Bad Expression.");
  }
  else if(p==q){
	//here can only be an number or register;
	switch(tokens[p].type){
		case TK_REG:
			return getRegValue(tokens[p].str);
		case TK_INT:
			return s2i(tokens[p].str);
		case TK_HEX:
			return h2i(tokens[p].str);
	}
  }else if(check_parentheses(p,q)==true){
	//remove the bracket and continue;
	return eval(p+1,q-1);
  }
  else{
	
	int cnt=0,dominant_pos=0,level=5,op=0;
	// find the dominant op
	for(int i=p;i<=q;i++){
	    if(tokens[i].type==TK_LBRACKET)cnt++;
	    if(tokens[i].type==TK_RBRACKET)cnt--;
	    if(!cnt){
		//Anything in Bracket must not be calculate at first.
		if(tokens[i].type==TK_OR){
			dominant_pos=i;
			level=0;
			break;
		}else if(tokens[i].type==TK_AND&&level>1){
			dominant_pos=i;
			level=2;
		}else if((tokens[i].type==TK_EQ||tokens[i].type==TK_NEQ)&&level>2){
			dominant_pos=i;
			level=2;
		}else if((tokens[i].type==TK_ADD||tokens[i].type==TK_SUB)&&level>3){
	        	dominant_pos=i;
			level=3;
	        }else if((tokens[i].type==TK_TIMES||tokens[i].type==TK_DIVIDE)&&level>4){
			dominant_pos=i;
			level=4;
		}else if((tokens[i].type==TK_NEG||tokens[i].type==TK_NOT||tokens[i].type==TK_PTR)&&level>5){
			dominant_pos=i;
			level=5;
		}
	    }
	}
	//calculate according to op and split.
	uint32_t n;
	op=tokens[dominant_pos].type;
	switch(op){
	  case TK_ADD:	return eval(p,dominant_pos-1)+eval(dominant_pos+1,q);
	  case TK_SUB:	return eval(p,dominant_pos-1)-eval(dominant_pos+1,q);
	  case TK_TIMES:return eval(p,dominant_pos-1)*eval(dominant_pos+1,q);
	  case TK_DIVIDE:
		n=eval(dominant_pos+1,q);
		if(n==0){
			printf("Can't Divide by 0.\n");
			assert(0);
		}		
		return eval(p,dominant_pos-1)/n;
	  case TK_NEG:		return -1*eval(dominant_pos+1,q);
	  case TK_NOT:		return !eval(dominant_pos+1,q);
	  case TK_AND:		return eval(p,dominant_pos-1)&&eval(dominant_pos+1,q);
	  case TK_OR:		return eval(p,dominant_pos-1)||eval(dominant_pos+1,q);
	  case TK_EQ:		return eval(p,dominant_pos-1)==eval(dominant_pos+1,q);
	  case TK_NEQ:		return eval(p,dominant_pos-1)!=eval(dominant_pos+1,q);
	  case TK_PTR:		return vaddr_read(eval(dominant_pos+1,q),4);
	}
  }
  return -1; 
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  if(!match_parentheses()){printf("Invalid Expression\n");success=false;return 0;}

  return eval(0,nr_token-1);
}
























