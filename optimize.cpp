#include <iostream>
#include <stdio.h>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include "optimize.h"
using namespace std;

ifstream inFile1;
ofstream outFile1;
vector<string> midword;
vector<string> midword1; // 重复的t已经不存在
vector<string> midword2;
vector<string> midword3; // 负责处理基本的内容

vector<string> tname;
vector<string> samename;

string wordtemp[6];
int wordnum = 0;


void optimize() {
	inFile1.open("17373217_段牧知_优化前中间代码.txt");
	outFile1.open("17373217_段牧知_优化后中间代码.txt");
	cout << "optimize begin!!!\n\n\n\n\n";
	string inword;
	while (inFile1 >> inword) {
		midword.push_back(inword);
	}
	// show();
	for (int i = 0; i < midword.size(); i++) {
		wordnum = 0;
		while (midword[i] != ";") {
			wordtemp[wordnum] = midword[i];
			i++;
			wordnum++;
		}
		if (wordnum == 4) {
			if (wordtemp[0] == "char" || wordtemp[0] == "int") {
				if (wordtemp[3] != "RET") // 返回函数的v0需要立刻储存
					add_same();
			}
		}
	}
	change_same(); // 第二遍搞定，替换了所有相同的内容
	delete_same();
	show();
	output_finalmid();
	inFile1.close();
	outFile1.close();
}

void show() {
	for (int i = 0; i < midword2.size(); i++) {
		cout << midword2[i] << " ";
		if (midword2[i] == ";")
			cout << endl;
	}
}

void add_same() {
	string x = wordtemp[1];
	string y = wordtemp[3];
	tname.push_back(x);
	while (tfind(y)) {
		for (int i = tname.size() - 1; i >= 0; i--) {
			if (y == tname[i]) {
				y = samename[i];
				break;
			}
		}
		if (y[0] != 't')
			break;
	}
	samename.push_back(y);
} // 处理同名问题

void change_same() {
	for (int i = 0; i < midword.size(); i++) {
		wordnum = 0;
		if (midword[i][0] != 't') {
			midword1.push_back(midword[i]);
			continue;
		}
		int find = 0;
		for (int j = tname.size() - 1; j >= 0; j--) {
			if (tname[j] == midword[i]) {
				find = 1;
				midword1.push_back(samename[j]);
				break;
			}
		}
		if (find == 0)
			midword1.push_back(midword[i]);
	}
}

int tfind(string x) {
	for (int i = tname.size() - 1; i >= 0; i--) {
		if (x == tname[i])
			return 1;
	}
	return 0;
}

void delete_same() {
	for (int i = 0; i < midword1.size(); i++) {
		wordnum = 0;
		while (midword1[i] != ";") {
			wordtemp[wordnum] = midword1[i];
			i++;
			wordnum++;
		}
		if (wordnum == 4) {
			if (wordtemp[0] == "char" || wordtemp[0] == "int") {
				if (wordtemp[1] == wordtemp[3]) {
					continue;
				}
			}
		}
		for (int j = 0; j < wordnum; j++)
			midword2.push_back(wordtemp[j]);
		midword2.push_back(";");
	}
} // 删除所有重复的语句

void output_finalmid() {
	for (int i = 0; i < midword2.size(); i++) {
		outFile1 << midword2[i] << " ";
		if (midword2[i] == ";")
			outFile1 << "\n";
	}
}
