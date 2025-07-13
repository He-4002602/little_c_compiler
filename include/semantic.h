#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "blocks.h"
#include "syntax.h"
#define INTEGER 1
#define BOOL 2
extern std::vector<std::string> semanticErrorList;  // 错误列表
extern std::vector<std::string> irCodeList;  // 中间代码列表
extern int varCount;  // 变量计数器
extern std::unordered_map<std::string, std::string> idnameInIr;  // 变量对应哈希表
extern block* curIrBlock;  // 当前生成代码的基本块
extern std::vector<std::string> irHead; // 中间代码头部
extern std::vector<std::string> mainHead; // 主函数头部
extern std::vector<std::string> mainTail; // 主函数尾部
bool type_check(node* root,int type);
void generate_ir(node* root);
void semantic_analyze(node* root);
void print_ir();
#endif