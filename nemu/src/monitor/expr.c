#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,
    NUM, HEX, REG, VAR,
    DEREF, AND, OR, NOT, EQ, NEQ,
	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"\\+", '+'},
    {"\\-",'-'},
    {"\\*",'*'},
    {"\\/",'/'},
    {"\\(",'('},
    {"\\)",')'},

    {"==",             EQ},    // 等于
    {"!=",             NEQ},   // 不等
    {"&&",             AND},   // 逻辑与
    {"\\|\\|",         OR},    // 逻辑或
    {"!",              NOT},   // 逻辑非
    {"\\*",             DEREF},   // 引用

    {"0x[0-9a-fA-F]+", HEX},   // 十六进制
    {"[0-9]+",         NUM},   // 十进制
    {"\\$[a-z]+",      REG},   // 寄存器 $eax
    {"[a-zA-Z_][a-zA-Z0-9_]*", VAR},  // 变量名

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))//rules长度

static regex_t re[NR_REGEX];//

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;
int check_parentheses(int p, int q);
  int find_op(int p, int q);
  int get_prio(int type);

int eval(uint32_t p, uint32_t q){

    if(p > q) {
        return 0;
    } else if(p == q) {
        bool ok;
        if (tokens[p].type == HEX) {
            return strtol(tokens[p].str, NULL, 16);
        } else if (tokens[p].type == REG) {
            return get_reg_val(tokens[p].str + 1, &ok);
        } else if (tokens[p].type == NUM) {
            return atoi(tokens[p].str);
        } else if (tokens[p].type == VAR) {
            return 0;  // TODO: symbol table lookup
        }
        return 0;
    } else if (check_parentheses(p, q)) {
        return eval(p + 1, q - 1);
    } else {
        // 单目运算符: DEREF / NOT
        if (tokens[p].type == DEREF || tokens[p].type == NOT) {
            uint32_t val = eval(p + 1, q);
            if (tokens[p].type == DEREF) return vaddr_read(val, SREG_DS, 4);
            if (tokens[p].type == NOT)   return !val;
        }
        // 双目运算符
        int op = find_op(p, q);
        if (op < 0) return 0;
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);
        switch (tokens[op].type) {
            case '+':  return val1 + val2;
            case '-':  return val1 - val2;
            case '*':  return val1 * val2;
            case '/':  return val1 / val2;
            case EQ:   return val1 == val2;
            case NEQ:  return val1 != val2;
            case AND:  return val1 && val2;
            case OR:   return val1 || val2;
            default:   assert(0); return 0;
        }
    }
}

//uint32_t find_op(int p,int q){
//    int dom;
//    int prioty = 1000000000000;
//    for(int i=nr_token -1;i>0;i--){
//
//        int tmp_prioty = get_prioty(tokens,i);
//        if(tmp_prioty<=prioty){
//            dom = i;
//            prioty = tmp_prioty;
//        }
//    }
//    return  dom;
//}

int find_op(int p, int q) {
    int op = -1;
    int lowest = 999;
    int depth = 0;
    for (int i = p; i <= q; i++) {
        if (tokens[i].type == '(')      { depth++; continue; }
        if (tokens[i].type == ')')      { depth--; continue; }
        if (depth > 0)                   continue;
        int prio = get_prio(tokens[i].type);
        if (prio > 0 && prio <= lowest) {
            lowest = prio;
            op = i;
        }
    }
    return op;
}


//int get_prio(int type){
//    switch (type) {
//        case '+': return 1;
//        case '-': return 1;
//        case '*': return 2;
//        case '/': return 2;
//    }
//    return 0;
//}
int get_prio(int type) {
    switch (type) {
        case NOT:  return 4;   // 单目最高（但由 eval 直接处理，不走 find_op）
        case DEREF: return 3;
        case '*': return 3;
        case '/': return 3;
        case '+': return 2;
        case '-': return 2;
        case EQ: return 1;
        case NEQ:  return 1;
        case AND: return 0;
        case OR:  return -1;
        default:  return -1;  // 不是运算符
    }
}


int check_parentheses(int p, int q){
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }

    int depth = 0;
    for (int i = p; i <= q; i++) {
        if (tokens[i].type == '(') depth++;
        if (tokens[i].type == ')') depth--;

        // 在中途深度降到 0 → 外层括号提前关闭了
        if (depth == 0 && i != q) {
            return false;
        }
        // 右括号多了 → 非法
        if (depth < 0) {
            return false;
        }
    }
    return depth == 0;
}

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				printf("match regex[%d] at position %d with len %d: %.*s", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */
                switch (rules[i].token_type)
                {
                    case NOTYPE:
                        break;                               // 空格，丢弃
                    default:
                        tokens[nr_token].type = rules[i].token_type;
                        substr_len =  substr_len < 32 ? substr_len : 31;
                        strncpy(tokens[nr_token].str, substr_start,substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                        nr_token++;
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

    /* TODO: Implement code to evaluate the expression. */
    for (int i = 0; i < nr_token; i++) {
        if (tokens[i].type == '*') {
            if (i == 0
                || tokens[i - 1].type == '+'
                || tokens[i - 1].type == '-'
                || tokens[i - 1].type == '*'
                || tokens[i - 1].type == '/'
                || tokens[i - 1].type == '('
                || tokens[i - 1].type == EQ
                || tokens[i - 1].type == NEQ
                || tokens[i - 1].type == AND
                || tokens[i - 1].type == OR
                || tokens[i - 1].type == NOT) {
                tokens[i].type = DEREF;
            }
        }
    }

    *success = true;
    return eval(0, nr_token - 1);
}

