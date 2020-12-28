#include <iostream>
#include <stdio.h>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include "generate_mid.h"
#include "generate_mips.h"
using namespace std;

vector<string> mid_word; // 统计中间代码的所有单词
vector<parameter> mips_para; // 统计当前所有的单词
parameter para1; // 统计当前的单词信息

extern vector<gfunction> func; // 统计了所有函数对应的信息
extern vector<string> print_string; // 用于储存后续需要输出的字符串内容
//寄存器信息
vector<string> regname;
int regtime = -1; // 统计当前即将赋值的寄存器,先++再用，保证完美匹配
// t7-t9三个寄存器轮流倒
int reg_limit = 8; // 记录当前最多有几个寄存器赋给定义量
vector<treg> temp_reg;

int mid_lev = 1;
int gp = 0; // 储存全局变量栈的地址
int sp = 0; // 储存局部变量栈的地址
string word[6]; // 统计每一行所有词，便于后续判断
ifstream inFile;
ofstream outFile;

vector<string> func_para; // 储存对应函数的信息
int para_num = 0; // 与上面的func_para对应，如果两个一样大就直接结束
extern vector<gfunction> func;
// 保险起见不能用else，只能全程else if，避免出现未知语句

int push_num = 0; // 统计当前的参数数量,最大为3 每一个push x该数+1
int getpara_num = 0; // 统计当前提取的参数数量，每一个para int x该数加1
int save = 0; // 用来储存当前寄存器是否已经全部储存
string func_name;
void generate_mips() {
	inFile.open("17373217_段牧知_优化后中间代码.txt");
	outFile.open("mips.txt");
	print_funcinfo();
	initial();
	reginitial();
	treg_initial();
	for (int i = 0; i < mid_word.size(); i++) {
		if (mid_word[i] == "end" && i == mid_word.size() - 1)
			break; //判断终止条件
		int line_word = 0;
		para1.isreg = 0;
		while (mid_word[i] != ";") {
			word[line_word] = mid_word[i];
			i++;
			line_word++;
		}
		if (line_word == 1) {
			if (word[line_word - 1] == "begin") {
				// 同时搞定sp减小的问题
				mid_lev++;
				sp = -4;
			}
			else if (word[line_word - 1] == "end") {
				// 因为不能连续定义，故直接清除本目录对应的所有内容，并且sp也不需要其来处理
				function_end();
				cout << "\n";
				outFile << "\n"; // 方便debug
				mid_lev--;
				// 同时搞定sp增加的问题
				// 将表里面不符合规定的内容直接删除
			}
			else if (word[0] == "ret") {
				mips_return1();
			}
			else { // 对应标签类内容
				cout << word[0] << "\n";
				outFile << word[0] << "\n";
			}
		}
		else if (line_word == 2) {
			if (word[0] == "jal") {
				cout << "jal main\n\n";
				outFile << "jal main\n\n";
				mips_end(); //防止main函数存在return语句而处理不了
			}
			else if (word[0] == "ret") {
				mips_return();
				printf("ok\n");
			} // 表示函数返回值
			else if (word[0] == "call") {
				mips_call();
			} // 表示函数调用
			else if (word[0] == "push") {
				mips_push();
			} // 表示函数传参
			else if (word[0] == "scanf") {
				mips_scanf();
			}
			else if (word[0] == "goto") {
				mips_goto();
			}
		}
		else if (line_word == 3) {
			if (word[0] == "printf") {
				mips_print1();
			}
			else if (word[0] == "var") { // 单独变量
				var_add(3);
			} // 对应变量
			else if (word[1] == "=") {
				assign_t();
			}
			else if (word[0] == "para") {
				para_add();
			}
			else if (word[0] == "function") {
				// function int k
				mips_function();
			}
		}
		else if (line_word == 4) {
			if (word[0] == "printf") {
				mips_print2();
			}
			else if (word[0] == "var") { // 变量数组
				var_add(4);
			}
			else if (word[0] == "int") {
				add_intt();
			}
			else if (word[0] == "char") {
				add_chart();
			}
			else if (word[0] == "PLUS" || word[0] == "MINU" || word[0] == "MULT" || word[0] == "DIV") {
				cal_t();
			}
			else if (word[0] == "condition") {
				mips_condition(4);
			}
			else if (word[2] == "=") {
				assign1_t();
			}
		}
		else if (line_word == 5) {
			if (word[0] == "const") {
				const_add();
			} // 添加常量
			else if (word[0] == "condition") {
				mips_condition(5);
			}
			else if (word[0] == "int") {
				add_intt1();
			}
			else if (word[0] == "char") {
				add_chart1();
			}
		}
		/*
		for (int k = 0; k < line_word; k++) {
			cout << word[k] << " ";
		}
		cout << "\n";
		*/
		// 开始根据开头词判断
	}

	mips_end();
	inFile.close();
	outFile.close();
}

void initial() { // 处理开始时的全局变量
	// 先处理string字符串的问题
	for (int i = 0; i < print_string.size(); i++) {
		string y = "";
		for (int j = 0; j < print_string[i].size(); j++) {
			y = y + print_string[i][j];
			if (print_string[i][j] == '\\') {
				y = y + '\\';
			}
		}
		print_string[i] = y;
	}
	string word;
	while (inFile >> word) {
		mid_word.push_back(word);
	}
	cout << ".data\n";
	outFile << ".data\n";
	cout << "string0:.asciiz \"\\n\"\n";
	outFile << "string0:.asciiz \"\\n\"\n";
	for (int i = 0; i < print_string.size(); i++) {
		cout << "string" << (i + 1) << ":.asciiz " << "\"" << print_string[i] << "\"\n";
		outFile << "string" << (i + 1) << ":.asciiz " << "\"" << print_string[i] << "\"\n";
	}
	cout << ".text\n";
	outFile << ".text\n";
	return;
}

