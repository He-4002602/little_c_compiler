#ifndef SYNTAX_H
#define SYNTAX_H
#include "ll1parser.h"
extern DeclarationParser* DeclParser;  // 声明语句分析器
extern StatementParser* StmtParser;    // 执行语句分析器
extern node* astRoot;  // 语法树根节点
extern std::vector<int> syntaxErrorList;  // 语法错误列表
void parser_init();  // 初始化语法分析器
void syntactic_analyze(std::vector<token>& tokens);  // 语法分析
void print_syntaxErrorList();  // 打印语法错误列表
#endif