#include "lex.h"
#define START 0
#define END 1
#define IDORKEY 2
#define NUM 3
#define OP 4
using namespace std;
int seperatorI,operatorI,keywordI;
vector<token> tokenList;
unordered_map<string,int> type_hash_table,attr_hash_table;
vector<string> type_table;
vector<idAttr> attr_table;
bool is_op_ch[128];
vector<string> errorList;
// 从文件获取单词表
void read_types(string fileName){
    string index,type;
    ifstream fin(fileName);
    if(!fin.is_open()){
        fin.close();
        throw string("Failed to open file "+fileName);
    }
    while(getline(fin,index,',')){
        getline(fin,type);
        if(type[0]=='\"'&&type.back()=='\"')  // 有的时候带引号
            type=type.substr(1,type.size()-2);
        type_hash_table[type]=stoi(index);
        type_table.push_back(type);  // 这里需要确保输入的csv中index是按顺序排列的
        if(type==";")
            seperatorI=stoi(index);
        if(type=="=")
            operatorI=stoi(index);
        if(type=="int")
            keywordI=stoi(index);
        // 单独存储一下属于运算符的字符
        if(operatorI!=0&&keywordI==0){
            for(auto it:type)
                is_op_ch[it]=1;
        }
    }
    fin.close();
}
// 从loc开始获取一个token
token get_a_token(string& content,unsigned long long loc){
    token ret={0};
    int state,nextstate;
    string temp="";
    state=nextstate=START;
    for(unsigned long long i=loc;i<=content.size()+1;++i){
        state=nextstate;
        switch(state){
            case START:
                temp+=content[i];
                if(is_seperator(content.substr(i,1))){
                    nextstate=END;
                    ret.index=type_hash_table[temp];
                    break;
                }
                if(is_character(content[i])){
                    nextstate=IDORKEY;
                    break;
                }
                if(is_number(content[i])){
                    nextstate=NUM;
                    break;
                }
                if(content[i]>=0&&is_op_ch[content[i]]){
                    nextstate=OP;
                    break;
                }
                // 其他情况属于非法字符
                // 这里把该字符值放入属性的位置 用于向上输出错误的内容
                nextstate=END;
                ret.attr=(unsigned char)content[i];
                break;
            case IDORKEY:
                if(i!=content.size()&&(is_character(content[i])||is_number(content[i]))){
                    temp+=content[i];
                    nextstate=IDORKEY;
                    break;
                }
                nextstate=END;
                // 如果是布尔常量直接赋值
                if(is_bool(temp)){
                    ret.index=3;
                    ret.attr=(temp=="true");
                    break;
                }
                ret.index=type_hash_table[temp];
                if(!ret.index){
                    ret.index=1;  // 没找到 认为是标识符
                    if(!attr_hash_table[temp]){
                        attr_table.push_back({temp,0,false});  // 语义分析实验修改属性构成
                        ret.attr=attr_hash_table[temp]=attr_table.size()-1;
                    }else ret.attr=attr_hash_table[temp];
                }
                break;
            case NUM:
                if(i!=content.size()&&is_number(content[i])){
                    temp+=content[i];
                    nextstate=NUM;
                    break;
                }
                nextstate=END;
                ret.index=2;
                ret.attr=stoi(temp);
                break;
            case OP:
                if(i!=content.size()&&is_operator(temp+content[i])){
                    temp+=content[i];
                    nextstate=OP;
                    break;
                }
                nextstate=END;
                ret.index=type_hash_table[temp];
                if(!ret.index)  // 无法识别的运算符
                    ret.attr=temp[0];
                break;
            case END:
                return ret;
        }
    }
}
void print_tokenList(string fileName){
    if(!errorList.empty()){
        for(auto it:errorList)
            cout<<"error: "<<it<<endl;
        return;
    }
    for(auto it:tokenList){
        cout<<"<"<<it.index<<","<<it.attr<<","<<it.row<<","<<it.col<<">";
        if(it.index==1)
            cout<<" Identifier "<<attr_table[it.attr].name;
        if(it.index==2)
            cout<<" Constant "<<it.attr;
        if(it.index==3)
            cout<<" BooleanConstant "<<(it.attr?"true":"false");
        if(it.index>3&&it.index<operatorI)
            cout<<" Seperator "<<type_table[it.index];
        if(it.index>=operatorI&&it.index<keywordI)
            cout<<" Operator "<<type_table[it.index];
        if(it.index>=keywordI)
            cout<<" Keyword "<<type_table[it.index];
        cout<<endl;
    }
    ofstream fout;
    if(fileName!="")
        fout.open(fileName);
    else
        fout.open("out.txt");
    for(auto it:tokenList)
        fout<<"<"<<it.index<<","<<it.attr<<","<<it.row<<","<<it.col<<">";
    fout.close();
}
void lexical_analyze(string fileName,string tableName){
    if(tableName.substr(tableName.size()-4,4)!=".csv")
        throw string("The table file must be a CSV file!");
    string content;
    try{
        // 预处理
        content=preprocess(fileName);
    }catch(const string& e){
        throw e;  // 继续向上throw
    }
    type_table.push_back("");
    attr_table.push_back({""});
    try{
        read_types(tableName);
    }catch(const string& e){
        throw e;  // 继续向上throw
    }
    unsigned long long row=1,col=0;
    token tempToken;
    for(unsigned long long i=0;i<content.size();++i){
        ++col;
        switch(content[i]){
            case '\n':
                ++row;
                col=0;
                break;
            case ' ':
                break;
            default:
                tempToken=get_a_token(content,i);
                if(!tempToken.index){
                    if(tempToken.attr<128&&is_op_ch[tempToken.attr]){
                        stringstream ss;
                        ss<<"Undefined '"<<(char)tempToken.attr<<"' ("<<row<<": "<<col<<")";
                        errorList.push_back(ss.str());
                        break;
                    }
                    stringstream ss;
                    ss<<"Invalid '\\"<<oct<<tempToken.attr<<"' ("<<dec<<row<<": "<<col<<")";
                    errorList.push_back(ss.str());
                    break;
                }
                if(tempToken.index==1&&attr_table[tempToken.attr].name.size()>8){
                    stringstream ss;
                    ss<<"Identifier "<<attr_table[tempToken.attr].name<<" too long ("<<row<<": "<<col<<")";
                    errorList.push_back(ss.str());
                }
                if(tempToken.index==2&&tempToken.attr>99999999){
                    stringstream ss;
                    ss<<"Constant "<<tempToken.attr<<" out of range ("<<row<<": "<<col<<")";
                    errorList.push_back(ss.str());
                }
                tempToken.row=row;
                tempToken.col=col;
                tokenList.push_back(tempToken);
                switch(tempToken.index){
                    case 1:
                        i+=(attr_table[tempToken.attr].name.size()-1);
                        col+=(attr_table[tempToken.attr].name.size()-1);
                        break;
                    case 2:
                        i+=(to_string(tempToken.attr).size()-1);
                        col+=(to_string(tempToken.attr).size()-1);
                        break;
                    case 3:
                        i+=(tempToken.attr?3:4);
                        col+=(tempToken.attr?3:4);
                        break;
                    default:
                        i+=(type_table[tempToken.index].size()-1);
                        col+=(type_table[tempToken.index].size()-1);
                }
        }
    }
}