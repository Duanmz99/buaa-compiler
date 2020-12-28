// 对应于第二次作业词法分析
// 本部分函数可以提取文件中的所有单词并识别其特性
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// 在grammer.c里面定义，此处为重新声明
extern FILE* input;
extern FILE* output;
extern char sym[100];
extern char word[100];
extern int time1;

extern char symall[100000][100]; // 改变思路储存全部
extern char wordall[100000][100];
extern int wordNumber;
extern int pos;
extern int line;

// 本文件特有变量
char Reserver[30][100] = { "const","int","char","void","main","if","else","do","while","for","scanf","printf","return" };
char Reserver_class[30][100] = { "CONSTTK","INTTK","CHARTK","VOIDTK","MAINTK","IFTK","ELSETK","DOTK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK" };
char c;

void printInfo() {
	printf("%s %s\n", sym, word);
	fprintf(output, "%s %s\n", sym, word);
}

int isreserver(char str[]) {
	for (int i = 0; i < 20; i++) {
		if (strcmp(str, Reserver[i]) == 0) {
			strcpy(sym, Reserver_class[i]);
			return 1;
		}
	}
	return 0;
}

int isletter(char ch) {
	if (isalpha(ch) || ch == '_') {
		return 1;
	}
	return 0;
}

void catToken(char ch) {
	int len = strlen(word);
	word[len] = ch;
	word[len + 1] = '\0';
}

int issingle(char ch) {
	if (ch == '+') {
		strcpy(sym, "PLUS");
	}
	else if (ch == '-') {
		strcpy(sym, "MINU");
	}
	else if (ch == '*') {
		strcpy(sym, "MULT");
	}
	else if (ch == '/') {
		strcpy(sym, "DIV");
	}
	else if (ch == ';') {
		strcpy(sym, "SEMICN");
	}
	else if (ch == ',') {
		strcpy(sym, "COMMA");
	}
	else if (ch == '(') {
		strcpy(sym, "LPARENT");
	}
	else if (ch == ')') {
		strcpy(sym, "RPARENT");
	}
	else if (ch == '[') {
		strcpy(sym, "LBRACK");
	}
	else if (ch == ']') {
		strcpy(sym, "RBRACK");
	}
	else if (ch == '{') {
		strcpy(sym, "LBRACE");
	}
	else if (ch == '}') {
		strcpy(sym, "RBRACE");
	}
	else {
		// 不是single的模型
		return 0;
	}
	catToken(ch);
	return 1;
}

int isdouble(char ch) {
	if (ch == '<') {
		catToken(ch);
		c = fgetc(input);
		if (c == '=') {
			catToken(c);
			strcpy(sym, "LEQ");
			c = fgetc(input);
		}
		else {
			strcpy(sym, "LSS");
		}
	}
	else if (ch == '>') {
		catToken(ch);
		c = fgetc(input);
		if (c == '=') {
			catToken(c);
			strcpy(sym, "GEQ");
			c = fgetc(input);
		}
		else {
			strcpy(sym, "GRE");
		}
	}
	else if (ch == '=') {
		catToken(ch);
		c = fgetc(input);
		if (c == '=') {
			catToken(c);
			strcpy(sym, "EQL");
			c = fgetc(input);
		}
		else {
			strcpy(sym, "ASSIGN");
		}
	}
	else if (ch == '!') {
		catToken(ch);
		c = fgetc(input);
		if (c == '=') {
			catToken(c);
			strcpy(sym, "NEQ");
			c = fgetc(input);
		}
	}
	else {
		return 0;
	}
	return 1;

}

int trygetsym() {
	// clear token
	strcpy(word, "");
	strcpy(sym, "");
	if (!time1) {
		time1++;
		c = fgetc(input);
	}
	while (isspace(c)) {
		if (c == '\n') {
			strcpy(wordall[wordNumber], "\n");
			wordNumber++;
		}
		c = fgetc(input); // 读取无用字符 
	}
	if (isletter(c)) {
		while (isletter(c) || isdigit(c)) {
			catToken(c);
			c = fgetc(input);
		}
		// retract机制已经通过其他设置避免
		int result = isreserver(word);
		if (!result)
			strcpy(sym, "IDENFR");
	}
	else if (isdigit(c)) {
		while (isdigit(c)) {
			catToken(c);
			c = fgetc(input);
		}
		strcpy(sym, "INTCON");
	}
	else if (issingle(c)) {
		c = fgetc(input);
	}
	else if (isdouble(c)) {
		// 此处为了统一处理设为空
		// 一开始因为这里直接return导致getsym无法输出
	}
	else if (c == '\"') {
		c = fgetc(input);
		while (c != '\"') {
			catToken(c);
			c = fgetc(input);
		}
		strcpy(sym, "STRCON");
		c = fgetc(input);
	} // 是字符串 
	else if (c == '\'') {
		c = fgetc(input);
		catToken(c);
		c = fgetc(input);
		strcpy(sym, "CHARCON");
		c = fgetc(input);
	}
	else {
		//此时进入了eof阶段 
		return 1;
	}
	return 0;
}

void tryget() {
	strcpy(word, wordall[pos]);
	while (strcmp(word, "\n") == 0) {
		line++;
		//printf("1\n");
		pos++;
		strcpy(word, wordall[pos]);
	}
	strcpy(sym, symall[pos]);
	//printf("%s line:%d\n", word,line);
	pos++;
	//printf("word:%s sym:%s\n", word, sym);
}

void getsym() {
	tryget();
	return;
}
