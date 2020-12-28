/*
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "analysis.h"
#include "grammer.h"
#include "error.h"
extern int line;
extern char sym[40];
extern char word[40];
extern FILE* output;

extern eneity namelist[100]; // 记录当前阶段出现的所有变量和函数
extern function func[10]; // 记录整个程序出现过的函数
extern int level;
extern int nameNumber; // 统计当前层有多少出现过的变量或函数
extern int funcNumber; // 统计整个程序有几个函数

extern int return_exist;
extern int return_type;
extern int functype;
extern int para[10];

void error_a() {
	int wrong = 1;
	char ch = word[0];
	if (isdigit(ch))
		wrong = 0;
	else if (isalpha(ch) || ch == '_')
		wrong = 0;
	else if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
		wrong = 0;
	if (wrong == 1) {
		printf("%d %c\n", line, 'a');
		fprintf(output, "%d %c\n", line, 'a');
	}
	return;
}

// 名字重定义
int error_b(char addname[40]) {
	for (int i = 0; i < nameNumber; i++) {
		if (namelist[i].lev == level) {
			if (strcmp(namelist[i].name, addname) == 0) {
				printf("%d %c\n", line, 'b');
				fprintf(output, "%d %c\n", line, 'b');
				return 1;
			}
		}
	}
	return 0;
}

// 未定义的名字
void error_c() {
	// 本次测试评测机没测此项，不管了
	for (int i = 0; i < nameNumber; i++) {
		if (strcmp(namelist[i].name, word) == 0)
			return;
	}
	printf("%d %c\n", line, 'c');
	fprintf(output, "%d %c\n", line, 'c');

}

// 函数参数个数不匹配
void error_d(char temp[40], int time) {
	for (int i = 0; i < funcNumber; i++) {
		if (strcmp(func[i].name, temp) == 0) {
			if (func[i].number != time) {
				printf("%d %c\n", line, 'd');
				fprintf(output, "%d %c\n", line, 'd');
			}
			return;
		} // 只要找到了不管如何都直接退出
	}
}

// 函数参数类型不匹配
void error_e(char temp[40], int time) {
	for (int i = 0; i < funcNumber; i++) {
		if (strcmp(func[i].name, temp) == 0) {
			for (int j = 0; j < time; j++) {
				if (func[i].
					type[j] != para[j]) {
					printf("%d %c\n", line, 'e');
					fprintf(output, "%d %c\n", line, 'e');
				}
			}
			return;
		} // 只要找到了不管如何都直接退出
	}
}

// 条件判断中出现不合法的类型
void error_f(int expreType) {
	if (expreType == 1) {
		printf("%d %c\n", line, 'f');
		fprintf(output, "%d %c\n", line, 'f');
	}
}

// 无返回值的函数存在不匹配的return语句
void error_g() {
	if (return_exist != 0) {
		if (return_type != 0) {
			printf("%d %c\n", line, 'g');
			fprintf(output, "%d %c\n", line, 'g');
		}
	}
}

// 有返回值的函数缺少return语句或存在不匹配的return语句
void error_h() {
	if (return_exist == 0) {
		printf("%d %c\n", line, 'h');
		fprintf(output, "%d %c\n", line, 'h');
	}
	else {
		if (return_type > 1)
			return_type = 2;
		if (return_type != functype) {
			printf("%d %c\n", line, 'h');
			fprintf(output, "%d %c\n", line, 'h');
		}
	}
}

// 数组元素的下标只能是整型表达式
void error_i(int expreType) {
	if (expreType == 1) {
		printf("%d %c\n", line, 'i');
		fprintf(output, "%d %c\n", line, 'i');
	}
}

// 不能改变常量的值
void error_j() {
	for (int i = 0; i < nameNumber; i++) {
		if (strcmp(namelist[i].name, word) == 0) {
			if (namelist[i].type == 3) {
				printf("%d %c\n", line, 'j');
				fprintf(output, "%d %c\n", line, 'j');
				break;
			}
		}
	}
	return;
}

// 应为分号
void error_k() {
	if (strcmp(sym, "SEMICN") != 0) {
		printf("%d %c\n", (line - 1), 'k');
		fprintf(output, "%d %c\n", (line - 1), 'k');
	} // 不一定需要减一，需要具体判断
	else
		getsym();
	return;
}

// 应为右小括号’)’
void error_l() {
	if (strcmp(sym, "RPARENT") != 0) {
		printf("%d %c\n", line, 'l');
		fprintf(output, "%d %c\n", line, 'l');
	}
	else
		getsym();
	return;
}

// 应为右中括号’]’
void error_m() {
	if (strcmp(sym, "RBRACK") != 0) {
		printf("%d %c\n", line, 'm');
		fprintf(output, "%d %c\n", line, 'm');
	}
	else
		getsym();
	return;
}

void error_n() {
	if (strcmp(sym, "WHILETK") != 0) {
		printf("%d %c\n", line, 'n');
		fprintf(output, "%d %c\n", line, 'n');
	}
	else {
		getsym(); // 否则需要向后读一个
	}
	return;
}

// 常量定义中=后面只能是整型或字符型常量
void error_o() {
	printf("%d %c\n", line, 'o');
	fprintf(output, "%d %c\n", line, 'o');
	return;
}
*/