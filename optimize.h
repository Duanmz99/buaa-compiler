#ifndef _OPTIMIZE_H // 避免重复引用
#define _OPTIMIZE_H
#include <iostream>
#include <string>
using namespace std;
void optimize();
void show();
void add_same();
void change_same();
void delete_same();
int tfind(string x);
void output_finalmid(); // 输出优化后的中间代码
#endif /* _OPTIMIZE_H */
