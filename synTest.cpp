#include "syntax.h"
using namespace std;
int main(int argc,char* argv[]){
    if(argc!=2){
        cerr<<"The correct input form is:"<<endl;
        cerr<<argv[0]<<" <source program>"<<endl;
        return 0;
    }
    try{
        string sourceName=argv[1],tableName="../config/WordsTable.csv";
        lexical_analyze(sourceName,tableName);
        if(!errorList.empty()){
            print_tokenList("");
            exit(1);
        }
        syntactic_analyze(tokenList);
        if(!syntaxErrorList.empty()){
            print_syntaxErrorList();
            exit(1);
        }
        TreePrinter printer("AST.json");
        printer.print_tree(astRoot);
    }catch(string& e){
        cout<<e<<endl;
    }
    return 0;
}