void mips_end() {
	cout << "li $v0,10\n";
	outFile << "li $v0,10\n";
	cout << "syscall\n";
	outFile << "syscall\n";
}

void const_add() {
	if (mid_lev == 1) {
		para1.level = 1;
		para1.type = word[1];
		para1.name = word[2];
		para1.offset = gp;
		if (regtime < reg_limit) { // 替换为regname.size()-1可能存在无符号有符号比较障碍问题
			regtime++;
			para1.isreg = 1;
			para1.reg = regname[regtime];
			cout << "li " << regname[regtime] << "," << word[4] << "\n";
			outFile << "li " << regname[regtime] << "," << word[4] << "\n";
		}
		else {
			para1.isreg = 0;
			cout << "li $t9," << word[4] << "\n";
			outFile << "li $t9," << word[4] << "\n";
			cout << "sw $t9," << to_string(gp) << "($gp)\n";
			outFile << "sw $t9," << to_string(gp) << "($gp)\n";
		}
		mips_para.push_back(para1);
		gp += 4;
	} // 全局变量
	else {
		// 此时需要储存在sp里面
		para1.level = mid_lev;
		para1.type = word[1];
		para1.name = word[2];
		para1.offset = sp;
		if (regtime < reg_limit) {
			regtime++;
			para1.isreg = 1;
			para1.reg = regname[regtime];
			cout << "li " << regname[regtime] << "," << word[4] << "\n";
			outFile << "li " << regname[regtime] << "," << word[4] << "\n";
		}
		else {
			para1.isreg = 0;
			cout << "li $t9," << word[4] << "\n";
			outFile << "li $t9," << word[4] << "\n";
			cout << "sw $t9," << to_string(sp) << "($sp)\n";
			outFile << "sw $t9," << to_string(sp) << "($sp)\n";
		}
		mips_para.push_back(para1);
		sp -= 4;
	} // 函数定义的局部变量
	//cout << "const add后regtime为：" << regtime;
}

void var_add(int word_num) {
	if (word_num == 3) { // 单独变量
		para1.level = mid_lev;
		para1.type = word[1];
		para1.name = word[2];
		para1.dim = 0;
		if (regtime < reg_limit) {
			regtime++;
			//cout << "regtime is:" << regtime << "var name:" << word[2] << endl;
			para1.isreg = 1;
			para1.reg = regname[regtime];
		}
		else
			para1.isreg = 0;
		if (mid_lev == 1) {
			para1.offset = gp;
			mips_para.push_back(para1); // 未被赋值时无需输出
			gp += 4;
		}
		else {
			para1.offset = sp;
			mips_para.push_back(para1); // 未被赋值时无需输出
			sp -= 4;
		}
		cout << "变量信息:" + para1.name + "                        " + "isreg:" + to_string(para1.isreg) + "               ";
		if (para1.isreg) {
			cout << para1.reg << endl;
		}
	}
	else if (word_num == 4) { // 变量数组,数组名也增添该功能
		para1.level = mid_lev;
		para1.type = word[1];
		para1.dim = 1;
		para1.name = word[2];
		if (regtime < reg_limit) {
			regtime++;
			//cout << "regtime is:" << regtime << "var name:" << word[2] << endl;
			para1.isreg = 1;
			para1.reg = regname[regtime];
		}
		else
			para1.isreg = 0;
		if (mid_lev == 1) {
			para1.offset = gp;
			mips_para.push_back(para1); // 未被赋值时无需输出
		}
		else {
			para1.offset = sp;
			mips_para.push_back(para1); // 未被赋值时无需输出
		}
		if (para1.isreg == 1) {
			if (mid_lev == 1) {
				cout << "addi " + para1.reg + ",$gp," + to_string(para1.offset) + "\n";
				outFile << "addi " + para1.reg + ",$gp," + to_string(para1.offset) + "\n";
			}
			else {
				cout << "addi " + para1.reg + ",$sp," + to_string(para1.offset) + "\n";
				outFile << "addi " + para1.reg + ",$sp," + to_string(para1.offset) + "\n";
			}
		}
		for (int i = 0; i < tonumber(word[3]); i++) {
			if (mid_lev == 1)
				gp += 4;
			else
				sp -= 4;
		}
	}
} // 数组的处理存在问题

int tonumber(string x) {
	int num = 0;
	for (int i = 0; i < x.length(); i++) {
		num = num * 10 + (x[i] - '0');
	}
	return num;
}

void mips_print1() { // 输出字符串
	cout << "la $a0," << word[1] << "\n";
	outFile << "la $a0," << word[1] << "\n";
	cout << "li $v0,4\n";
	outFile << "li $v0,4\n";
	cout << "syscall\n";
	outFile << "syscall\n";
} // 输出字符串

