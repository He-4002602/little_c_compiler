#ifndef LEX_H
#define LEX_H
#include "preprocessor.h"
#define is_number(ch) (ch>='0'&&ch<='9')
#define is_character(ch) ((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||ch=='_')
#define is_seperator(str) (type_hash_table[str]>=seperatorI&&type_hash_table[str]<operatorI)
#define is_operator(str) (type_hash_table[str]>=operatorI&&type_hash_table[str]<keywordI)
#define is_keyword(str) (type_hash_table[str]>=keywordI)
#define is_bool(str) (str=="true"||str=="false")
struct token{
    int index,attr;  // 类型表序号 属性或属性表序号
    unsigned long long row,col;  // 行号 列号
};
struct idAttr{
    std::string name;  // 标识符名称
    int type;  // 标识符类型 0未声明 1整型 2布尔型
    bool isValued;  // 是否赋值
};
extern int seperatorI,operatorI,keywordI;  // 分隔符 运算符 关键字在单词表中开始的位置
extern std::vector<token> tokenList;  // 最后词法分析得到的tokens
extern std::unordered_map<std::string,int> type_hash_table,attr_hash_table;  // 用于快速寻找的哈希表
extern std::vector<std::string> type_table;  // 单词表
extern std::vector<idAttr> attr_table;  // 属性表
extern bool is_op_ch[128];  // 某一个字符是否可以组成运算符
extern std::vector<std::string> errorList;  // 错误列表
void print_tokenList(std::string fileName);  // 输出分析结果
void lexical_analyze(std::string fileName,std::string tableName);  // 词法分析方法
#endif