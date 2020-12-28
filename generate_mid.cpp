#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>
#include "analysis.h"
#include "generate_mid.h"
using namespace std;

extern FILE* input;
extern FILE* output;
extern char sym[100]; // 表示类型
extern char word[100]; // 表示读到的词
extern int line;
extern int pos;

char refuncName[80][100]; // 储存有值函数的名字
int relen = 0; // 储存数量
char vofuncName[80][100]; // 储存无值函数的名字
int volen = 0; // 储存数量

// 为了便于统计数字和字符添加的设置
char intcon[100];

// 为了统计标签的相关信息添加的量
int temp_num = 0; // 储存临时变量的数量，先加加再用
int if_cnt = 1; //先用再加加
int while_cnt = 1;
int do_cnt = 1;
int for_cnt = 1;
int string_cnt = 1; // 固定从1开始，为0表示\n字符串
int glevel = 1; // 统计当前层次

// 为了直接处理condition添加的变量
// 1对应==,2对应<，3对应>,4对应<=,5对应>=,6对应!=
int condition_type = 0;
int condition_temp1 = 0;
int condition_temp2 = 0;
// 使用时需要根据BZ还是BNZ进行特判


vector<gentity> para;
vector<gfunction> func;
gentity entity; // 表示当前正在统计的实体
gfunction function; // 表示当前正在统计的函数
vector<string> print_string; // 用于储存后续需要输出的字符串内容


//＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void generate_mid() {
	// 需要预读时返回指针
	// 储存预读时无法处理的中间变量	
	// 进入常量说明检测阶段
	if (strcmp(sym, "CONSTTK") == 0) {
		constantDeclare();
	}

	// 进入变量说明检测阶段
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		char symTemp[100];
		char wordTemp[100];
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
	// 进入函数定义部分之前需要直接进入main语句，不然后续会顺序下去出现问题
	printf("jal main ;\n");
	fprintf(output, "jal main ;\n");
	// 开始函数定义部分
	while (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0 || strcmp(sym, "VOIDTK") == 0) {
		if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
			refunction();
		}
		else if (strcmp(sym, "VOIDTK") == 0) {
			// 此时为了与主函数区分需要额外预读一个字符
			char symTemp[100];
			char wordTemp[100];
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
	// 输出对应函数信息
	/*
	for (int i = 0; i < func.size(); i++) {
		printf("name %s:para %d\n", func[i].name, func[i].para_number);
	}
	*/
	return;
}

// ＜常量说明＞ :: = const＜常量定义＞; { const＜常量定义＞; }
void constantDeclare() {
	if (strcmp(sym, "CONSTTK") == 0) {
		getsym(); // 读入下一个
		constantDefine();
		if (strcmp(sym, "SEMICN") != 0) error(); // 基本没用
		else {
			getsym();
			while (strcmp(sym, "CONSTTK") == 0) {
				getsym(); // 得到后面第一个字符
				constantDefine();
				if (strcmp(sym, "SEMICN") != 0) error();
				getsym(); // 后来添加，目的是得到常量定义;后的下一个字符
			}
		}
	}
}

// ＜常量定义＞   :: = int＜标识符＞＝＜整数＞{ ,＜标识符＞＝＜整数＞ }
// |char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
void constantDefine() {
	function.var_number++;
	char idenfr[100];
	char type[100];
	if (strcmp(sym, "INTTK") == 0) {
		getsym(); // 读入标识符
		strcpy(idenfr, word);
		getsym(); // 读入等于号
		getsym();
		integer();
		printf("const int %s = %s ;\n", idenfr, intcon);
		fprintf(output, "const int %s = %s ;\n", idenfr, intcon);
		strcpy(type, "int");
		addpara(type, idenfr);
		while (strcmp(sym, "COMMA") == 0) {
			getsym();
			strcpy(idenfr, word);
			getsym();
			getsym(); // 得到整数第一项
			integer();
			printf("const int %s = %s ;\n", idenfr, intcon);
			fprintf(output, "const int %s = %s ;\n", idenfr, intcon);
			strcpy(type, "int");
			addpara(type, idenfr);
		}
	}
	else if (strcmp(sym, "CHARTK") == 0) {
		getsym(); // 读入标识符
		strcpy(idenfr, word);
		getsym(); // 读入等于号
		getsym(); // 读入字符
		printf("const char %s = %d ;\n", idenfr, word[0]);
		fprintf(output, "const char %s = %d ;\n", idenfr, word[0]); // 强制类型转换
		strcpy(type, "char");
		addpara(type, idenfr);
		getsym(); // 看是否是逗号
		while (strcmp(sym, "COMMA") == 0) {
			getsym();
			strcpy(idenfr, word);
			getsym();
			getsym();
			printf("const char %s = %d ;\n", idenfr, word[0]);
			fprintf(output, "const char %s = %d ;\n", idenfr, word[0]);
			addpara(type, idenfr);
			getsym();
		}
	}
	else error();
}