void mips_print2() { // printf expression int 9/t
	if (isnumber(word[3])) {
		cout << "li $a0," << word[3] << "\n";
		outFile << "li $a0," << word[3] << "\n";
	} // 如果是数字则直接准备输出
	else {
		int judge = 0;
		for (int i = reg_limit + 1; i < temp_reg.size(); i++) {
			if (temp_reg[i].isuse == 1) {
				if (temp_reg[i].name == word[3]) {
					temp_reg[i].isuse = 0;
					cout << "addu $a0," << temp_reg[i].reg << ",$zero\n";
					outFile << "addu $a0," << temp_reg[i].reg << ",$zero\n";
					judge = 1;
					break;
				}
			}
		}
		if (!judge) {
			for (int i = mips_para.size() - 1; i >= 0; i--) {
				if (mips_para[i].level != 1 && mips_para[i].level != mid_lev)
					continue;
				else if (mips_para[i].name == word[3]) {
					if (mips_para[i].isreg == 1) {
						cout << "addu $a0," << mips_para[i].reg << ",$zero\n";
						outFile << "addu $a0," << mips_para[i].reg << ",$zero\n";
					}
					else {
						if (mips_para[i].level == 1) {
							cout << "lw $a0," << mips_para[i].offset << "($gp)\n";
							outFile << "lw $a0," << mips_para[i].offset << "($gp)\n";
						}
						else {
							cout << "lw $a0," << mips_para[i].offset << "($sp)\n";
							outFile << "lw $a0," << mips_para[i].offset << "($sp)\n";
						}
					}
					break;
				}
			}
		}
	}
	if (word[2] == "int") {
		cout << "li $v0,1\n";
		outFile << "li $v0,1\n";
	} // 输出int型表达式
	else {
		cout << "li $v0,11\n";
		outFile << "li $v0,11\n";
	} // 输出char型表达式
	cout << "syscall\n";
	outFile << "syscall\n";
}

void mips_scanf() {
	for (int i = mips_para.size() - 1; i >= 0; i--) {
		if (mips_para[i].level != 1 && mips_para[i].level != mid_lev)
			continue;
		else if (mips_para[i].name == word[1]) {
			if (mips_para[i].type == "int") {
				cout << "li $v0,5\n";
				outFile << "li $v0,5\n";
			}
			else {
				cout << "li $v0,12\n";
				outFile << "li $v0,12\n";
			}
			cout << "syscall\n";
			outFile << "syscall\n";
			if (mips_para[i].isreg == 1) {
				cout << "addu " << mips_para[i].reg << ",$v0,$zero\n";
				outFile << "addu " << mips_para[i].reg << ",$v0,$zero\n";
			}
			else {
				if (mips_para[i].level == 1) {
					cout << "sw $v0," << mips_para[i].offset << "($gp)\n";
					outFile << "sw $v0," << mips_para[i].offset << "($gp)\n";
				}
				else {
					cout << "sw $v0," << mips_para[i].offset << "($sp)\n";
					outFile << "sw $v0," << mips_para[i].offset << "($sp)\n";
				}
			}
			break;
		}
	}
}

void mips_return() {
	string regi = "$v0";
	if (isnumber(word[1])) {
		cout << "li $v0," << word[1] << "\n";
		outFile << "li $v0," << word[1] << "\n";
	}
	else
		regi = find_t(word[1], "$v0");
	if (regi != "$v0") {
		cout << "addu $v0,$zero," << regi << "\n";
		outFile << "addu $v0,$zero," << regi << "\n";
	}
	int need = 1;
	if (func_name == "main")
		need = 1;
	else {
		for (int i = 0; i < func.size(); i++) {
			if (string(func[i].name) == func_name) {
				if (func[i].hascall == 0)
					need = 0;
				break;
			}
		}
	}
	if (need) {
		cout << "lw $ra,0($sp)\n";
		outFile << "lw $ra,0($sp)\n"; // 恢复ra
	}
	cout << "jr $ra\n";
	outFile << "jr $ra\n"; // 恢复ra
} // 需要由return语句来控制函数返回

void add_intt() {
	// 现在只会出现int t = RET或者int t = k i
	// 需要分类讨论，看右边是直接的值还是参数
	para1.type = "int";
	para1.name = word[1];
	para1.level = mid_lev;
	para1.dim = 0;
	string regi;
	// 用寄存器t7来表示并储存结果
	para1.offset = sp;
	sp -= 4;
	mips_para.push_back(para1);
	int judge = 0;
	for (int i = reg_limit + 1; i < temp_reg.size(); i++) {
		if (temp_reg[i].isuse == 0) {
			temp_reg[i].isuse = 1;
			temp_reg[i].name = word[1];
			if (word[3] != "RET") {
				cout << "addu " << temp_reg[i].reg << "," << regi << ",$zero\n";
				outFile << "addu " << temp_reg[i].reg << "," << regi << ",$zero\n";
			}
			else {
				cout << "addu " << temp_reg[i].reg << ",$v0,$zero\n";
				outFile << "addu " << temp_reg[i].reg << ",$v0,$zero\n";
				judge = 1;
			}
			return;
		}
	}
	if (!judge) {
		if (word[3] != "RET") {
			cout << "sw " << regi << "," << para1.offset << "($sp)\n";
			outFile << "sw " << regi << "," << para1.offset << "($sp)\n";
		}
		else {
			cout << "sw $v0," << para1.offset << "($sp)\n";
			outFile << "sw $v0," << para1.offset << "($sp)\n";
		}
	}
}

