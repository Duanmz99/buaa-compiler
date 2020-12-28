# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <iostream>
# include <string>
# include <vector>
# include "analysis.h"
# include "grammer.h"
# include "generate_mid.h"
# include "generate_mips.h"
#include "optimize.h"
using namespace std;

FILE* input = NULL;
FILE* output = NULL;
char sym[100];
char word[100];
char symall[5000][100]; // 改变思路储存全部
char wordall[5000][100];
int wordNumber = 0; // 统计数量
int pos = 0;
int time1 = 0; // 可以考虑在analysis.h里面设为static类型
int line = 1; // 统计行号

int main(void) {
	input = fopen("testfile.txt", "r");
	output = fopen("17373217_段牧知_优化前中间代码.txt", "w");
	// 读入全部单词并储存
	while (!trygetsym()) {
		strcpy(wordall[wordNumber], word);
		strcpy(symall[wordNumber], sym);
		wordNumber++;
	}
	//tryget();
	// 此次先不进行错误处理检查，回头再在此基础上改，因为涉及函数重载的问题
	//program();
	//printf("finish error check!\n");
	// 一切归0开始进行中间代码翻译
	printf("开始生成中间代码\n");
	pos = 0;
	line = 1;
	tryget();
	generate_mid();
	fclose(input);
	fclose(output);
	// 开始转换成最后的mips代码
	optimize();
	// printf("\n\n\n\n\n\n\n\n\n\n开始生成mips代码\n");
	generate_mips();
	return 0;
}