void integer() {
	strcpy(intcon, "");
	if (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0) {
		strcpy(intcon, word);
		getsym();
	}
	uninteger();
}

// ＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
void variableDeclare() {
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		variableDefine();
		if (strcmp(sym, "SEMICN") != 0) error();
		getsym(); // 读入下一个字符
		while (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
			char symTemp[100];
			char wordTemp[100];
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
				getsym();
			} // 表示继续变量定义
			else {
				pos = temp;
				strcpy(sym, symTemp);
				strcpy(word, wordTemp);
				break; // 不再是变量说明部分，而是函数定义部分
			}
		}
	}
}

// ＜变量定义＞  :: = ＜类型标识符＞(＜标识符＞ | ＜标识符＞'['＜无符号整数＞']') { , (＜标识符＞ | ＜标识符＞'['＜无符号整数＞']') }
// ＜无符号整数＞表示数组元素的个数，其值需大于0
void variableDefine() {
	// 已经保证是int或者char
	function.var_number++;
	char idenfr[100];
	char type[100];
	strcpy(type, word);
	getsym(); // 读入标识符
	strcpy(idenfr, word);
	getsym(); // 不一定是，故此处不输出
	if (strcmp(sym, "LBRACK") == 0) {
		getsym(); // 得到整数第一个符号
		strcpy(intcon, "");
		uninteger(); // 读入无符号整数
		// 此时已经读入右括号
		getsym();
		printf("var %s %s %s ;\n", type, idenfr, intcon);
		fprintf(output, "var %s %s %s ;\n", type, idenfr, intcon);
	}
	else {
		printf("var %s %s ;\n", type, idenfr);
		fprintf(output, "var %s %s ;\n", type, idenfr);
	}
	addpara(type, idenfr); // 暂不关心是一维的还是二维的，这部分由中间代码部分处理
	while (strcmp(sym, "COMMA") == 0) {
		getsym();
		strcpy(idenfr, word);
		getsym();
		if (strcmp(sym, "LBRACK") == 0) {
			getsym();
			strcpy(intcon, "");
			uninteger(); // 得到无符号整数和下一个大括号
			getsym(); // 得到逗号或下一个字符
			printf("var %s %s %s ;\n", type, idenfr, intcon);
			fprintf(output, "var %s %s %s ;\n", type, idenfr, intcon);
		}
		else {
			printf("var %s %s ;\n", type, idenfr);
			fprintf(output, "var %s %s ;\n", type, idenfr);
		}
		addpara(type, idenfr);
	}
}

// ＜有返回值函数定义＞  :: = ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
void refunction() {
	function.para_number = 0;
	function.var_number = 0;
	function.hascall = 0;
	// 声明头部必然已存在
	// 因为声明头部只有refunction有，所以可以再里面直接输出
	headState();
	begin();
	getsym(); // 得到参数表第一个字符
	parameterList();
	// printf("函数名是：%s,有%d个参数\n", function.name, function.para_number);
	// 此时已经读完右括号
	getsym(); // 得到左大括号
	getsym(); // 得到复合语句第一个词
	compoundStatement();
	func.push_back(function);
	getsym(); // 读取下一个
	end();
	// 已经读到右大括号
}

// ＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
void headState() {
	char type[100];
	char idenfr[100];
	strcpy(type, word);
	getsym(); // 得到有返回值函数的名字
	strcpy(idenfr, word);
	strcpy(refuncName[relen], word);
	printf("function %s %s ;\n", type, word);
	fprintf(output, "function %s %s ;\n", type, idenfr);
	if (strcmp(type, "int") == 0)
		function.type = 2;
	else
		function.type = 1;
	strcpy(function.name, idenfr);
	getsym();
	relen++;
}

// ＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
void vofunction() {
	function.para_number = 0;
	function.var_number = 0;
	function.hascall = 0;
	char idenfr[100];
	getsym();
	strcpy(vofuncName[volen], word);
	strcpy(idenfr, word);
	strcpy(function.name, idenfr);
	function.type = 0;
	printf("function void %s ;\n", idenfr);
	fprintf(output, "function void %s ;\n", idenfr);
	begin();
	volen++; // 储存名字
	getsym(); // (
	getsym();
	parameterList();
	// printf("函数名是：%s,有%d个参数\n", function.name, function.para_number);
	getsym(); // 得到{
	getsym();
	compoundStatement();
	func.push_back(function);
	getsym(); // 读取本段结束的下一个字符
	end();
}

// ＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
void parameterList() {
	//printf("函数名是：%s,有%d个参数\n", function.name, function.para_number);
	glevel++;
	char type[100];
	char idenfr[100];
	if (strcmp(sym, "INTTK") == 0 || strcmp(sym, "CHARTK") == 0) {
		function.para_number++;
		strcpy(type, word);
		getsym(); // 得到标识符
		strcpy(idenfr, word);
		getsym();
		printf("para %s %s ;\n", type, idenfr);
		fprintf(output, "para %s %s ;\n", type, idenfr);
		addpara(type, idenfr);
		while (strcmp(sym, "COMMA") == 0) {
			function.para_number++;
			getsym(); // 得到类型标识符
			strcpy(type, word);
			getsym(); // 得到标识符
			strcpy(idenfr, word);
			getsym();
			printf("para %s %s ;\n", type, idenfr);
			fprintf(output, "para %s %s ;\n", type, idenfr);
			addpara(type, idenfr);
		}
	}
	else {
		// 否则直接为空,无需进行任何处理
	}
	//printf("函数名是：%s,有%d个参数\n", function.name, function.para_number);
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
	glevel--;
	while (para.size() > 0 && para[para.size() - 1].lev > glevel) // 先后顺序有一定问题
		para.pop_back();
}

// ＜语句列＞   ::= ｛＜语句＞｝
void statementColumn() {
	while (isStatement()) {
		statement();
	}
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
			getsym();
		}
		else if (isvofuncCall()) {
			vofunctionCall();
			getsym();
		}
		else {
			assignStatement();
			getsym();
		}
	}
	else if (strcmp(sym, "SCANFTK") == 0) {
		scanStatement();
		getsym();
	}
	else if (strcmp(sym, "PRINTFTK") == 0) {
		printStatement(); // 读完;
		getsym();
	}
	else if (strcmp(sym, "SEMICN") == 0) {
		// 空语句
		getsym();
	}
	else if (strcmp(sym, "RETURNTK") == 0) {
		returnStatement();
		getsym();
	}
}

// ＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
// ＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
void ifStatement() {
	int temp = if_cnt;
	if_cnt++;
	getsym(); // 读取左括号
	getsym();
	condition(); // 内部直接输出condition %d %d <= %d形式的内容
	// 至此已经读完右括号
	condition_print(0);
	printf("if_label%d_end ;\n", temp);
	fprintf(output, "if_label%d_end ;\n", temp);
	getsym();
	statement();
	printf("goto if_total_end%d ;\n", temp);
	fprintf(output, "goto if_total_end%d ;\n", temp);
	// 不满足条件直接到else模块
	printf("if_label%d_end: ;\n", temp);
	fprintf(output, "if_label%d_end: ;\n", temp);
	if (strcmp(sym, "ELSETK") == 0) {
		getsym(); // 获得下一个语句的第一个词
		statement();
	}
	printf("if_total_end%d: ;\n", temp);
	fprintf(output, "if_total_end%d: ;\n", temp);
}