void add_chart() {
	// 需要分类讨论，看右边是直接的值还是参数
	// 还需要看右边的第3个是否是RET
	para1.type = "char";
	para1.name = word[1];
	para1.level = mid_lev;
	para1.dim = 0;
	string regi;
	// 用寄存器t7来表示并储存结果
	para1.offset = sp;
	sp -= 4;
	mips_para.push_back(para1);
	int judge = 0;
	for (int i = reg_limit + 1; i < temp_reg.size(); i++) {
		if (temp_reg[i].isuse == 0) {
			temp_reg[i].isuse = 1;
			temp_reg[i].name = word[1];
			if (word[3] != "RET") {
				cout << "addu " << temp_reg[i].reg << "," << regi << ",$zero\n";
				outFile << "addu " << temp_reg[i].reg << "," << regi << ",$zero\n";
			}
			else {
				judge = 1;
				cout << "addu " << temp_reg[i].reg << ",$v0,$zero\n";
				outFile << "addu " << temp_reg[i].reg << ",$v0,$zero\n";
			}
			return;
		}
	}
	if (!judge) {
		if (word[3] != "RET") {
			cout << "sw " << regi << "," << para1.offset << "($sp)\n";
			outFile << "sw " << regi << "," << para1.offset << "($sp)\n";
		}
		else {
			cout << "sw $v0," << para1.offset << "($sp)\n";
			outFile << "sw $v0," << para1.offset << "($sp)\n";
		}
	}
}

void cal_t() {
	string cal;
	if (word[0] == "PLUS")
		cal = "add";
	else if (word[0] == "MINU")
		cal = "sub";
	else if (word[0] == "MULT")
		cal = "mult";
	else if (word[0] == "DIV")
		cal = "div";
	string reg1 = "$t7";
	string reg2 = "$t8";
	string regi1 = "$t7";
	string regi2 = "$t8";
	string resultreg = "$t9";
	resultreg = set_treg(word[1], "$t9");
	if (isnumber(word[2])) {
		cout << "li $t7," << word[2] << "\n";
		outFile << "li $t7," << word[2] << "\n";
	}
	else
		regi1 = find_t(word[2], reg1);
	if (cal == "add" || cal == "sub") {
		if (word[2] == "0") {
			if (isnumber(word[3])) {
				cout << "li $t8," << word[3] << "\n";
				outFile << "li $t8," << word[3] << "\n";
			}
			else
				regi2 = find_t(word[3], reg2);
			cout << cal << " " + resultreg + ",$zero," << regi2 << "\n";
			outFile << cal << " " + resultreg + ",$zero," << regi2 << "\n";
		}
		else {
			if (isnumber(word[3])) {
				int number = atoi(word[3].c_str());
				if (cal == "sub") {
					number = number * -1;
				}
				cout << "addi " + resultreg + "," << regi1 << "," << to_string(number) << "\n";
				outFile << "addi " + resultreg + "," << regi1 << "," << to_string(number) << "\n";
			}
			else {
				regi2 = find_t(word[3], reg2);
				cout << cal << " " + resultreg + "," << regi1 << "," << regi2 << "\n";
				outFile << cal << " " + resultreg + "," << regi1 << "," << regi2 << "\n";
			}
		}
	}
	else {
		if (isnumber(word[3])) {
			cout << "li $t8," << word[3] << "\n";
			outFile << "li $t8," << word[3] << "\n";
		}
		else
			regi2 = find_t(word[3], reg2);
		if (cal == "mult") {
			cout << "mul " + resultreg + "," << regi1 << "," << regi2 << "\n";
			outFile << "mul " + resultreg + "," << regi1 << "," << regi2 << "\n";
		}
		else {
			cout << cal << " " << regi1 << "," << regi2 << "\n";
			outFile << cal << " " << regi1 << "," << regi2 << "\n";
			cout << "mflo " + resultreg + "\n";
			outFile << "mflo " + resultreg + "\n";
		}
	}
	// 开始添加最新变量的信息
	para1.name = word[1];
	para1.type = "int";
	para1.level = mid_lev;
	para1.offset = sp;
	if (resultreg == "$t9") {
		cout << "sw $t9," << para1.offset << "($sp)\n";
		outFile << "sw $t9," << para1.offset << "($sp)\n";
	}
	sp -= 4;
	mips_para.push_back(para1);
}

void assign_t() { // 对应三个词，类似change1=t3这种情况
	// 第一遍取出后者的值
	// 第二遍找到前者的位置
	int isiden = 0;
	string regi = "$t7";
	for (int i = 0; i < word[2].size(); i++) {
		if ((!(word[2][i] >= '0' && word[2][i] <= '9')) && word[2][i] != '+' && word[2][i] != '-') {
			isiden = 1;
		}
	}
	/*
	if (isiden)
		regi = find_t(word[2], "$t7");
	else {
		cout << "li $t7," << word[2] << "\n";
		outFile << "li $t7," << word[2] << "\n";
	}
	*/
	for (int i = mips_para.size() - 1; i >= 0; i--) {
		if (mips_para[i].level != 1 && mips_para[i].level != mid_lev)
			continue;
		else if (mips_para[i].name == word[0]) {
			if (mips_para[i].isreg == 1) {
				if (isiden) {
					regi = find_t(word[2], mips_para[i].reg);
					if (regi != mips_para[i].reg) {
						cout << "addu " + mips_para[i].reg + ",$zero," + regi + "\n";
						outFile << "addu " + mips_para[i].reg + ",$zero," + regi + "\n";
					}
				}
				else {
					cout << "li " << mips_para[i].reg << "," << word[2] << "\n";
					outFile << "li " << mips_para[i].reg << "," << word[2] << "\n";
				}
			}
			else {
				if (isiden)
					regi = find_t(word[2], "$t7");
				else {
					cout << "li $t7," << word[2] << "\n";
					outFile << "li $t7," << word[2] << "\n";
				}
				if (mips_para[i].level == 1) {
					cout << "sw " << regi << "," << mips_para[i].offset << "($gp)\n";
					outFile << "sw " << regi << "," << mips_para[i].offset << "($gp)\n";
				}
				else {
					cout << "sw " << regi << "," << mips_para[i].offset << "($sp)\n";
					outFile << "sw " << regi << "," << mips_para[i].offset << "($sp)\n";
				}
			}
			break; // 需要及时终止避免错误赋值
		}
	}
}

