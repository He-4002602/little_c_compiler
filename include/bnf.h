#ifndef BNF_H
#define BNF_H
#define WLH  0
#define RLH  1
#define WRH  2
#define RRH  3
#define RVN  4
#define RVT1 5
#define RVT2 6
#include "cppinclude.h"
#include "preprocessor.h"  // get_file_size
// 产生式结构体
struct production{
    std::string LH;  // 左部
    std::vector<std::string> RH;  // 右部
};
// 文法结构体
struct CFG{
    std::vector<production> P;  // 所有产生式
    std::unordered_set<std::string> VN,VT;  // 非终结符和终结符集合
    std::string S;  // 开始符号
};
CFG read_BNF(std::string fileName);
void print_BNF(const CFG& G);
#endif