// ＜条件＞    :: = ＜表达式＞＜关系运算符＞＜表达式＞ //整型表达式之间才能进行关系运算
//｜＜表达式＞    //表达式为整型，其值为0条件为假，值不为0时条件为真      
void condition() {
	int temp1, temp2;
	char oper[100];
	expression();
	condition_temp1 = temp_num;
	if (isreOperator()) {
		strcpy(oper, word);
		getsym();
		expression();
		condition_temp2 = temp_num;
		assign_conditiontype(oper);
		/*
		temp_num++;
		printf("condition t%d t%d %s t%d ;\n", temp_num, temp1, oper, temp2);
		fprintf(output, "condition t%d t%d %s t%d ;\n", temp_num, temp1, oper, temp2);
		*/
	}
	else {
		/*
		printf("condition t%d t%d != 0 ;\n", temp_num, temp1);
		fprintf(output, "condition t%d t%d != 0 ;\n", temp_num, temp1);
		*/
		condition_type = 6;
		condition_temp2 = 0;
	}
}

// ＜表达式＞    :: = ［＋｜－］＜项＞{ ＜加法运算符＞＜项＞ }   //[+|-]只作用于第一个<项>
int expression() { // 第一个字符已经读入，结束时输出其本身终止点的下一个字符
	int expretype = 0;
	char oper[100];
	int mult = 0;
	int temp1, temp2;
	char right[100];
	char righttype[100];
	if (strcmp(sym, "MINU") == 0)
		mult = -1;
	if (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0)
		getsym();
	expretype += term();
	temp1 = temp_num;
	if (mult == -1) {
		printf("MINU t%d 0 t%d ;\n", temp_num + 1, temp_num);
		fprintf(output, "MINU t%d 0 t%d ;\n", temp_num + 1, temp_num);
		temp1 = temp_num + 1; // 此处进行修正
		temp_num++;
	}
	while (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0) {
		// 此处似乎也少了个getsym来得到第一个字符
		expretype += 2;
		strcpy(oper, sym);
		getsym();
		term();
		temp2 = temp_num;
		temp_num++;
		printf("%s t%d t%d t%d ;\n", oper, temp_num, temp1, temp2);
		fprintf(output, "%s t%d t%d t%d ;\n", oper, temp_num, temp1, temp2);
		temp1 = temp_num;
	}
	/*printf("表达式:");
	if (expretype > 1)
		printf("int\n");
	else
		printf("char\n");
	*/
	return expretype;
}

// ＜项＞     :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
int term() {
	int termtype = 0;
	char oper[100];
	int temp1, temp2;
	termtype += factor();
	temp1 = temp_num;
	while (strcmp(sym, "MULT") == 0 || strcmp(sym, "DIV") == 0) {
		termtype += 2;
		strcpy(oper, sym);
		getsym();
		factor();
		temp2 = temp_num;
		temp_num++; // 后续进行的添加
		printf("%s t%d t%d t%d ;\n", oper, temp_num, temp1, temp2);
		fprintf(output, "%s t%d t%d t%d ;\n", oper, temp_num, temp1, temp2);
		temp1 = temp_num;
	}
	return termtype;
}

// ＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞         
int factor() {
	int factortype = 0;
	char type[100];
	if (isrefuncCall()) { // 有返回值函数调用
		for (int i = func.size() - 1; i >= 0; i--) {
			if (strcmp(func[i].name, word) == 0) {
				factortype += func[i].type;
				break;
			}
		}
		refunctionCall(); // 外面不用添加任何处理
		if (factortype > 1)
			strcpy(type, "int");
		else
			strcpy(type, "char");
		temp_num++;
		printf("%s t%d = RET ;\n", type, temp_num);
		fprintf(output, "%s t%d = RET ;\n", type, temp_num);
	}
	else if (strcmp(sym, "IDENFR") == 0) {
		char idenfr[100];
		strcpy(idenfr, word);
		for (int i = para.size() - 1; i >= 0; i--) {
			if (strcmp(para[i].name, idenfr) == 0) {
				factortype += para[i].type;
				break;
			}
		}
		if (factortype > 1)
			strcpy(type, "int");
		else
			strcpy(type, "char");
		getsym();
		if (strcmp(sym, "LBRACK") == 0) { // 正常的表达式,直接顺着往后推a[7]
			// 此时需要先输出上一项标识符的内容
			getsym();
			expression();
			temp_num++;
			printf("%s t%d = %s t%d ;\n", type, temp_num, idenfr, (temp_num - 1));
			fprintf(output, "%s t%d = %s t%d ;\n", type, temp_num, idenfr, (temp_num - 1));
			getsym();
		}
		else {
			// 表示是单独的标识符
			temp_num++;
			printf("%s t%d = %s ;\n", type, temp_num, idenfr);
			fprintf(output, "%s t%d = %s ;\n", type, temp_num, idenfr);
		}
	}
	else if (strcmp(sym, "LPARENT") == 0) {
		getsym();
		expression();
		temp_num++;
		printf("int t%d = t%d ;\n", temp_num, (temp_num - 1));
		fprintf(output, "int t%d = t%d ;\n", temp_num, (temp_num - 1));
		factortype += 2; // 带有括号的内容必然已经直接完成转换
		// 此时没有必要对当前储存最后值的t进行调整
		getsym(); // 读取括号后面一个字符
	}
	else if (strcmp(sym, "PLUS") == 0 || strcmp(sym, "MINU") == 0 || strcmp(sym, "INTCON") == 0) {
		factortype += 2;
		integer(); // 自动读取自身结束的后一个字符
		temp_num++;
		printf("int t%d = %s ;\n", temp_num, intcon);
		fprintf(output, "int t%d = %s ;\n", temp_num, intcon);
	}
	else if (strcmp(sym, "CHARCON") == 0) {
		factortype += 1;
		temp_num++;
		printf("char t%d = %d ;\n", temp_num, word[0]);
		fprintf(output, "char t%d = %d ;\n", temp_num, word[0]);
		getsym();
	}
	return factortype;
	//else return; // 否则不合规范，已经是下一个字符直接return
}

// ＜有返回值函数调用语句＞ :: = ＜标识符＞'('＜值参数表＞')'
void refunctionCall() {
	function.hascall++;
	char name[100];
	strcpy(name, word);
	getsym(); // 读入(
	getsym(); // 读入值参数表第一个字符
	valueList();
	printf("call %s ;\n", name);
	fprintf(output, "call %s ;\n", name);
	getsym(); // 读取下一个字符
}

// ＜无返回值函数调用语句＞ :: = ＜标识符＞'('＜值参数表＞')'
void vofunctionCall() {
	function.hascall++;
	char name[100];
	strcpy(name, word);
	getsym(); // 读入(
	getsym(); // 读入值参数表第一个字符
	valueList();
	printf("call %s ;\n", name);
	fprintf(output, "call %s ;\n", name);
	getsym(); // 读取下一个字符
}

// ＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
void assignStatement() {
	// 首先是表达式
	char idenfr[100];
	char right[100];
	char righttype[100];
	char symTemp[100];
	char wordTemp[100];
	int  temp;
	int temp1, temp2;
	strcpy(idenfr, word);
	getsym();
	if (strcmp(sym, "ASSIGN") == 0) {
		temp = pos;
		strcpy(symTemp, sym);
		strcpy(wordTemp, word);
		tryget();
		strcpy(right, word);
		strcpy(righttype, sym);
		tryget();
		if (strcmp(word, ";") == 0) {
			if (strcmp(righttype, "CHARCON") == 0) {
				printf("%s = %d ;\n", idenfr, right[0]);
				fprintf(output, "%s = %d ;\n", idenfr, right[0]);
			}
			else {
				printf("%s = %s ;\n", idenfr, right);
				fprintf(output, "%s = %s ;\n", idenfr, right);
			}
		}
		else {
			pos = temp;
			strcpy(sym, symTemp);
			strcpy(word, wordTemp);
			getsym();
			expression();
			printf("%s = t%d ;\n", idenfr, temp_num);
			fprintf(output, "%s = t%d ;\n", idenfr, temp_num);
		}
	}
	else if (strcmp(sym, "LBRACK") == 0) {
		// 对于数组元素想要赋值的话必须先用个t提取出来再弄
		getsym();
		expression();
		temp1 = temp_num;
		getsym(); // 得到=
		temp = pos;
		strcpy(symTemp, sym);
		strcpy(wordTemp, word);
		tryget(); // 得到表达式第一个数
		strcpy(right, word);
		strcpy(righttype, sym);
		tryget(); // 看是否能得到;
		if (strcmp(word, ";") == 0) {
			if (strcmp(righttype, "CHARCON") == 0) {
				printf("%s t%d = %d ;\n", idenfr, temp1, right[0]); // 此处进行特殊修改
				fprintf(output, "%s t%d = %d ;\n", idenfr, temp1, right[0]);
			}
			else {
				printf("%s t%d = %s ;\n", idenfr, temp1, right); // 此处进行特殊修改
				fprintf(output, "%s t%d = %s ;\n", idenfr, temp1, right);
			}
		}
		else {
			pos = temp;
			strcpy(sym, symTemp);
			strcpy(word, wordTemp);
			getsym(); // 得到表达式第一个符号
			expression();
			temp2 = temp_num;
			printf("%s t%d = t%d ;\n", idenfr, temp1, temp2); // 此处进行特殊修改
			fprintf(output, "%s t%d = t%d ;\n", idenfr, temp1, temp2);
		}
	}
}

// ＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞
void valueList() {
	vector<int> para_temp1;
	if (strcmp(sym, "RPARENT") == 0) {
		// 表示为空
	}
	else {
		expression(); // 默认必然是值参数表
		para_temp1.push_back(temp_num);
		while (strcmp(sym, "COMMA") == 0) {
			getsym();
			expression();
			para_temp1.push_back(temp_num);
		}
		for (int i = 0; i < para_temp1.size(); i++) {
			printf("push t%d ;\n", para_temp1[i]);
			fprintf(output, "push t%d ;\n", para_temp1[i]);
		}
	}
}