string find_t(string paraname, string reg) {
	for (int i = reg_limit + 1; i < regname.size(); i++) {
		if (temp_reg[i].isuse == 1)
			if (temp_reg[i].name == paraname) {
				temp_reg[i].isuse = 0;
				return temp_reg[i].reg;
			}
	}
	for (int i = mips_para.size() - 1; i >= 0; i--) {
		if (mips_para[i].level != 1 && mips_para[i].level != mid_lev)
			continue;
		else if (mips_para[i].name == paraname) {
			if (mips_para[i].isreg == 1) {
				return mips_para[i].reg;
			}
			else {
				if (mips_para[i].level == 1) {
					cout << "lw " << reg << "," << mips_para[i].offset << "($gp)\n";
					outFile << "lw " << reg << "," << mips_para[i].offset << "($gp)\n";
				}
				else {
					cout << "lw " << reg << "," << mips_para[i].offset << "($sp)\n";
					outFile << "lw " << reg << "," << mips_para[i].offset << "($sp)\n";
				}
			}
			break;
		}
	}
	return reg;
}

void mips_condition(int num) {
	string name1 = word[2];
	string name2;
	string relation = word[1];
	string regi1 = "$t7";
	string regi2 = "$t8";
	// 分别使用t7和t8来处理
	if (isnumber(name1)) {
		cout << "li $t7," << name1 << "\n";
		outFile << "li $t7," << name1 << "\n";
	}
	else
		regi1 = find_t(name1, "$t7");
	string label;
	if (num == 4) {
		//bgtz $t7,label
		label = word[3];
		cout << relation << " " << regi1 << "," << label << "\n";
		outFile << relation << " " << regi1 << "," << label << "\n";
	}
	else if (num == 5) {
		// condition bne t7 t8 label
		name2 = word[3];
		if (isnumber(name2)) {
			cout << "li $t8," << name2 << "\n";
			outFile << "li $t8," << name2 << "\n";
			regi2 = "$t8";
		}
		else if (name2 == "t0")
			regi2 = "$zero";
		else {
			regi2 = find_t(name2, "$t8");
		}
		label = word[4];
		cout << relation << " " << regi1 << "," << regi2 << "," << label << "\n";
		outFile << relation << " " << regi1 << "," << regi2 << "," << label << "\n";
	}
}

void para_add() { // 此时直接储存
	parameter para_temp;
	para_temp.type = word[1];
	para_temp.level = mid_lev;
	para_temp.name = word[2];
	para_temp.dim = 0;
	para_temp.offset = sp;
	if (getpara_num < 3) {
		getpara_num++;
		para_temp.isreg = 1;
		para_temp.reg = "$a" + to_string(getpara_num);
	}
	mips_para.push_back(para_temp); // 将新读入的信息添加进后面的内容
	para_num++;
	sp -= 4;
	/*
	if (para_num == func_para.size()) {
		func_para.clear();
		para_num = 0;
	}
	*/ // 似乎不应该在这里进行处理
}

void mips_push() {
	func_para.push_back(word[1]);
}

