/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "analysis.h"
#include "grammer.h"
#include "error.h"
extern FILE* input;
extern FILE* output;
extern char sym[40]; // 表示类型
extern char word[40]; // 表示读到的词
extern int time; // 

extern char symall[5000][40]; // 改变思路储存全部
extern char wordall[5000][40];
extern int wordNumber;
extern int pos;
extern int line;

char refuncName[80][40]; // 储存有值函数的名字
int relen = 0; // 储存数量
char vofuncName[80][40]; // 储存无值函数的名字
int volen = 0; // 储存数量

// 开始统计建表的问题
eneity namelist[700]; // 记录当前阶段出现的所有变量和函数
function func[50]; // 记录整个程序出现过的函数
int level = 0; // 0对应着全局变量
int nameNumber = 0; // 统计当前层有多少出现过的变量或函数
int funcNumber = 0; // 统计整个程序有几个函数
function currentfunc; // 为了便于后续处理添加全局变量记录当前函数

// 为了处理defghi引入如下变量
//int expreType = 0; // 判断是整型还是非整型，只有为1时为非整型，否则为整型(递归时不能这么整)
int para[10] = { 0 }; // 负责统计引用时插入的各个值参数的类型，辅助判断

int return_exist = 0;
int return_type = 0;
int functype = 0;

//＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void program() {
	// 需要预读时返回指针
	// 储存预读时无法处理的中间变量	
	// 进入常量说明检测阶段
	if (strcmp(sym, "CONSTTK") == 0) {
		constantDeclare();
	}

	// 进入变量说明检测阶段
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		char symTemp[40];
		char wordTemp[40];
		int temp = pos;
		strcpy(symTemp, sym);
		strcpy(wordTemp, word);
		// 往后预读两个判断是有返回值函数还是常量定义
		tryget();  // 应该为try
		tryget();
		if (strcmp(sym, "COMMA") == 0 || strcmp(sym, "LBRACK") == 0 || strcmp(sym, "SEMICN") == 0) {
			// 进入这里时还没有问题
			pos = temp;
			strcpy(sym, symTemp);
			strcpy(word, wordTemp);
			variableDeclare();	// 在这里有问题
		} // 表示有这个环节
		else {
			pos = temp;
			strcpy(sym, symTemp);
			strcpy(word, wordTemp);
		} // 进入下个阶段
	}

	// 开始函数定义部分
	while (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0 || strcmp(sym, "VOIDTK") == 0) {
		if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
			refunction();
		}
		else if (strcmp(sym, "VOIDTK") == 0) {
			// 此时为了与主函数区分需要额外预读一个字符
			char symTemp[40];
			char wordTemp[40];
			int temp = pos;
			strcpy(symTemp, sym);
			strcpy(wordTemp, word);
			tryget();
			if (strcmp(sym, "MAINTK") == 0) { // 是的话直接顺下去先
				pos = temp;
				strcpy(sym, symTemp);
				strcpy(word, wordTemp);
				break;
			}
			else {
				pos = temp;
				strcpy(sym, symTemp);
				strcpy(word, wordTemp);
				vofunction(); // 否则是无返回值函数定义
			}
		}
	}

	// 正式进入主函数部分
	mainFunction();
	//printf("<程序>\n");
	//fprintf(output, "<程序>\n");
	return;
}

// ＜常量说明＞ :: = const＜常量定义＞; { const＜常量定义＞; }
void constantDeclare() {
	if (strcmp(sym, "CONSTTK") == 0) {
		getsym(); // 读入下一个
		constantDefine();
		error_k();
		while (strcmp(sym, "CONSTTK") == 0) {
			getsym(); // 得到后面第一个字符
			constantDefine();
			error_k();
		}
		//printf("<常量说明>\n");
		//fprintf(output, "<常量说明>\n");
	}
}

