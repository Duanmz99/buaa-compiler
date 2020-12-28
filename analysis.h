#ifndef _ANALYSIS_H // 避免重复引用
#define _ANALYSIS_H

// analysis中定义的函数
int isreserver(char str[]);
int isletter(char ch);
void catToken(char ch);
int issingle(char ch);
int isdouble(char ch);
int trygetsym(); // 不进行输出
void tryget();
void getsym(); // 进行输出(先输出后读入)
void printInfo();

#endif /* _ANALYSIS_H */