void mips_call() { // 对sp进行修改
	// call的时候存之前push的变量，到para的时候只需要声明一下即可
	// 储存ra对应的地址
	/*for (int i = 0; i < mips_para.size(); i++) {
		if (mips_para[i].isreg == 1)
			cout << mips_para[i].name + "  " + "isreg:" + to_string(mips_para[i].isreg) + " " + mips_para[i].reg << endl;
	}*/
	int sum = sp;
	vector<treg> x = temp_reg;
	// 用t9暂时储存下sp内容
	cout << "addu $t9,$sp,$zero\n";
	outFile << "addu $t9,$sp,$zero\n"; // 处理sp前先处理内容
	cout << "addi $sp,$sp," << sum << "\n";
	outFile << "addi $sp,$sp," << sum << "\n"; // 把不符合条件的内容删掉
	// 开始储存之前对应的push参数
	int need;
	need = getInfo(word[1]);
	cout << "开始进行特殊判断:" << word[1] << "                             need:" << need << endl;
	reg_save(need); // 储存当前函数的寄存器信息
	for (int i = 0; i < func_para.size(); i++) {
		int t = -4 * (i + 1);
		string reg = "$t7";
		if (i < 3) {
			reg = "$a" + to_string(i + 1);
		}
		if (isnumber(func_para[i])) {
			cout << "li " << reg << "," << func_para[i] << "\n";
			outFile << "li " << reg << "," << func_para[i] << "\n";
		}
		else {
			int judge = 0;
			string reg1;
			for (int j = reg_limit + 1; j < temp_reg.size(); j++) {
				if (temp_reg[j].isuse == 1) {
					if (temp_reg[j].name == func_para[i]) {
						reg1 = temp_reg[j].reg;
						temp_reg[j].isuse = 0;
						judge = 1;
						if (i < 3) {
							cout << "addu " + reg + ",$zero," + reg1 + "\n";
							outFile << "addu " + reg + ",$zero," + reg1 + "\n";
						}
						else
							reg = reg1;
						break;
					}
				}
			}
			if (!judge) {
				for (int j = mips_para.size() - 1; j >= 0; j--) {
					if (mips_para[j].level != 1 && mips_para[j].level != mid_lev)
						continue;
					else if (mips_para[j].name == func_para[i]) {
						if (mips_para[j].isreg == 1 && mips_para[j].reg[1] != 'a') {
							reg1 = mips_para[j].reg;
							if (i < 3) {
								cout << "addu " + reg + ",$zero," + reg1 + "\n";
								outFile << "addu " + reg + ",$zero," + reg1 + "\n";
							}
							else
								reg = reg1;
						} // 如果是a寄存器则会存在循环迭代产生障碍的问题
						else if (mips_para[j].level == 1) {
							cout << "lw " << reg << "," << mips_para[j].offset << "($gp)\n";
							outFile << "lw " << reg << "," << mips_para[j].offset << "($gp)\n";
						}
						else {
							cout << "lw " << reg << "," << mips_para[j].offset << "($t9)\n";
							outFile << "lw " << reg << "," << mips_para[j].offset << "($t9)\n";
						}
						break;
					}
				}
			}
		}
		/*
		if (i < 3 ) {
			cout << "sw " + reg + "," << t << "($sp)\n";
			outFile << "sw " + reg + "," << t << "($sp)\n";
		}
		if (i >= 3) {
			cout << "sw " + reg + "," << t << "($sp)\n";
			outFile << "sw " + reg + "," << t << "($sp)\n";
		}
		*/
		cout << "sw " + reg + "," << t << "($sp)\n";
		outFile << "sw " + reg + "," << t << "($sp)\n";
	} // 储存函数参数
	// sp = 0; 这句话似乎判断失误
	cout << "jal " << word[1] << "\n";
	outFile << "jal " << word[1] << "\n";
	func_para.clear();
	para_num = 0;
	sp = sum;
	sum *= -1;
	cout << "addi $sp,$sp," << sum << "\n";
	outFile << "addi $sp,$sp," << sum << "\n"; // 立刻把栈加回来
	temp_reg = x; // 临时变量记录寄存器使用情况
	reg_recover(need); // 恢复本函数寄存器的信息
}

void mips_function() {
	save = 0;
	sp = 0;
	getpara_num = 0;
	cout << word[2] << ":\n";
	outFile << word[2] << ":\n";
	func_name = word[2];
	int need = 1;
	if (word[2] == "main")
		need = 1;
	else {
		for (int i = 0; i < func.size(); i++) {
			if (string(func[i].name) == word[2]) {
				if (func[i].hascall == 0)
					need = 0;
				break;
			}
		}
	}
	if (need) {
		cout << "sw $ra," << sp << "($sp)\n";
		outFile << "sw $ra," << sp << "($sp)\n";
	}
	sp -= 4;
}

void function_end() {
	for (int i = mips_para.size() - 1; i >= 0; i--) {
		if (mips_para[i].level == 1)
			break;
		else {
			if (mips_para[i].isreg == 1) {
				if (mips_para[i].reg[1] != 'a') {
					regtime--;
				}
			}
			mips_para.pop_back();
		}
	}
	int need = 1;
	if (func_name == "main")
		need = 1;
	else {
		for (int i = 0; i < func.size(); i++) {
			if (string(func[i].name) == func_name) {
				if (func[i].hascall == 0)
					need = 0;
				break;
			}
		}
	}
	if (need) {
		cout << "lw $ra,0($sp)\n";
		outFile << "lw $ra,0($sp)\n"; // 恢复ra
	}
	cout << "jr $ra\n";
	outFile << "jr $ra\n"; // 恢复ra
}


void mips_goto() {
	// 无条件直接跳转
	cout << "j " << word[1] << "\n";
	outFile << "j " << word[1] << "\n";
}

void mips_return1() { // 直接返回时，无需进行任何返回项处理
	int need = 1;
	if (func_name == "main")
		need = 1;
	else {
		for (int i = 0; i < func.size(); i++) {
			if (string(func[i].name) == func_name) {
				if (func[i].hascall == 0)
					need = 0;
				break;
			}
		}
	}
	if (need) {
		cout << "lw $ra,0($sp)\n";
		outFile << "lw $ra,0($sp)\n"; // 恢复ra
	}
	cout << "jr $ra\n";
	outFile << "jr $ra\n"; // 恢复ra
}

// int t6 = arr t5  类似这种结构
void add_intt1() {
	// 不需要讨论，右边必然为数组
	para1.type = "int";
	para1.name = word[1];
	para1.level = mid_lev;
	para1.dim = 0;
	// 用寄存器t7来表示并储存结果
	string reg;
	reg = set_treg(word[1], "$t7");
	find_array(word[3], word[4], reg);
	para1.offset = sp;
	sp -= 4;
	if (reg == "$t7") {
		cout << "sw $t7," << para1.offset << "($sp)\n";
		outFile << "sw $t7," << para1.offset << "($sp)\n";
	}
	mips_para.push_back(para1);
}

