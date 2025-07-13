#include "lex.h"
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
        print_tokenList("lex_test_out.txt");
    }catch(string& e){
        cout<<e<<endl;
    }
    return 0;
}