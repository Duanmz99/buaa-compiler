/*
#ifndef _ERROR_H
#define _ERROR_H
typedef struct {
	int type; // 1为有值函数，2为无值函数，3为const，4为变量
	int lev; // 表示位于当前堆栈的第几层
	int dim; // 如果是变量的话记录有几层
	char name[40]; // 记录此实体的名字
	int value; // 为1表示是整型，否则表示是字符
}eneity;

typedef struct {
	int number; // 表示此函数有多少个参数
	int type[10]; // 记录每个参数的类型，1为int，0为char
	char name[40]; // 记录此函数的名字
	char value[40]; // 记录返回值的类型
}function;

void error_a(); // 完成
int error_b(char addname[40]); // 完成
void error_c(); // 本次评测机没考，最后整
void error_d(char temp[40], int time); // 完成
void error_e(char temp[40], int time); // 完成
void error_f(int expreType); // 完成
void error_g(); // 完成
void error_h(); // 完成
void error_i(int expreType); // 完成（但是判断是否有重复的地方存在问题，需要进行修改）
void error_j(); // 完成
void error_k(); // 完成
void error_l(); // 完成
void error_m(); // 完成
void error_n(); // 完成
void error_o(); // 完成
#endif /* _ERROR_H */