void add_chart1() {
	para1.type = "char";
	para1.name = word[1];
	para1.level = mid_lev;
	para1.dim = 0;
	// 用寄存器t7来表示并储存结果
	string reg;
	reg = set_treg(word[1], "$t7");
	find_array(word[3], word[4], reg);
	para1.offset = sp;
	sp -= 4;
	if (reg == "$t7") {
		cout << "sw $t7," << para1.offset << "($sp)\n";
		outFile << "sw $t7," << para1.offset << "($sp)\n";
	}
	mips_para.push_back(para1);
}

// arr t3 = t4 ;
void assign1_t() {
	int isiden = 0;
	string reg = "$t7";
	string arrreg = "$t8";
	for (int i = 0; i < word[3].size(); i++) {
		if ((!(word[3][i] >= '0' && word[3][i] <= '9')) && word[3][i] != '+' && word[3][i] != '-') {
			isiden = 1;
		}
	}
	if (isiden)
		reg = find_t(word[3], "$t7");
	else {
		cout << "li $t7," << word[3] << "\n";
		outFile << "li $t7," << word[3] << "\n";
	}
	//cout << "word[0]:" + word[0] << endl;
	for (int i = mips_para.size() - 1; i >= 0; i--) {
		//cout << "当前变量名字:" + mips_para[i].name << endl;
		if (mips_para[i].level != 1 && mips_para[i].level != mid_lev)
			continue;
		if (mips_para[i].dim != 1)
			continue;
		else if (mips_para[i].name == word[0]) {
			//cout << "找到了word[0]:" + word[0] << endl;
			// 先需要找到之前的值，然后计算偏差量
			if (isnumber(word[1])) {
				cout << "li $t8," << word[1] << "\n";
				outFile << "li $t8," << word[1] << "\n";
			}
			else
				arrreg = find_t(word[1], "$t8"); // 得到具体的数表示数组第几项
			cout << "sll " << "$t8," << arrreg << ",2\n";
			outFile << "sll " << "$t8," << arrreg << ",2\n";
			if (mips_para[i].isreg == 1) {
				if (mips_para[i].level == 1) {
					cout << "addu $t8,$t8," + mips_para[i].reg + "\n";
					outFile << "addu $t8,$t8," + mips_para[i].reg + "\n";
				}
				else {
					cout << "sub $t8," + mips_para[i].reg + ",$t8\n";
					outFile << "sub $t8," + mips_para[i].reg + ",$t8\n";
				}
			}
			else {
				if (mips_para[i].level == 1) {
					cout << "addi $t8,$t8," << mips_para[i].offset << "\n"; // 此项必定为正数
					outFile << "addi $t8,$t8," << mips_para[i].offset << "\n";
					cout << "add $t8,$t8,$gp\n";
					outFile << "add $t8,$t8,$gp\n";
				}
				else {
					cout << "sub $t8,$sp,$t8\n";
					outFile << "sub $t8,$sp,$t8\n";
					cout << "addi $t8,$t8," << mips_para[i].offset << "\n"; // 此项必定为正数
					outFile << "addi $t8,$t8," << mips_para[i].offset << "\n";
				}
			}
			break;
		}
	}
	cout << "sw " << reg << ",0($t8)\n";
	outFile << "sw " << reg << ",0($t8)\n";
}

void find_array(string arrayname, string off, string reg) {
	// 值得注意的是off也是形如t的形式而非单纯的数字或字符
	string reg1 = "$t8";
	for (int i = mips_para.size() - 1; i >= 0; i--) {
		if (mips_para[i].level != 1 && mips_para[i].level != mid_lev)
			continue;
		if (mips_para[i].dim != 1)
			continue;
		else if (mips_para[i].name == arrayname) {
			// 先需要找到之前的值，然后计算偏差量
			if (isnumber(off)) {
				cout << "li $t8," << off << "\n";
				outFile << "li $t8," << off << "\n";
			}
			else
				reg1 = find_t(off, "$t8"); // 得到具体的数表示数组第几项
			cout << "sll " << "$t8," << reg1 << ",2\n";
			outFile << "sll " << "$t8," << reg1 << ",2\n";
			if (mips_para[i].isreg == 1) {
				if (mips_para[i].level == 1) {
					cout << "addu $t8,$t8," << mips_para[i].reg << "\n";
					outFile << "addu $t8,$t8," << mips_para[i].reg << "\n";
				}
				else {
					cout << "sub $t8," << mips_para[i].reg << ",$t8\n";
					outFile << "sub $t8," << mips_para[i].reg << ",$t8\n";
				}
			}
			else {
				if (mips_para[i].level == 1) {
					cout << "addi $t8,$t8," << mips_para[i].offset << "\n"; // 此项必定为正数
					outFile << "addi $t8,$t8," << mips_para[i].offset << "\n";
					cout << "add $t8,$t8,$gp\n";
					outFile << "add $t8,$t8,$gp\n";
				}
				else {
					cout << "sub $t8,$sp,$t8\n";
					outFile << "sub $t8,$sp,$t8\n";
					cout << "addi $t8,$t8," << mips_para[i].offset << "\n"; // 此项必定为正数
					outFile << "addi $t8,$t8," << mips_para[i].offset << "\n";
				}
			}
			cout << "lw " << reg << ",0($t8)\n";
			outFile << "lw " << reg << ",0($t8)\n";
			break;
		}
	}
}

