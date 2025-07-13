#include "opparser.h"
using namespace std;
void OperatorPecePrecedenceParser::find_FirstVT(){
    FirstVT.clear();
    FirstVT.resize(G.VN.size()+1, vector<bool>(G.VT.size()+1, 0));
    bool changed=true;
    while(changed){
        changed=false;
        for(auto& it:G.P){
            int thisVN=VN_hash[it.LH],firstVN=0,firstVT=0;
            for(auto& subit:it.RH){
                if(G.VN.find(subit)!=G.VN.end()&&!firstVN)
                    firstVN=VN_hash[subit];
                if(G.VT.find(subit)!=G.VT.end()){
                    firstVT=VT_hash[subit];
                    break;
                }
            }
            // P -> Ra... | P -> R...
            if(firstVN){
                for(int i=1;i<=G.VT.size();++i){
                    if(FirstVT[firstVN][i]&&!FirstVT[thisVN][i]){
                        FirstVT[thisVN][i]=true;
                        changed=true;
                    }
                }
            }
            if(firstVT){
                if(!FirstVT[thisVN][firstVT]){
                    FirstVT[thisVN][firstVT]=true;
                    changed=true;
                }
            }
        }
    }
}
void OperatorPecePrecedenceParser::find_LastVT(){
    LastVT.clear();
    LastVT.resize(G.VN.size()+1, vector<bool>(G.VT.size()+1, 0));
    bool changed=true;
    while(changed){
        changed=false;
        for(auto& it:G.P){
            int thisVN=VN_hash[it.LH],lastVN=0,lastVT=0;
            for(int i=it.RH.size()-1;i>=0;--i){
                if(G.VN.find(it.RH[i])!=G.VN.end()&&!lastVN)
                    lastVN=VN_hash[it.RH[i]];
                if(G.VT.find(it.RH[i])!=G.VT.end()){
                    lastVT=VT_hash[it.RH[i]];
                    break;
                }
            }
            // P -> ...Q | P -> ...aQ
            if(lastVN){
                for(int i=1;i<=G.VT.size();++i){
                    if(LastVT[lastVN][i]&&!LastVT[thisVN][i]){
                        LastVT[thisVN][i]=true;
                        changed=true;
                    }
                }
            }
            if(lastVT){
                if(!LastVT[thisVN][lastVT]){
                    LastVT[thisVN][lastVT]=true;
                    changed=true;
                }
            }
        }
    }

}
void OperatorPecePrecedenceParser::find_priority(){
    priority.clear();
    priority.resize(G.VT.size()+1, vector<int>(G.VT.size()+1, UNDEFINED));
    for(auto& it:G.P){
        string now,nxt;
        for(int i=0;i<it.RH.size()-1;++i){
            now=it.RH[i];
            nxt=it.RH[i+1];
            if(G.VT.find(now)!=G.VT.end()){
                // 终结符
                if(G.VT.find(nxt)!=G.VT.end()){
                    // 下一个也是终结符 a=b
                    if(priority[VT_hash[now]][VT_hash[nxt]]==BIGGER||priority[VT_hash[now]][VT_hash[nxt]]==SMALLER){
                        // 可以找到超过一种优先级关系
                        // 说明不是算符优先文法 报错
                        stringstream ss;
                        ss<<"Invalid Grammer: more than one priority relation!";
                        throw string(ss.str());
                    }
                    if(priority[VT_hash[now]][VT_hash[nxt]]==UNDEFINED)
                        priority[VT_hash[now]][VT_hash[nxt]]=EQUAL;
                }else{
                    // 下一个是非终结符 a<FIRSTVT(b)
                    for(int i=1;i<=G.VT.size();++i){
                        if(FirstVT[VN_hash[nxt]][i]){
                            if(priority[VT_hash[now]][i]==BIGGER||priority[VT_hash[now]][i]==EQUAL){
                                // 可以找到超过一种优先级关系
                                // 说明不是算符优先文法 报错
                                stringstream ss;
                                ss<<"Invalid Grammer: more than one priority relation!";
                                throw string(ss.str());
                            }
                            if(priority[VT_hash[now]][i]==UNDEFINED)
                                priority[VT_hash[now]][i]=SMALLER;
                        }
                    }
                    if(i+2<it.RH.size()&&G.VT.find(it.RH[i+2])!=G.VT.end()){
                        // 下下一个也是终结符 a=b
                        if(priority[VT_hash[now]][VT_hash[it.RH[i+2]]]==BIGGER||priority[VT_hash[now]][VT_hash[it.RH[i+2]]]==SMALLER){
                            // 可以找到超过一种优先级关系
                            // 说明不是算符优先文法 报错
                            stringstream ss;
                            ss<<"Invalid Grammer: more than one priority relation!";
                            throw string(ss.str());
                        }
                        if(priority[VT_hash[now]][VT_hash[it.RH[i+2]]]==UNDEFINED)
                            priority[VT_hash[now]][VT_hash[it.RH[i+2]]]=EQUAL;
                    }
                }
            }else{
                // 非终结符
                if(G.VT.find(nxt)==G.VT.end()){
                    // 下一个也是非终结符
                    // 说明不是算符文法 报错
                    stringstream ss;
                    ss<<"Invalid Grammer: a VN is next to another VN!";
                    throw string(ss.str());
                }
                // 下一个是终结符 LASTVT(a)>b
                for(int i=1;i<=G.VT.size();++i){
                    if(LastVT[VN_hash[now]][i]){
                        if(priority[i][VT_hash[nxt]]==SMALLER||priority[i][VT_hash[nxt]]==EQUAL){
                            // 可以找到超过一种优先级关系
                            // 说明不是算符优先文法 报错
                            stringstream ss;
                            ss<<"Invalid Grammer: more than one priority relation!";
                            throw string(ss.str());
                        }
                        if(priority[i][VT_hash[nxt]]==UNDEFINED)
                            priority[i][VT_hash[nxt]]=BIGGER;
                    }
                }
            }
        }
    }
}
void OperatorPecePrecedenceParser::find_reduction(){
    string temp;
    for(auto& it:G1.P){
        temp="";
        for(auto& subit:it.RH)
            temp+=subit;
        reduction[temp]=it.LH;
    }
}
void OperatorPecePrecedenceParser::init(){
    // 对非终结符进行排序并映射
    VN_order=vector<string>(G.VN.begin(), G.VN.end());
    VN_order.insert(VN_order.begin(), "");
    for(int i=1;i<VN_order.size();++i)
        VN_hash[VN_order[i]]=i;
    // 对终结符进行排序并映射
    VT_order=vector<string>(G.VT.begin(), G.VT.end());
    VT_order.insert(VT_order.begin(), "");
    for(int i=1;i<VT_order.size();++i)
        VT_hash[VT_order[i]]=i;
    find_FirstVT();
    find_LastVT();
    try{
        find_priority();
    }catch(string e){
        throw e;
    }
    find_reduction();
}
pair<node*,unsigned long long> OperatorPecePrecedenceParser::parse(const vector<token>& tokens,const unsigned long long& pos){
    stack<node*> st_node;  // 符号栈
    stack<string> st_VT;  // 终结符栈
    node *inToken=NULL,*temp=NULL;
    string op="",phrase="";
    pair<node*,int> ret={NULL,0};
    node* sharp=new node;
    sharp->self="#";
    sharp->children.clear();
    st_node.push(sharp);
    st_VT.push("#");
    for(unsigned long long i=pos;i<tokens.size();++i){
        inToken=new node;
        inToken->tokenIndex=i;  // 方便语义分析错误处理
        // 先识别终结符
        if(tokens[i].index==1){
            // 标识符
            inToken->self="ID";
            inToken->children.push_back(new node);
            inToken->children.back()->self=attr_table[tokens[i].attr].name;
            inToken->children.back()->tokenIndex=0;  // 给ID结点赋值就够了
            inToken->children.back()->children.clear();
        }
        if(tokens[i].index==2){
            // 整型常量
            inToken->self="NUM";
            inToken->children.clear();
            inToken->children.push_back(new node);
            inToken->children.back()->self=to_string(tokens[i].attr);
            inToken->children.back()->tokenIndex=0;  // 给NUM结点赋值就够了
            inToken->children.back()->children.clear();
        }
        if(tokens[i].index==3){
            // 布尔型常量
            inToken->self="BCONST";
            inToken->children.clear();
            inToken->children.push_back(new node);
            inToken->children.back()->self=tokens[i].attr?"true":"false";
            inToken->children.back()->tokenIndex=0;  // 给BCONST结点赋值就够了
            inToken->children.back()->children.clear();
        }
        if(tokens[i].index>3){
            if(type_table[tokens[i].index]==";"){
                // 分号作为结束符 替换为#
                inToken->self="#";
                inToken->children.clear();
                ret.second=i;  // 直接返回当前位置给上层函数
            }else{
                // 剩下的终结符只有负号需要特殊处理
                op=type_table[tokens[i].index];
                if(op=="-"){
                    // 区分负号
                    if(i==pos)
                        op="M";
                    else{
                        string last=type_table[tokens[i-1].index];
                        if(last=="+"||last=="-"||last=="*"||last=="/"||last=="(")
                            op="M";
                    }
                }
                inToken->self=op;
                inToken->children.clear();
            }
        }
        // 再按算符优先分析的逻辑处理
        // 栈顶终结符大于当前终结符 归约直到可以移进
        while(priority[VT_hash[st_VT.top()]][VT_hash[inToken->self]]==BIGGER){
            temp=new node;
            temp->tokenIndex=0;  // 非终结符
            phrase="";
            // 寻找最左素短语
            while(reduction.find(phrase)==reduction.end()){
                if(st_node.empty()){
                    // 说明没有可以归约项 报错
                    return {NULL,i};
                }
                temp->children.insert(temp->children.begin(),st_node.top());
                phrase=st_node.top()->self+phrase;
                if(st_node.top()->self==st_VT.top())
                    st_VT.pop();
                st_node.pop();
            }
            // 归约进栈
            temp->self=reduction[phrase];
            st_node.push(temp);
        }
        if(priority[VT_hash[st_VT.top()]][VT_hash[inToken->self]]==UNDEFINED){
            // 关系未定义 报错
            return {NULL,i};
        }
        // 此时满足小于或者等于
        // 如果 是#=# 分析成功 退出即可
        if(priority[VT_hash[st_VT.top()]][VT_hash[inToken->self]]==EQUAL&&inToken->self=="#"&&st_VT.top()=="#"){
            break;
        }
        // 其余情况直接移进
        st_node.push(inToken);
        st_VT.push(inToken->self);
    }
    if(st_VT.top()=="#"&&st_node.size()==2&&inToken->self=="#"){
        // 此时可以接受 不用归约 S' -> #S# 直接输出即可
        ret.first=st_node.top();
        st_node.pop();
        st_node.pop();
    }else{
        // 说明出现错误
        // stringstream ss;
        // ss<<"Syntax Error 2:"<<ret.second;
        // throw string(ss.str());
        return {NULL,ret.second};
    }
    return ret;
}
OperatorPecePrecedenceParser::OperatorPecePrecedenceParser(const string& fileName,const string& fileName1){
    G=read_BNF(fileName);
    G1=read_BNF(fileName1);
    try{
        init();
    }catch(string e){
        throw e;
    }
}
void OperatorPecePrecedenceParser::print_priority(){
    cout<<"Priority Relation Table:"<<endl;
    cout<<"\t";
    for(int i=1;i<=G.VT.size();++i)
        cout<<VT_order[i]<<"\t";
    cout<<endl;
    for(int i=1;i<=G.VT.size();++i){
        cout<<VT_order[i]<<"\t";
        for(int j=1;j<=G.VT.size();++j){
            if(priority[i][j]==UNDEFINED)
                cout<<"U\t";
            else if(priority[i][j]==BIGGER)
                cout<<">\t";
            else if(priority[i][j]==EQUAL)
                cout<<"=\t";
            else
                cout<<"<\t";
        }
        cout<<endl;
    }
}