#ifndef _GENERATE_MID_H
#define _GENERATE_MID_H
typedef struct {
	char name[40]; // 储存变量的名字
	int type; // 为0表示void，为1表示char类型，为2表示int类型
	int lev; // 储存变量的层次
	//值不是在程序本身就能计算出来的
}gentity;

typedef struct {
	char name[40]; // 储存函数的名字
	int type; // 为0表示void，为1表示char类型，为2表示int类型
	int para_number = 0; // 统计参数表的个数
	int var_number = 0;
	int hascall = 0;
}gfunction;
void generate_mid(); // 程序
void constantDeclare(); // 常量声明
void constantDefine(); // 常量定义
void integer(); // 整数
void variableDeclare(); // 变量声明
void variableDefine(); // 变量定义
void refunction(); // 有返回值函数定义
void headState(); // 声明头部
void vofunction(); // 无返回值函数定义
void parameterList(); // 参数表
void compoundStatement(); // 复合语句
void statementColumn(); // 语句列
void statement(); // 语句
void ifStatement(); // 条件语句
void condition(); // 条件
int expression(); // 表达式
int term(); // 项
int factor(); // 因子
void refunctionCall(); // 有返回值函数调用语句
void vofunctionCall(); // 无返回值函数调用语句
void assignStatement(); // 赋值语句
void valueList(); // 值参数表
void loopStatement(); // 循环语句
void scanStatement(); // 读语句
void printStatement(); // 写语句
void returnStatement(); // 返回语句
void mainFunction(); //主函数
void step(); // 步长
void cString(); // 字符串
void uninteger(); // 无符号整数
int isStatement(); // 判断是否是语句
int isreOperator(); // 判断是否是关系比较符
int isrefuncCall(); // 看是否是有返回值的函数调用
int isvofuncCall(); // 看是否是无返回值的函数调用
void print(); // 单独处理主函数的格式输出
void error(); // 异常处理，暂时没有用
void begin();
void end();
void addpara(char type[100], char idenfr[100]);
void assign_conditiontype(char oper[100]);
void condition_print(int type);
void condition_print1(int ct1, int ct2, int typetemp);
#endif /* _GENERATE_MID_H */