void word_print(int line_word) {
	for (int i = 0; i < line_word; i++)
		cout << word[i] << " ";
	cout << endl;
}

int isnumber(string x) {
	for (int i = 0; i < x.size(); i++) {
		if (!(x[i] >= '0' && x[i] <= '9') && x[i] != '+' && x[i] != '-')
			return 0;
	}
	return 1;
}

void reginitial() {
	char regcondition[20][100] = { "$t0","$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7" };
	for (int i = 0; i < 15; i++) {
		regname.push_back(string(regcondition[i]));
	}
}

void reg_save(int need) {
	string reg;
	if (need == 0) {
		for (int i = 0; i < mips_para.size(); i++) {
			if (mips_para[i].level != 1) {
				if (mips_para[i].isreg == 1) {
					reg = mips_para[i].reg;
					cout << "sw " << reg << "," << mips_para[i].offset << "($t9)\n";
					outFile << "sw " << reg << "," << mips_para[i].offset << "($t9)\n";
				}
			} // 非全局变量出让寄存器
		} // 如果之前有a寄存器说明其本身是在其他函数内部调用，也应该进行储存
	}
	for (int i = 0; i < mips_para.size(); i++) {
		if (mips_para[i].level != 1) {
			if (mips_para[i].isreg == 1 && need == 1 && mips_para[i].reg[1] == 'a') {
				reg = mips_para[i].reg;
				cout << "sw " << reg << "," << mips_para[i].offset << "($t9)\n";
				outFile << "sw " << reg << "," << mips_para[i].offset << "($t9)\n";
			}
		} // 非全局变量出让寄存器
	} // 如果之前有a寄存器说明其本身是在其他函数内部调用，也应该进行储存
	//cout << "reglimit+1:" << temp_reg[reg_limit + 1].reg;
	for (int i = reg_limit + 1; i < temp_reg.size(); i++) {
		if (temp_reg[i].isuse == 1) { // 函数跳转时仍然没有用到
			for (int j = mips_para.size() - 1; j >= 0; j--) {
				if (mips_para[j].level != 1) {
					if (mips_para[j].name == temp_reg[i].name) {
						reg = temp_reg[i].reg;
						cout << "sw " << reg << "," << mips_para[j].offset << "($t9)\n";
						outFile << "sw " << reg << "," << mips_para[j].offset << "($t9)\n";
						break;
					}
				} // 非全局变量出让寄存器
			} // 如果之前有a寄存器说明其本身是在其他函数内部调用，也应该进行储存
		}
	}
}

void reg_recover(int need) {
	string reg;
	if (need == 0) {
		for (int i = 0; i < mips_para.size(); i++) {
			if (mips_para[i].level != 1) {
				if (mips_para[i].isreg == 1) {
					reg = mips_para[i].reg;
					cout << "lw " << reg << "," << mips_para[i].offset << "($sp)\n";
					outFile << "lw " << reg << "," << mips_para[i].offset << "($sp)\n";
					if (mips_para[i].reg[1] != 'a')
						regtime++;
				}
			} // 非全局变量出让寄存器
		}
	}
	for (int i = 0; i < mips_para.size(); i++) {
		if (mips_para[i].level != 1) {
			if (mips_para[i].isreg == 1 && need == 1 && mips_para[i].reg[1] == 'a') {
				reg = mips_para[i].reg;
				cout << "lw " << reg << "," << mips_para[i].offset << "($sp)\n";
				outFile << "lw " << reg << "," << mips_para[i].offset << "($sp)\n";
			}
		} // 非全局变量出让寄存器
	}
	for (int i = reg_limit + 1; i < temp_reg.size(); i++) {
		if (temp_reg[i].isuse == 1) { // 函数跳转时仍然没有用到
			for (int j = mips_para.size() - 1; j >= 0; j--) {
				if (mips_para[j].level != 1) {
					if (mips_para[j].name == temp_reg[i].name) {
						reg = temp_reg[i].reg;
						cout << "lw " << reg << "," << mips_para[j].offset << "($sp)\n";
						outFile << "lw " << reg << "," << mips_para[j].offset << "($sp)\n";
						break;
					}
				} // 非全局变量出让寄存器
			} // 如果之前有a寄存器说明其本身是在其他函数内部调用，也应该进行储存
		}
	}
}

void treg_initial() {
	char regcondition[20][100] = { "$t0","$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7" };
	treg temp;
	temp.isuse = 0;
	for (int i = 0; i < 15; i++) {
		temp.reg = string(regcondition[i]);
		temp_reg.push_back(temp);
	}
}

string set_treg(string name, string reg) {
	for (int i = reg_limit + 1; i < temp_reg.size(); i++) {
		if (temp_reg[i].isuse == 0) {
			temp_reg[i].isuse = 1;
			temp_reg[i].name = name;
			return temp_reg[i].reg;
		}
	}
	return reg;
}

void print_funcinfo() {
	for (int i = 0; i < func.size(); i++) {
		printf("Function Info  name:%s  para:%d hasreturn:%d var:%d\n", func[i].name, func[i].para_number, func[i].hascall, func[i].var_number);
	}
}

int getInfo(string name) {
	for (int i = 0; i < func.size(); i++) {
		if (string(func[i].name) == name) {
			if (func[i].hascall == 0 && func[i].var_number == 0)
				return 1;
			else
				return 0;
		}
	}
}
