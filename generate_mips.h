#ifndef _GENERATE_MIPS_H
#define _GENERATE_MIPS_H
#include <string>
#include <iostream>
using namespace std;
typedef struct {
	string name;
	string type;
	int level; // 统计在第几层
	int offset;  // 统计相对于gp偏移了多少
	int dim; // 如果是单纯的变量则为0，为数组的话则对应1
	int isreg = 0; // 表示是否储存在寄存器中，如果在的话就记为1，否则为0
	string reg; // 表示储存其的寄存器
}parameter;

typedef struct {
	string reg;
	string name; // 记录储存的变量名
	int isuse = 0; // 记录当前的使用情况
}treg; // 记录储存临时变量的寄存器的使用情况

void generate_mips();
void initial();
void mips_end();
void const_add();
void var_add(int word_num);
int tonumber(string x);
void mips_print1();
void mips_print2();
void mips_scanf();
void mips_return();
void add_intt();
void add_chart();
void cal_t();
void assign_t();
string find_t(string paraname, string reg); // 用来查找对应的临时变量
void mips_condition(int num);
void para_add();
void mips_push();
void mips_call();
void mips_function();
void function_end();
void mips_goto();
void mips_return1();
void assign1_t();
void add_chart1();
void add_intt1();
void find_array(string arrayname, string off, string reg);
void word_print(int line_word);
int isnumber(string x);
void reginitial();
void reg_save(int need);
void reg_recover(int need);
void treg_initial();
string set_treg(string name, string reg);
void print_funcinfo();
int getInfo(string name);
#endif /* _GENERATE_MIPS_H */
