#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define max_string_length 32
#define max_token_num 32

enum
{
	TK_NOTYPE = 256, TK_EQ,NEQ,NUMBER,HNUMBER,REGISTER,AND,OR

	/* TODO: Add more token types */


};


static struct rule
{
	char *regex;
	int token_type;
	int priority;
} rules[] =
{

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"\\b[0-9]+\\b",NUMBER,0},
	{"\\b0[xX][0-9A-Fa-f]+\\b",HNUMBER,0},
	{"\\b$[A-Za-z]+",REGISTER,0},
	{"	+",TK_NOTYPE,0},	//tabs
	{" +", TK_NOTYPE,0},    // spaces
	{"\\+", '+',4},	// plus
	{"\\*",'*',5},
	{"/",'/',5},
	{"==", TK_EQ,3},	// equal
	{"!=",NEQ,3},
	{"!",'!',},
	{"\\-",'-',4},
	{"\\(",'(',10},
	{"\\)",')',10},
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

	for (i = 0; i < NR_REGEX; i ++)
		{
			ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
			if (ret != 0)
				{
					regerror(ret, &re[i], error_msg, 128);
					panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
				}
		}
}

typedef struct token_t
{
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

	while (e[position] != '\0')
		{
			/* Try all rules one by one. */
			for (i = 0; i < NR_REGEX; i ++)
				{
					if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
						{
							char *substr_start = e + position;
							int substr_len = pmatch.rm_eo;

							Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
							    i, rules[i].regex, position, substr_len, substr_len, substr_start);
							position += substr_len;

							/* TODO: Now a new token is recognized with rules[i]. Add codes
							 * to record the token in the array `tokens'. For certain types
							 * of tokens, some extra actions should be performed.
							 */
							switch(rule[i].token_type)
								{
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

			if (i == NR_REGEX)
				{
					printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
					return false;
				}
		}

	return true;
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
		{
			*success = false;
			return 0;
		}

	/* TODO: Insert codes to evaluate the expression. */
	TODO();

	return 0;
}
