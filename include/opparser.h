#ifndef OPPARSER_H
#define OPPARSER_H
#include "cppinclude.h"
#include "bnf.h"
#include "treetools.h"
#include "lex.h"
#define UNDEFINED 0
#define BIGGER 3
#define EQUAL 2
#define SMALLER 1

class OperatorPecePrecedenceParser{
    private:
        // 按编号查非终结符和终结符名称
        std::vector<std::string> VN_order,VT_order;
        // 按名称查非终结符和终结符编号  
        std::unordered_map<std::string,int> VN_hash,VT_hash; 
        // 优先级关系表
        std::vector<std::vector<int>> priority;
        // 两个关键集合
        // 元素i,j表示编号为j的终结符在编号为i的非终结符对应集合中
        std::vector<std::vector<bool>> FirstVT,LastVT; 
        // 所有可归约项及对应非终结符
        std::unordered_map<std::string,std::string> reduction;
        CFG G,G1;  // 定义了优先级关系的文法 无单非产生式的文法
        // find方法用于构建对应内容
        void find_FirstVT();
        void find_LastVT();
        void find_priority();
        void find_reduction();
        // 初始化方法
        void init();
    public:
        // 构造函数
        // 两个参数分别对应 优先级文法 无单非产生式文法
        explicit OperatorPecePrecedenceParser(const std::string& fileName,const std::string& fileName1);
        // 核心语法分析方法
        // 传入token和分析起始位置
        // 返回值为语法树根节点和分析结束位置
        std::pair<node*,unsigned long long> parse(const std::vector<token>& tokens,const unsigned long long& pos);
        // 调试用 打印优先级关系表
        void print_priority();
};


#endif