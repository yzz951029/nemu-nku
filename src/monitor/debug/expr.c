#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define max_string_length 32
#define max_token_num 32

enum {
	TK_NOTYPE = 256, TK_EQ,NEQ,NUMBER,HNUMBER,REGISTER,AND,OR,POINTER,MINUS

	/* TODO: Add more token types */


};


static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"\\b[0-9]+\\b",NUMBER,0},
	{"\\b0[xX][0-9A-Fa-f]+\\b",HNUMBER,0},
	{"\\$[A-Za-z]+",REGISTER,0},
	{"	+",TK_NOTYPE,0},	//tabs
	{" +", TK_NOTYPE,0},    // spaces
	{"\\+", '+',4},	// plus
	{"\\*",'*',5},
	{"/",'/',5},
	{"==", TK_EQ,3},	// equal
	{"!=",NEQ,3},
	{"!",'!',},
	{"\\-",'-',4},
	{"\\(",'(',7},
	{"\\)",')',7},
	{"&&",AND,2},
	{"\\|\\|",OR,1},
	{"!",'!',6},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
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

typedef struct token_t {
	int type;
	char str[max_string_length];
	int priority;
} Token;

Token token[max_token_num];
int nr_token;

static bool make_token(char *e)
{
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
				switch(rules[i].token_type) {
				case TK_NOTYPE:
					break;
				case REGISTER://处理掉寄存器的$
					token[nr_token].type = rules[i].token_type;
					token[nr_token].priority = rules[i].priority;
					strncpy (token[nr_token].str,substr_start+1,substr_len-1);
					token [nr_token].str[substr_len-1]='\0';
					nr_token ++;
					break;
				default:
					token[nr_token].type = rules[i].token_type;
					token[nr_token].priority = rules[i].priority;
					strncpy (token[nr_token].str,substr_start,substr_len);
					token [nr_token].str[substr_len]='\0';
					nr_token ++;
					break;
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

bool check_parentheses(int l,int r)
{
	if(l>=r)
		return false;
	if(token[l].type!='('||token[r].type!=')')
		return false;
	else{
	  int cnt = 0;
		for(int i=l+1; i<r; i++) {
			if(token[i].type=='(')cnt++;
			if(token[i].type==')')cnt--;
			if(cnt<0) return false;
		}
	}
	return true;
}

int dominant_operator(int l,int r)
{
	int i,j;
	int min_priority = 10;
	int oper = l;
	for (i = l; i <= r; i ++) {
		if (token[i].type == NUMBER || token[i].type == HNUMBER || token[i].type == REGISTER )//跳过非运算符
			continue;
		int cnt = 0;
		bool key = true;
		for (j = i - 1; j >= l ; j --) {
			if (token[j].type == '(' && cnt==0) {//处理括号包围的情况
				key = false;
				break;
			}
			if (token[j].type == '(')cnt --;
			if (token[j].type == ')')cnt ++;
		}
		if (!key)continue;
		if (token[i].priority <= min_priority) {//选出运算符级别最低的运算符
			min_priority = token[i].priority;
			oper = i;
		}
	}
	return oper;
}

uint32_t eval(int l,int r)
{
	if(l>r) {
		Assert(l>r,"in expr calculation!\n");
		return 0;
	}
	if(l==r) {
		uint32_t num = 0;
		if(token[l].type==NUMBER)
			sscanf(token[l].str,"%d",&num);
		if(token[l].type==HNUMBER)
			sscanf(token[l].str,"%x",&num);
		if(token[l].type==REGISTER) {
			if(strlen(token[l].str)==3) { //32位寄存器
				int i;
				for(i=R_EAX; i<=R_EDI; i++) {
					if(strcmp(regsl[i],token[l].str)==0)
					  break;
				}
				if(i>R_EDI)
					  if (strcmp (token[l].str,"eip") == 0)
						num = cpu.eip;
					  else Assert(1,"no this register!\n");
					else
					  num = reg_l(i);

			} else if ( strlen( token[l].str ) == 2 ) {//16位或者8位
				if ( token[l].str[1] == 'x' || token[l].str[1] == 'p' || token[l].str[1] == 'i' ) {//16位
					int i;
					for ( i = R_AX; i <= R_DI; i++ )
						if ( strcmp( token[l].str, regsw[i] ) == 0 )
							break;
					num = reg_w( i );
				} else if ( token[l].str[1] == 'l' || token[l].str[1] == 'h' ) {//8位
					int i;
					for ( i = R_AL; i <= R_BH; i++ )
						if ( strcmp( token[l].str, regsb[i] ) == 0 )
							break;
					num = reg_b( i );
				} else assert( 0 );
			}
		}
		return num;
	} else if(check_parentheses(l,r)==true) {
		return eval(l+1,r-1);
	} else {
		int op = dominant_operator(l,r);
		if(op==l) {
			uint32_t val = eval(l+1,r);
			val = val;
			switch(token[op].type) {
			case MINUS:
				return -val;
			case POINTER:
				return vaddr_read(val,4);
			case '!':
				return !val;
			default:
			  printf("l:%d,r:%d\n",l,r);
			  printf("op:%d token_type:%d\n",op,token[op].type);
			  assert(0);
			}
		} else {
		  uint32_t lval = eval(l,op-1);
		  //printf("lval: %d",lval);
		  lval=lval;
		  uint32_t rval = eval(op+1,r);
		  //printf("rval: %d",rval);
		  rval = rval;
		  switch(token[op].type) {
		  case '+':
		    return lval+rval;
		  case '-':
		    return lval-rval;
		  case '*':
		    return lval*rval;
		  case '/':
		    return lval/rval;
		  case TK_EQ:
		    return lval==rval;
		  case NEQ:
		    return lval!=rval;
		  case AND:
		    return lval&&rval;
		  case OR:
		    return lval||rval;
		  default:
		    break;
			}
		}

	}
	return -123456;
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e)) {
		*success = false;
		return 0;
	}
	int i;
	for (i = 0; i < nr_token; i ++) {
		if (token[i].type == '*' && (i == 0 || (token[i - 1].type != NUMBER && token[i - 1].type != HNUMBER && token[i - 1].type != REGISTER && token[i - 1].type !=')'))) {
			token[i].type = POINTER;
			token[i].priority = 6;
		}
		if (token[i].type == '-' && (i == 0 || (token[i - 1].type != NUMBER && token[i - 1].type != HNUMBER && token[i - 1].type != REGISTER && token[i - 1].type !=')'))) {
			token[i].type = MINUS;
			token[i].priority = 6;
		}
	}
	/* TODO: Insert codes to evaluate the expression. */
	*success = true;


	return eval(0,nr_token-1);
}