// ＜常量定义＞   :: = int＜标识符＞＝＜整数＞{ ,＜标识符＞＝＜整数＞ }
// |char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
void constantDefine() {
	if (strcmp(sym, "INTTK") == 0) {
		getsym(); // 读入标识符
		add_int_const();
		getsym(); // 读入等于号
		getsym();
		if (strcmp(sym, "INTCON") != 0 && strcmp(sym, "PLUS") != 0 && strcmp(sym, "MINU") != 0) {
			error_o();
			getsym();
		}
		else
			integer();
		while (strcmp(sym, "COMMA") == 0) {
			getsym();
			add_int_const();
			getsym();
			getsym(); // 得到整数第一项
			if (strcmp(sym, "INTCON") != 0 && strcmp(sym, "PLUS") != 0 && strcmp(sym, "MINU") != 0) {
				error_o();
				getsym();
			}
			else
				integer();
		}
		//printf("<常量定义>\n");
		//fprintf(output, "<常量定义>\n");
	}
	else if (strcmp(sym, "CHARTK") == 0) {
		getsym(); // 读入标识符
		add_char_const();
		getsym(); // 读入等于号
		getsym(); // 读入字符
		error_a();
		if (strcmp(sym, "CHARCON") != 0)
			error_o();
		getsym(); // 看是否是逗号
		while (strcmp(sym, "COMMA") == 0) {
			getsym();
			add_char_const();
			getsym();
			getsym();
			error_a();
			if (strcmp(sym, "CHARCON") != 0)
				error_o();
			getsym();
		}
		//printf("<常量定义>\n");
		//fprintf(output, "<常量定义>\n");
	}
	else error();
}

void integer() {
	if (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0) {
		getsym();
	}
	uninteger();
	//printf("<整数>\n");
	//fprintf(output, "<整数>\n");
}

// ＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
void variableDeclare() {
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		variableDefine();
		error_k();
		while (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
			char symTemp[40];
			char wordTemp[40];
			int temp = pos;
			strcpy(symTemp, sym);
			strcpy(wordTemp, word);
			// 预读两个
			tryget();
			tryget();
			if (strcmp(sym, "COMMA") == 0 || strcmp(sym, "LBRACK") == 0 || strcmp(sym, "SEMICN") == 0) {
				pos = temp;
				strcpy(sym, symTemp);
				strcpy(word, wordTemp);
				variableDefine();
				error_k();
			} // 表示继续变量定义
			else {
				pos = temp;
				strcpy(sym, symTemp);
				strcpy(word, wordTemp);
				break; // 不再是变量说明部分，而是函数定义部分
			}
		}
		//printf("<变量说明>\n");
		//fprintf(output, "<变量说明>\n");
	}
}

// ＜变量定义＞  :: = ＜类型标识符＞(＜标识符＞ | ＜标识符＞'['＜无符号整数＞']') { , (＜标识符＞ | ＜标识符＞'['＜无符号整数＞']') }
// ＜无符号整数＞表示数组元素的个数，其值需大于0
void variableDefine() {
	// 已经保证是int或者char
	char type_temp[40];
	char name_temp[40];
	strcpy(type_temp, sym);
	getsym(); // 读入标识符
	strcpy(name_temp, word);
	int dim = 0;
	int time = 0;
	getsym(); // 不一定是，故此处不输出
	if (strcmp(sym, "LBRACK") == 0) {
		dim = 1;
		getsym(); // 得到整数第一个符号
		//uninteger(); // 读入无符号整数
		time = expression();
		// 此时已经读入右括号
		error_m();
		error_i(time);
	}
	add_var(type_temp, name_temp, dim);
	while (strcmp(sym, "COMMA") == 0) {
		getsym();
		strcpy(name_temp, word);
		dim = 0;
		getsym();
		if (strcmp(sym, "LBRACK") == 0) {
			dim = 1;
			getsym();
			//uninteger(); // 得到无符号整数和下一个大括号
			time = expression();
			error_i(time);
			error_m();
		}
		add_var(type_temp, name_temp, dim);
	}
	//printf("<变量定义>\n");
	//fprintf(output, "<变量定义>\n");
}

// ＜有返回值函数定义＞  :: = ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
void refunction() {
	// 声明头部必然已存在
	return_exist = 0;
	currentfunc.number = 0; // 先初始化
	headState();
	getsym(); // 得到参数表第一个字符
	parameterList();
	func[funcNumber] = currentfunc;
	funcNumber++;
	// 此时已经读完右括号
	error_l();
	getsym(); // 得到复合语句第一个词
	compoundStatement();
	//printf("return_exist:%d\n", return_exist);
	if (return_exist == 0) {
		printf("%d %c\n", line, 'h');
		fprintf(output, "%d %c\n", line, 'h');
	}
	getsym(); // 读取下一个
	//printf("<有返回值函数定义>\n");
	//fprintf(output, "<有返回值函数定义>\n");
	// 已经读到右大括号
}

// ＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
void headState() {
	char type_temp[40];
	strcpy(type_temp, sym);
	strcpy(currentfunc.value, sym);
	if (strcmp(sym, "CHARTK") == 0)
		functype = 1;
	else
		functype = 2;
	getsym(); // 得到有返回值函数的名字
	add_func(1, type_temp);
	strcpy(currentfunc.name, word);
	strcpy(refuncName[relen], word);
	getsym();
	relen++;
	//printf("<声明头部>\n");
	//fprintf(output, "<声明头部>\n");
}

// ＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
void vofunction() {
	return_exist = 0;
	functype = 0;
	char type_temp[40] = "VOIDTK";
	currentfunc.number = 0; // 先初始化
	strcpy(currentfunc.value, sym);
	getsym();
	add_func(2, type_temp);
	strcpy(vofuncName[volen], word);
	strcpy(currentfunc.name, word);
	volen++; // 储存名字
	getsym(); // (
	getsym();
	parameterList();
	func[funcNumber] = currentfunc;
	funcNumber++;
	error_l();
	getsym();
	compoundStatement();
	getsym(); // 读取本段结束的下一个字符
	//printf("<无返回值函数定义>\n");
	//fprintf(output, "<无返回值函数定义>\n");
}

// ＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
void parameterList() {
	level++;
	char name_temp[40];
	char type_temp[40];
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		strcpy(type_temp, sym);
		if (strcmp(sym, "INTTK") == 0)
			currentfunc.type[currentfunc.number] = 1;
		else
			currentfunc.type[currentfunc.number] = 0;
		getsym(); // 得到标识符
		currentfunc.number++;
		strcpy(name_temp, word);
		add_var(type_temp, name_temp, 0);
		getsym();
		while (strcmp(sym, "COMMA") == 0) {
			getsym(); // 得到类型标识符
			strcpy(type_temp, sym);
			if (strcmp(sym, "INTTK") == 0)
				currentfunc.type[currentfunc.number] = 1;
			else
				currentfunc.type[currentfunc.number] = 0;
			currentfunc.number++;
			getsym(); // 得到标识符
			strcpy(name_temp, word);
			add_var(type_temp, name_temp, 0);
			getsym();
		}
	}
	else {
		// 否则直接为空,无需进行任何处理
	}
	//printf("<参数表>\n");
	//fprintf(output, "<参数表>\n");
}

// ＜复合语句＞   :: = ［＜常量说明＞］［＜变量说明＞］＜语句列＞
void compoundStatement() {
	// 常量说明
	if (strcmp(sym, "CONSTTK") == 0) {
		constantDeclare();
	}
	// 变量说明
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		variableDeclare();
	}
	//语句列
	statementColumn();
	level--;
	drop_namelist();
	//printf("<复合语句>\n");
	//fprintf(output, "<复合语句>\n");
}

// ＜语句列＞   ::= ｛＜语句＞｝
void statementColumn() {
	while (isStatement()) {
		statement();
	}
	//printf("<语句列>\n");
	//fprintf(output, "<语句列>\n");
}

// ＜语句＞    :: = ＜条件语句＞｜＜循环语句＞ | '{'＜语句列＞'}' | ＜有返回值函数调用语句＞;
// | ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜返回语句＞;
void statement() { // 每个读到自己语句;后的下一个字符
	if (strcmp(sym, "IFTK") == 0) {
		ifStatement();
	}
	else if (strcmp(sym, "WHILETK") == 0 || strcmp(sym, "DOTK") == 0 || strcmp(sym, "FORTK") == 0) {
		loopStatement();
	}
	else if (strcmp(sym, "LBRACE") == 0) {
		getsym();
		statementColumn();
		getsym();
	}
	else if (strcmp(sym, "IDENFR") == 0) {
		// 此处有三种可能，需要进行判断
		// 有返回值，无返回值，赋值语句
		// 没采用文件指针的方法，此时采用头符号名判断法
		if (isrefuncCall()) {
			refunctionCall();
			error_k();
		}
		else if (isvofuncCall()) {
			vofunctionCall();
			error_k();
		}
		else {
			assignStatement();
			error_k();
		}
	}
	else if (strcmp(sym, "SCANFTK") == 0) {
		scanStatement();
		error_k();
	}
	else if (strcmp(sym, "PRINTFTK") == 0) {
		printStatement(); // 读完;
		error_k();
	}
	else if (strcmp(sym, "SEMICN") == 0) {
		// 空语句
		error_k();
	}
	else if (strcmp(sym, "RETURNTK") == 0) {
		returnStatement();
		error_k();
	}
	//printf("<语句>\n");
	//fprintf(output, "<语句>\n");
}

// ＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
void ifStatement() {
	getsym(); // 读取左括号
	getsym();
	condition();
	// 至此已经读完右括号
	error_l();
	statement();
	if (strcmp(sym, "ELSETK") == 0) {
		getsym(); // 获得下一个语句的第一个词
		statement();
	}
	//printf("<条件语句>\n");
	//fprintf(output, "<条件语句>\n");
}

// ＜条件＞    :: = ＜表达式＞＜关系运算符＞＜表达式＞ //整型表达式之间才能进行关系运算
//｜＜表达式＞    //表达式为整型，其值为0条件为假，值不为0时条件为真      
void condition() {
	int time;
	time = expression();
	error_f(time);
	if (isreOperator()) {
		getsym();
		time = expression();
		error_f(time);
	}
	//printf("<条件>\n");
	//fprintf(output, "<条件>\n");
}

// ＜表达式＞    :: = ［＋｜－］＜项＞{ ＜加法运算符＞＜项＞ }   //[+|-]只作用于第一个<项>
int expression() { // 第一个字符已经读入，结束时输出其本身终止点的下一个字符
	int expreType = 0;
	if (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0)
		getsym();
	expreType += term();
	while (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0) {
		expreType += 2;
		getsym();
		term();
	}
	return expreType;
	//printf("<表达式>\n");
	//fprintf(output, "<表达式>\n");
}

// ＜项＞     :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
int term() {
	int termType = 0;
	termType += factor();
	while (strcmp(sym, "MULT") == 0 || strcmp(sym, "DIV") == 0) {
		termType += 2;
		getsym();
		factor();
	}
	return termType;
	//printf("<项>\n");
	//fprintf(output, "<项>\n");
}

// ＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞         
int factor() {
	int factorType = 0;
	int time;
	if (isrefuncCall()) { // 有返回值函数调用
		refunctionCall();
		factorType += findfuncType(); // char加1，int加2
	}
	else if (strcmp(sym, "IDENFR") == 0) {
		factorType += findvarType();
		getsym();
		if (strcmp(sym, "LBRACK") == 0) { // 正常的表达式,直接顺着往后推
			// 此时需要先输出上一项标识符的内容
			getsym();
			time = expression();
			error_i(time);
			error_m();
		}
		else {
			// 表示是单独的标识符
		}
	}
	else if (strcmp(sym, "LPARENT") == 0) {
		getsym();
		factorType += expression();
		error_l();
	}
	else if (strcmp(sym, "CHARCON") == 0) {
		factorType += 1;
		error_a();
		getsym();
	}
	else if (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0 || strcmp(sym, "INTCON") == 0) {
		integer(); // 自动读取自身结束的后一个字符
		factorType += 2;
	}
	return factorType;
	//else return; // 否则不合规范，已经是下一个字符直接return
	//printf("<因子>\n");
	//fprintf(output, "<因子>\n");
}

// ＜有返回值函数调用语句＞ :: = ＜标识符＞'('＜值参数表＞')'
void refunctionCall() {
	char name_current[40];
	strcpy(name_current, word);
	getsym(); // 读入(
	getsym(); // 读入值参数表第一个字符
	int time;
	time = valueList();
	error_d(name_current, time);
	error_e(name_current, time);
	error_l();
	//printf("<有返回值函数调用语句>\n");
	//fprintf(output, "<有返回值函数调用语句>\n");
}

// ＜无返回值函数调用语句＞ :: = ＜标识符＞'('＜值参数表＞')'
void vofunctionCall() {
	char name_current[40];
	strcpy(name_current, word);
	getsym(); // 读入(
	getsym(); // 读入值参数表第一个字符
	int time;
	time = valueList();
	error_d(name_current, time);
	error_e(name_current, time);
	error_l();
	//printf("<无返回值函数调用语句>\n");
	//fprintf(output, "<无返回值函数调用语句>\n");
}

// ＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
void assignStatement() {
	// 首先是表达式
	int time;
	error_j();
	getsym();
	if (strcmp(sym, "ASSIGN") == 0) {
		getsym();
		expression();
	}
	else if (strcmp(sym, "LBRACK") == 0) {
		getsym();
		time = expression();
		error_i(time);
		error_m();
		getsym(); // 得到表达式第一个符号
		expression();
	}
	//printf("<赋值语句>\n");
	//fprintf(output, "<赋值语句>\n");
}

// ＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞
int valueList() {
	int time = 0;
	int type_temp = 0;
	if (strcmp(sym, "RPARENT") == 0) {
		// 表示为空
	}
	else {
		type_temp = expression(); // 默认必然是值参数表
		if (type_temp > 1)
			para[time] = 1;
		else
			para[time] = 0;
		time++;
		while (strcmp(sym, "COMMA") == 0) {
			getsym();
			type_temp = expression();
			if (type_temp > 1)
				para[time] = 1;
			else
				para[time] = 0;
			time++;
		}
	}
	return time;
	//printf("<值参数表>\n");
	//fprintf(output, "<值参数表>\n");
}

// ＜循环语句＞   :: = while '('＜条件＞')'＜语句＞ 
// | do＜语句＞while '('＜条件＞')' 
// | for'('＜标识符＞＝＜表达式＞; ＜条件＞; ＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
void loopStatement() { // 三个合一起写，不要分开写
	if (strcmp(sym, "WHILETK") == 0) { // while开始
		getsym();
		getsym();
		condition();
		error_l();
		statement(); // 读入语句
	}
	else if (strcmp(sym, "DOTK") == 0) { // do开始
		getsym();
		statement();
		error_n();
		getsym();
		condition();
		error_l();
	}
	else if (strcmp(sym, "FORTK") == 0) { // for开始
		getsym(); // (
		getsym(); // 标识符
		getsym(); // =
		getsym();
		expression();
		error_k();
		condition();
		error_k();
		getsym(); //=
		getsym(); // 标识符
		getsym(); // +-
		getsym();
		step(); // 无符号 整数
		error_l();
		statement(); // 已自动读取下一个
	}
	//printf("<循环语句>\n");
	//fprintf(output, "<循环语句>\n");
}

// ＜读语句＞    ::=  scanf '('＜标识符＞{,＜标识符＞}')'
void scanStatement() {
	getsym(); // (
	getsym();
	getsym();
	while (strcmp(sym, "COMMA") == 0) {
		getsym();
		getsym();
	}
	error_l();
	//printf("<读语句>\n");
	//fprintf(output, "<读语句>\n");
}

// ＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
void printStatement() {
	getsym();
	getsym();
	if (strcmp(sym, "STRCON") == 0) {
		cString();
		if (strcmp(sym, "COMMA") == 0) {
			getsym();
			expression();
		}
	}
	else {
		expression();
	}
	error_l();
	//printf("<写语句>\n");
	//fprintf(output, "<写语句>\n");
}

// ＜返回语句＞   :: = return['('＜表达式＞')']
void returnStatement() {
	return_exist++;
	getsym();
	if (strcmp(sym, "LPARENT") == 0) {
		getsym();
		return_type = expression();
		error_l();
	}
	if (functype == 0) {
		error_g();
	}
	else {
		error_h();
	}
	//printf("<返回语句>\n");
	//fprintf(output, "<返回语句>\n");
}

// ＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
void mainFunction() { // 修改后假设直接从main进来
	getsym();
	functype = 0;
	char type_temp[40] = "VOIDTK";
	add_func(2, type_temp);
	getsym();
	getsym(); // 读完()
	error_l();
	getsym(); // 读入复合语句第一个字符
	compoundStatement();
	// 此时已经是最后一个，无需再读入，需要进行特殊处理
	//print(); 多输出的一个与这里有关
	//printf("<主函数>\n");
	//fprintf(output, "<主函数>\n");
}

// ＜步长＞::= ＜无符号整数＞  
void step() {
	// 已经读入整数
	uninteger(); // 这里应该有个整数
	//printf("<步长>\n");
	//fprintf(output, "<步长>\n");
}

// ＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"
void cString() {
	getsym(); // 读入下一项的第一个字符
	//printf("<字符串>\n");
	//fprintf(output, "<字符串>\n");
}