// ＜循环语句＞   :: = while '('＜条件＞')'＜语句＞ 
// | do＜语句＞while '('＜条件＞')' 
// | for'('＜标识符＞＝＜表达式＞; ＜条件＞; ＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
void loopStatement() { // 三个合一起写，不要分开写
	int temp; // 储存中间的结果
	if (strcmp(sym, "WHILETK") == 0) { // while开始
		temp = while_cnt;
		while_cnt++;
		getsym();
		getsym();
		printf("while_label%d_begin: ;\n", temp);
		fprintf(output, "while_label%d_begin: ;\n", temp);
		condition();
		condition_print(0);
		printf("while_label%d_end ;\n", temp);
		fprintf(output, "while_label%d_end ;\n", temp);
		getsym(); // 得到语句第一个字符
		statement(); // 读入语句
		printf("goto while_label%d_begin ;\n", temp);
		fprintf(output, "goto while_label%d_begin ;\n", temp);
		printf("while_label%d_end: ;\n", temp);
		fprintf(output, "while_label%d_end: ;\n", temp);
	}
	else if (strcmp(sym, "DOTK") == 0) { // do开始
		temp = do_cnt;
		do_cnt++;
		printf("do_label%d_begin: ;\n", temp);
		fprintf(output, "do_label%d_begin: ;\n", temp);
		getsym();
		statement();
		getsym(); // while已经读入，读入左括号
		getsym();
		condition();
		condition_print(1);
		printf("do_label%d_begin ;\n", temp);
		fprintf(output, "do_label%d_begin ;\n", temp);
		getsym();
	}
	else if (strcmp(sym, "FORTK") == 0) { // for开始
		temp = for_cnt;
		for_cnt++;
		char idenfr[100];
		char idenfr2[100];
		char oper[100];
		char intcon1[100]; // 不能按顺序输出，因为中间可能会用到i
		getsym(); // (
		getsym(); // 标识符
		strcpy(idenfr, word);
		getsym(); // =
		getsym();
		expression();
		// 输出起始条件
		printf("%s = t%d ;\n", idenfr, temp_num);
		fprintf(output, "%s = t%d ;\n", idenfr, temp_num);
		getsym();
		// 输出循环标签
		printf("for_label%d_begin: ;\n", temp);
		fprintf(output, "for_label%d_begin: ;\n", temp);
		condition();
		// 不符合条件直接结束
		condition_print(0);
		printf("for_label%d_end ;\n", temp);
		fprintf(output, "for_label%d_end ;\n", temp);
		getsym(); // 上一个已读入;,现在读标识符
		strcpy(idenfr, word);
		getsym(); //=
		getsym(); // 读取表达式第一个
		expression();
		int t = temp_num;
		/*
		getsym(); // 标识符
		strcpy(idenfr2, word);
		getsym(); // +-
		strcpy(oper, sym);
		getsym();
		step(); // 无符号 整数
		strcpy(intcon1, intcon);
		*/
		getsym(); // 读语句第一个词
		statement(); // 已自动读取下一个
		// i = i + 1,此处需要人工对这种类型的语句进行解析
		// 直接返回开头
		printf("%s = t%d ;\n", idenfr, t);
		fprintf(output, "%s = t%d ;\n", idenfr, t); // 为了避免出现问题选择将内容进行反转
		printf("goto for_label%d_begin ;\n", temp);
		fprintf(output, "goto for_label%d_begin ;\n", temp);
		// 结束标签
		printf("for_label%d_end: ;\n", temp);
		fprintf(output, "for_label%d_end: ;\n", temp);
	}
}

// ＜读语句＞    ::=  scanf '('＜标识符＞{,＜标识符＞}')'
void scanStatement() {
	char idenfr[100];
	getsym(); // (
	getsym();
	strcpy(idenfr, word);
	printf("scanf %s ;\n", idenfr);
	fprintf(output, "scanf %s ;\n", idenfr);
	getsym();
	while (strcmp(sym, "COMMA") == 0) {
		getsym();
		strcpy(idenfr, word);
		printf("scanf %s ;\n", idenfr);
		fprintf(output, "scanf %s ;\n", idenfr);
		getsym();
	}
	getsym(); // 读)后面的字符
}

// ＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
void printStatement() {
	getsym();
	getsym();
	int type = 0;
	char type1[100];
	if (strcmp(sym, "STRCON") == 0) {
		printf("printf string%d \"wordx\" ;\n", string_cnt);
		fprintf(output, "printf string%d \"wordx\" ;\n", string_cnt);
		string str = word;
		print_string.push_back(str);
		string_cnt++;
		cString();
		if (strcmp(sym, "COMMA") == 0) {
			getsym();
			type = expression();
			if (type == 1)
				strcpy(type1, "char");
			else
				strcpy(type1, "int");
			printf("printf expression %s t%d ;\n", type1, temp_num);
			fprintf(output, "printf expression %s t%d ;\n", type1, temp_num);
		}
	}
	else {
		type = expression();
		if (type == 1)
			strcpy(type1, "char");
		else
			strcpy(type1, "int");
		printf("printf expression %s t%d ;\n", type1, temp_num);
		fprintf(output, "printf expression %s t%d ;\n", type1, temp_num);
	}
	printf("printf string0 x ;\n");
	fprintf(output, "printf string0 x ;\n");
	getsym();
}

// ＜返回语句＞   :: = return['('＜表达式＞')']
void returnStatement() {
	getsym();
	if (strcmp(sym, "LPARENT") == 0) {
		getsym();
		expression();
		getsym(); // 得到返回语句的下一个
		printf("ret t%d ;\n", temp_num);
		fprintf(output, "ret t%d ;\n", temp_num);
	}
	else {
		printf("ret ;\n");
		fprintf(output, "ret ;\n");
	} // 即使为空也需要返回
	// 否则为没有返回内容的return语句，无需表述
}

// ＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
void mainFunction() { // 修改后假设直接从main进来
	glevel++;
	printf("\n\nfunction void main ;\n");
	fprintf(output, "function void main ;\n");
	begin();
	getsym();
	getsym();
	getsym(); // 读完()
	getsym();
	getsym(); // 读入复合语句第一个字符
	compoundStatement();
	//end();
	// 此时已经是最后一个，无需再读入，需要进行特殊处理
	//print();
}

// ＜步长＞::= ＜无符号整数＞  
void step() {
	// 已经读入整数
	strcpy(intcon, "");
	uninteger(); // 这里应该有个整数
}

// ＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"
void cString() {
	getsym(); // 读入下一项的第一个字符
}

void uninteger() {
	strcat(intcon, word);
	getsym();
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

void begin() {
	printf("begin ;\n");
	fprintf(output, "begin ;\n");
}

void end() {
	printf("end ;\n");
	fprintf(output, "end ;\n");
}

void addpara(char type[100], char idenfr[100]) {
	strcpy(entity.name, idenfr);
	if (strcmp(type, "int") == 0)
		entity.type = 2;
	else
		entity.type = 1;
	entity.lev = glevel;
	para.push_back(entity); // 添加到vector序列里
	return;
}

void assign_conditiontype(char oper[100]) {
	// 1对应==,2对应<，3对应>,4对应<=,5对应>=,6对应!=
	if (strcmp(oper, "==") == 0) {
		condition_type = 1;
	}
	if (strcmp(oper, "<") == 0) {
		condition_type = 2;
	}
	if (strcmp(oper, ">") == 0) {
		condition_type = 3;
	}
	if (strcmp(oper, "<=") == 0) {
		condition_type = 4;
	}
	if (strcmp(oper, ">=") == 0) {
		condition_type = 5;
	}
	if (strcmp(oper, "!=") == 0) {
		condition_type = 6;
	}
}

void condition_print(int type) {
	int relation_num = condition_type;
	// 1对应 == , 2对应<，3对应>, 4对应 <= , 5对应 >= , 6对应 !=
	char relation[8][100] = { "","beq","bltz","bgtz","blez","bgez","bne" };
	if (type != 1) {
		relation_num = 7 - relation_num;
	}
	if (relation_num == 1 || relation_num == 6) {
		printf("condition %s t%d t%d ", relation[relation_num], condition_temp1, condition_temp2);
		fprintf(output, "condition %s t%d t%d ", relation[relation_num], condition_temp1, condition_temp2);
	} // 如果是t0表示0
	else {
		temp_num++;
		printf("MINU t%d t%d t%d ;\n", temp_num, condition_temp1, condition_temp2);
		fprintf(output, "MINU t%d t%d t%d ;\n", temp_num, condition_temp1, condition_temp2);
		printf("condition %s t%d ", relation[relation_num], temp_num);
		fprintf(output, "condition %s t%d ", relation[relation_num], temp_num);
	}
}

void condition_print1(int ct1, int ct2, int typetemp) {
	int relation_num = typetemp;
	// 1对应 == , 2对应<，3对应>, 4对应 <= , 5对应 >= , 6对应 !=
	char relation[8][100] = { "","beq","bltz","bgtz","blez","bgez","bne" };
	if (relation_num == 1 || relation_num == 6) {
		printf("condition %s t%d t%d ", relation[relation_num], ct1, ct2);
		fprintf(output, "condition %s t%d t%d ", relation[relation_num], ct1, ct2);
	} // 如果是t0表示0
	else {
		temp_num++;
		printf("MINU t%d t%d t%d ;\n", temp_num, ct1, ct2);
		fprintf(output, "MINU t%d t%d t%d ;\n", temp_num, ct1, ct2);
		printf("condition %s t%d ", relation[relation_num], temp_num);
		fprintf(output, "condition %s t%d ", relation[relation_num], temp_num);
	}
}
