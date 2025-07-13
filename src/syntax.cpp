#include "syntax.h"
using namespace std;
DeclarationParser* DeclParser;
StatementParser* StmtParser;
node* astRoot;
vector<int> syntaxErrorList;
void parser_init(){
    try{
        DeclParser=new DeclarationParser("../config/decl.txt");
        StmtParser=new StatementParser("../config/stmt.txt");
        DeclParser->init();
        StmtParser->init();
    }catch(string& e){
        throw e;
    }
}
void syntactic_analyze(std::vector<token>& tokens){
    try{
        parser_init();
    }catch(string& e){
        throw e;
    }
    if(type_table[tokens.front().index]=="{"
        &&type_table[tokens.back().index]=="}"){
        pair<node*,unsigned long long> declRoot,stmtRoot;
        
        declRoot=DeclParser->parse(tokens,1);
        stmtRoot=StmtParser->parse(tokens,declRoot.second);
        if(DeclParser->parserErrorList.size()!=0){
            // 语法错误
            for(auto& it:DeclParser->parserErrorList){
                syntaxErrorList.push_back(it);
            }
        }
        if(StmtParser->parserErrorList.size()!=0){
            // 语法错误
            for(auto& it:StmtParser->parserErrorList){
                syntaxErrorList.push_back(it);
            }
        }
        astRoot=new node;
        astRoot->self="PROG";
        astRoot->children.clear();
        astRoot->children.push_back(new node);
        astRoot->children.push_back(declRoot.first);
        astRoot->children.push_back(stmtRoot.first);
        astRoot->children.push_back(new node);
        astRoot->children.front()->self="{";
        astRoot->children.front()->children.clear();
        astRoot->children.back()->self="}";
        astRoot->children.back()->children.clear();
    }else{
        // 语法错误
        throw string("Syntax Error: The program must be enclosed in {}");
    }
}
void print_syntaxErrorList(){
    cout<<syntaxErrorList.size()<<" Syntax Error Detected Before: "<<endl;
    for(auto& it:syntaxErrorList){
        cout<<tokenList[it].row<<":"<<tokenList[it].col<<":"<<type_table[tokenList[it].index]<<endl;
    }
}