void uninteger() {
	getsym();
	//printf("<无符号整数>\n");
	//fprintf(output, "<无符号整数>\n");
}

// ＜语句＞    :: = ＜条件语句＞｜＜循环语句＞ | '{'＜语句列＞'}' | ＜有返回值函数调用语句＞;
// | ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜返回语句＞;
int isStatement() {
	if (strcmp(sym, "IFTK") == 0) return 1;
	if (strcmp(sym, "WHILETK") == 0) return 2;
	if (strcmp(sym, "DOTK") == 0) return 2;
	if (strcmp(sym, "FORTK") == 0) return 2;
	if (strcmp(sym, "LBRACE") == 0) return 3;
	if (strcmp(sym, "IDENFR") == 0) return 4; // 有返回值，无返回值，赋值
	if (strcmp(sym, "SCANFTK") == 0) return 5;
	if (strcmp(sym, "PRINTFTK") == 0) return 6;
	if (strcmp(sym, "SEMICN") == 0) return 7; // 空语句仍然需要处理
	if (strcmp(sym, "RETURNTK") == 0) return 8;
	return 0;
}

int isreOperator() { // 是否是关系比较符
	if (strcmp(sym, "LSS") == 0 || strcmp(sym, "LEQ") == 0 || strcmp(sym, "GRE") == 0)
		return 1;
	if (strcmp(sym, "GEQ") == 0 || strcmp(sym, "EQL") == 0 || strcmp(sym, "NEQ") == 0)
		return 1;
	return 0;
}

int isrefuncCall() {
	int i;
	for (i = 0; i < relen; i++) {
		if (strcmp(word, refuncName[i]) == 0)
			return 1;
	}
	return 0;
}

int isvofuncCall() {
	int i;
	for (i = 0; i < volen; i++) {
		if (strcmp(word, vofuncName[i]) == 0)
			return 1;
	}
	return 0;
}

void print() {
	printf("%s %s\n", sym, word);
	fprintf(output, "%s %s\n", sym, word);
}

void error() {
	printf("syntax error!\n");
}

void add_int_const() {
	if (error_b(word)) {
		return;
	}
	namelist[nameNumber].type = 3;
	namelist[nameNumber].dim = 0;
	strcpy(namelist[nameNumber].name, word);
	namelist[nameNumber].lev = level;
	namelist[nameNumber].value = 1;
	nameNumber++;
}

void add_char_const() {
	if (error_b(word)) {
		return;
	}
	namelist[nameNumber].type = 3;
	namelist[nameNumber].dim = 0;
	strcpy(namelist[nameNumber].name, word);
	namelist[nameNumber].lev = level;
	namelist[nameNumber].value = 0;
	nameNumber++;
}

void add_var(char type1[40], char name1[40], int dimension) {
	if (error_b(name1)) {
		return;
	}
	namelist[nameNumber].type = 4;
	namelist[nameNumber].dim = dimension;
	strcpy(namelist[nameNumber].name, name1);
	namelist[nameNumber].lev = level;
	if (strcmp(type1, "INTTK") == 0)
		namelist[nameNumber].value = 1;
	else
		namelist[nameNumber].value = 0;
	nameNumber++;
}

void drop_namelist() {
	for (int i = nameNumber - 1; i >= 0; i--) {
		if (namelist[i].lev > level)
			nameNumber--;
		else
			break;
	}
}

void add_func(int type1, char type2[40]) { // 此处出现问题
	if (error_b(word)) {
		return;
	}
	namelist[nameNumber].type = type1;
	namelist[nameNumber].dim = 0;
	strcpy(namelist[nameNumber].name, word);
	namelist[nameNumber].lev = level;
	if (strcmp(type2, "INTTK") == 0)
		namelist[nameNumber].value = 1;
	else
		namelist[nameNumber].value = 0;
	nameNumber++;
}

int findfuncType() {
	for (int i = 0; i < funcNumber; i++) {
		if (strcmp(func[i].name, word) == 0) {
			if (strcmp(func[i].value, "INTTK") == 0) {
				return 2;
			}
			else
				return 1;
		}
	}
	return 0;
}

int findvarType() {
	for (int i = 0; i < nameNumber; i++) {
		if (strcmp(namelist[i].name, word) == 0)
			return (namelist[i].value + 1);
	}
	return 0;
}
*/