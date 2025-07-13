#ifndef LL1PARSER_H
#define LL1PARSER_H
#include "treetools.h"
#include "lex.h"
#include "bnf.h"
#include "opparser.h"
class LL1Parser{
    protected:
        std::vector<std::string> VN_order,VT_order;
        std::unordered_map<std::string,int> VN_hash,VT_hash;
        std::vector<std::vector<bool>> First,Follow;
        std::vector<std::vector<int>> analyzingTable;  // 元素指向产生式的下标
        CFG G;
        void find_first();
        void find_follow();
        virtual void find_analyzingTable();
        // void deleteEmpty(node* root);  // 删除空产生式对应节点
        // 错误处理函数
        virtual void error_handle(const std::vector<token>& tokens,std::stack<node*>& st_node,unsigned long long *pos);

    public:
        std::vector<unsigned long long> parserErrorList;  // 错误列表
        void init();
        explicit LL1Parser(const std::string& fileName);
        virtual std::pair<node*,unsigned long long> parse(const std::vector<token>& tokens, const unsigned long long& pos);
        // 调试用
        void print_First();
        void print_Follow();
        void print_analyzingTable();
            
};

// 声明语句分析
class DeclarationParser: public LL1Parser{
    private:
        void error_handle(const std::vector<token>& tokens,std::stack<node*>& st_node,unsigned long long *pos) override;
    public:
        explicit DeclarationParser(const std::string& fileName): LL1Parser(fileName){};
        std::pair<node*,unsigned long long> parse(const std::vector<token>& tokens, const unsigned long long& pos) override;
};

// 执行语句分析
class StatementParser: public LL1Parser{
    protected:
        OperatorPecePrecedenceParser *ExprParser,*BoolParser;
        std::string expr1="../config/BNF.txt",expr2="../config/NonSingle.txt",bool1="../config/BNF1.txt",bool2="../config/NonSingle1.txt";
        void find_analyzingTable() override;
        void error_handle(const std::vector<token>& tokens,std::stack<node*>& st_node,unsigned long long *pos) override;

    public:
        explicit StatementParser(const std::string& fileName)
        : LL1Parser(fileName){
            try{
                ExprParser=new OperatorPecePrecedenceParser(expr1,expr2);
                BoolParser=new OperatorPecePrecedenceParser(bool1,bool2);
            }catch(std::string& e){
                throw e;
            }
        };
        std::pair<node*,unsigned long long> parse(const std::vector<token>& tokens, const unsigned long long& pos) override;
};

#endif