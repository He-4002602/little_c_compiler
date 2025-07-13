#include "ll1parser.h"
using namespace std;
void LL1Parser::find_first(){
    First.clear();
    First.resize(G.VN.size()+1,vector<bool>(G.VT.size()+1,false));
    bool changed=true;
    int eid=VT_hash["EMPTY"];
    while(changed){
        changed=false;
        for(auto& it:G.P){
            for(int i=0;i<it.RH.size();++i){
                if(G.VN.find(it.RH[i])==G.VN.end()){
                    // 终结符 直接加入 包括EMPTY
                    if(!First[VN_hash[it.LH]][VT_hash[it.RH[0]]]){
                        First[VN_hash[it.LH]][VT_hash[it.RH[0]]]=true;
                        changed=true;
                    }
                    break;
                }
                // 非终结符
                if(!First[VN_hash[it.RH[i]]][eid]||i==it.RH.size()-1){
                    // 如果推导不出EMPTY 或者是最后一个 直接添加后退出
                    for(int j=1;j<=G.VT.size();++j){
                        if(First[VN_hash[it.RH[i]]][j]&&!First[VN_hash[it.LH]][j]){
                            First[VN_hash[it.LH]][j]=true;
                            changed=true;
                        }
                    }
                    break;
                }
                // 如果可以推导出EMPTY 且不是最后一个
                // 加入非空元素 继续向后找
                for(int j=1;j<=G.VT.size();++j){
                    if(j==eid)  // 跳过EMPTY
                        continue;
                    if(First[VN_hash[it.RH[i]]][j]&&!First[VN_hash[it.LH]][j]){
                        First[VN_hash[it.LH]][j]=true;
                        changed=true;
                    }
                }
            }
        }
    }
}
void LL1Parser::find_follow(){
    Follow.clear();
    Follow.resize(G.VN.size()+1,vector<bool>(G.VT.size()+1,false));
    bool changed=true;
    int eid=VT_hash["EMPTY"],sharp=VT_hash["#"];
    while(changed){
        changed=false;
        for(auto& it:G.P){
            if(it.LH==G.S){
                // 如果是开始符号 加入结束符
                if(!Follow[VN_hash[it.LH]][sharp]){
                    Follow[VN_hash[it.LH]][sharp]=true;
                    changed=true;
                }
            }
            for(int i=0;i<it.RH.size();++i){
                if(G.VN.find(it.RH[i])==G.VN.end()){
                    // 终结符 直接跳过
                    continue;
                }
                if(i!=it.RH.size()-1){
                    // 如果不是最后一个 把下一个的First中的非空元素加入
                    if(G.VN.find(it.RH[i+1])==G.VN.end()){
                        // 下一个是终结符 直接加入
                        if(!Follow[VN_hash[it.RH[i]]][VT_hash[it.RH[i+1]]]){
                            Follow[VN_hash[it.RH[i]]][VT_hash[it.RH[i+1]]]=true;
                            changed=true;
                        }
                    }else{
                        // 下一个是非终结符
                        for(int j=1;j<=G.VT.size();++j){
                            if(First[VN_hash[it.RH[i+1]]][j]&&!Follow[VN_hash[it.RH[i]]][j]&&j!=eid){
                                Follow[VN_hash[it.RH[i]]][j]=true;
                                changed=true;
                            }
                        }
                    }
                }
                if(i==it.RH.size()-1||(G.VN.find(it.RH[i+1])!=G.VN.end()&&First[VN_hash[it.RH[i+1]]][eid])){
                    // 如果是最后一个或者下一个是非终结符且可以推出EMPTY
                    // 把左部Follow加入
                    for(int j=1;j<=G.VT.size();++j){
                        if(Follow[VN_hash[it.LH]][j]&&!Follow[VN_hash[it.RH[i]]][j]){
                            Follow[VN_hash[it.RH[i]]][j]=true;
                            changed=true;
                        }
                    }
                }
            }
        }
    }
}
void LL1Parser::find_analyzingTable(){
    analyzingTable.clear();
    analyzingTable.resize(G.VN.size()+1,vector<int>(G.VT.size()+1,-1));
    int eid=VT_hash["EMPTY"];
    for(int i=0;i<G.P.size();++i){
        if(G.VN.find(G.P[i].RH[0])==G.VN.end()){
            // 终结符 自己就是First
            if(VT_hash[G.P[i].RH[0]]==eid){
                // 如果是EMPTY
                // 更新左部Follow的入口为产生式下标
                for(int j=1;j<=G.VT.size();++j){
                    if(Follow[VN_hash[G.P[i].LH]][j]&&j!=eid){
                        if(analyzingTable[VN_hash[G.P[i].LH]][j]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][j]!=i){
                            // 多重入口
                            stringstream ss;
                            ss<<"Invalid Grammer: Multi-entry in analyzingTable!"; 
                            throw string(ss.str());
                        }
                        if(analyzingTable[VN_hash[G.P[i].LH]][j]==-1)
                            analyzingTable[VN_hash[G.P[i].LH]][j]=i;
                    }
                }
                continue;
            }
            if(analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]!=i){
                // 多重入口
                stringstream ss;
                ss<<"Invalid Grammer: Multi-entry in analyzingTable!";
                throw string(ss.str());
            }
            if(analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]==-1)
                analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]=i;
            continue;
        }
        // 非终结符 更新每个First中的非空元素的入口为产生式下标
        for(int j=1;j<=G.VT.size();++j){
            if(First[VN_hash[G.P[i].RH[0]]][j]&&j!=eid){
                if(analyzingTable[VN_hash[G.P[i].LH]][j]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][j]!=i){
                    // 多重入口
                    stringstream ss;
                    ss<<"Invalid Grammer: Multi-entry in analyzingTable!";
                    throw string(ss.str());
                }
                if(analyzingTable[VN_hash[G.P[i].LH]][j]==-1)
                    analyzingTable[VN_hash[G.P[i].LH]][j]=i;
            }
        }
        if(First[VN_hash[G.P[i].RH[0]]][eid]){
            // 如果可以推出EMPTY
            // 更新左部Follow的入口为产生式下标
            for(int j=1;j<=G.VT.size();++j){
                if(Follow[VN_hash[G.P[i].LH]][j]&&j!=eid){
                    if(analyzingTable[VN_hash[G.P[i].LH]][j]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][j]!=i){
                        // 多重入口
                        stringstream ss;
                        ss<<"Invalid Grammer: Multi-entry in analyzingTable!";
                        throw string(ss.str());
                    }
                    if(analyzingTable[VN_hash[G.P[i].LH]][j]==-1)
                        analyzingTable[VN_hash[G.P[i].LH]][j]=i;
                }
            }
        }
    }
}

void LL1Parser::init(){
    G.VT.insert("#");  // 添加结束符
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
    find_first();
    find_follow();
    try{
        find_analyzingTable();  
    }catch(string& e){
        throw e;
    }
}
LL1Parser::LL1Parser(const string& fileName){
    G=read_BNF(fileName);
}

pair<node*,unsigned long long> LL1Parser::parse(const vector<token>& tokens, const unsigned long long& pos){}
void LL1Parser::error_handle(const vector<token>& tokens,stack<node*>& st_node,unsigned long long *pos){}
void DeclarationParser::error_handle(const vector<token>& tokens,stack<node*>& st_node,unsigned long long *pos){
    // 声明语句错误处理直接找到下一个分号
    for(;*pos<tokens.size();++(*pos)){
        if(type_table[tokens[*pos].index]==";"){
            while(st_node.top()->self!="#")
                st_node.pop();
            st_node.push(new node);
            st_node.top()->self="DECL";
            st_node.top()->children.clear();
            parserErrorList.push_back(*pos);
            return;
        }
    }
    --(*pos);
    parserErrorList.push_back(*pos);
}
pair<node*,unsigned long long> DeclarationParser::parse(const vector<token>& tokens, const unsigned long long& pos){
    stack<node*> st_node;  // 符号栈
    node *inToken=NULL,*temp=NULL;
    node* sharp=new node;
    node* start=new node;
    pair<node*,int> ret={NULL,0};
    sharp->self="#";
    sharp->children.clear();
    start->self=G.S;
    start->children.clear();
    st_node.push(sharp);
    st_node.push(start);
    for(unsigned long long i=pos;i<tokens.size();++i){  
        inToken=new node;
        // 先识别终结符
        if(tokens[i].index==1){
            if(i!=0
                &&type_table[tokens[i-1].index]!="int"
                &&type_table[tokens[i-1].index]!="bool"
                &&type_table[tokens[i-1].index]!=","){
                // 单独判断一下是不是结束符
                // 这样的id交给执行语句处理
                inToken->self="#";
                inToken->children.clear();
                ret.second=i;  // 直接返回当前位置给上层函数
            }else{
                // 标识符
                inToken->self="ID";
                inToken->children.push_back(new node);
                inToken->children.back()->self=attr_table[tokens[i].attr].name;
                inToken->children.back()->tokenIndex=0;  // 给ID结点赋值就够了
                inToken->children.back()->children.clear();
            }
        }else{
            // 单独判断一下是不是结束符
            if(G.VT.find(type_table[tokens[i].index])==G.VT.end()){
                // 不在终结符集合中 就可以认为是一个结束符
                // 这里主要是观察文法发现除了id以外出现过的终结符都不在Follow中
                inToken->self="#";
                inToken->children.clear();
                ret.second=i;  // 直接返回当前位置给上层函数
            }else{
                // 其余情况正常处理即可
                inToken->self=type_table[tokens[i].index];
                inToken->children.clear();
            }
        }
        // 再按LL1分析的逻辑处理
        
        while(G.VT.find(st_node.top()->self)==G.VT.end()){
            // 栈顶不是终结符 一直按照分析表匹配处理
            if(analyzingTable[VN_hash[st_node.top()->self]][VT_hash[inToken->self]]==-1){
                // 出错
                error_handle(tokens,st_node,&i);
                if(i==tokens.size()-1){
                    return {NULL,i};
                }
                ++i;
                if(type_table[tokens[i].index]!="int"&&type_table[tokens[i].index]!="bool"){
                    return {NULL,i};
                }
                inToken->self=type_table[tokens[i].index];
                inToken->children.clear();
                continue;
            }else{
                // 匹配成功
                int index=analyzingTable[VN_hash[st_node.top()->self]][VT_hash[inToken->self]];
                if(G.P[index].RH[0]=="EMPTY"&&G.P[index].RH.size()==1){
                    // 如果是空产生式 直接出栈
                    st_node.pop();
                }else{
                    temp=st_node.top();
                    temp->tokenIndex=0; // 非终结符
                    st_node.pop();
                    for(int j=G.P[index].RH.size()-1;j>=0;--j){
                        // 逆序入栈 并置空子结点
                        st_node.push(new node);
                        st_node.top()->self=G.P[index].RH[j];
                        st_node.top()->children.clear();
                        temp->children.insert(temp->children.begin(),st_node.top());
                    }
                }
            }
        }
        // 栈顶是终结符 直接匹配出栈即可
        // 栈顶和指针都为结束符 分析结束
        if(st_node.top()->self=="#"&&inToken->self=="#"){
            // 匹配完成
            st_node.pop();
            break;
        }

        // 其他情况
        if(st_node.top()->self!=inToken->self){
            error_handle(tokens,st_node,&i);
            if(i==tokens.size()-1){
                return {NULL,i};
            }
            if(type_table[tokens[i+1].index]!="int"&&type_table[tokens[i+1].index]!="bool"){
                return {NULL,i+1};
            }
            // inToken->self=type_table[tokens[i+1].index];
            // inToken->children.clear();
            continue;
        }else{
            // 匹配成功
            // 单独处理一下标识符
            if(st_node.top()->self=="ID"){
                // 这里需要把标识符的值传递给父节点
                st_node.top()->children.push_back(inToken->children[0]);
            }
            st_node.top()->tokenIndex=i;  // 方便语义分析错误处理
            st_node.pop();
        }
    }
    ret.first=start;
    return ret;
}


void StatementParser::find_analyzingTable(){
    analyzingTable.clear();
    analyzingTable.resize(G.VN.size()+1,vector<int>(G.VT.size()+1,-1));
    int eid=VT_hash["EMPTY"];
    for(int i=0;i<G.P.size();++i){
        if(G.VN.find(G.P[i].RH[0])==G.VN.end()){
            // 终结符 自己就是First
            if(VT_hash[G.P[i].RH[0]]==eid){
                // 如果是EMPTY
                // 更新左部Follow的入口为产生式下标
                for(int j=1;j<=G.VT.size();++j){
                    if(G.P[i].LH=="ELSE"&&VT_order[j]=="else")  // 特判if-else
                        continue;
                    if(Follow[VN_hash[G.P[i].LH]][j]&&j!=eid){
                        if(analyzingTable[VN_hash[G.P[i].LH]][j]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][j]!=i){
                            // 多重入口
                            stringstream ss;
                            ss<<"Invalid Grammer: Multi-entry in analyzingTable!"; 
                            throw string(ss.str());
                        }
                        if(analyzingTable[VN_hash[G.P[i].LH]][j]==-1)
                            analyzingTable[VN_hash[G.P[i].LH]][j]=i;
                    }
                }
                continue;
            }
            if(analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]!=i){
                // 多重入口
                stringstream ss;
                ss<<"Invalid Grammer: Multi-entry in analyzingTable!";
                throw string(ss.str());
            }
            if(analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]==-1)
                analyzingTable[VN_hash[G.P[i].LH]][VT_hash[G.P[i].RH[0]]]=i;
            continue;
        }
        // 非终结符 更新每个First中的非空元素的入口为产生式下标
        for(int j=1;j<=G.VT.size();++j){
            if(First[VN_hash[G.P[i].RH[0]]][j]&&j!=eid){
                if(analyzingTable[VN_hash[G.P[i].LH]][j]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][j]!=i){
                    // 多重入口
                    stringstream ss;
                    ss<<"Invalid Grammer: Multi-entry in analyzingTable!";
                    throw string(ss.str());
                }
                if(analyzingTable[VN_hash[G.P[i].LH]][j]==-1)
                    analyzingTable[VN_hash[G.P[i].LH]][j]=i;
            }
        }
        if(First[VN_hash[G.P[i].RH[0]]][eid]){
            // 如果可以推出EMPTY
            // 更新左部Follow的入口为产生式下标
            for(int j=1;j<=G.VT.size();++j){
                if(Follow[VN_hash[G.P[i].LH]][j]&&j!=eid){
                    if(analyzingTable[VN_hash[G.P[i].LH]][j]!=-1&&analyzingTable[VN_hash[G.P[i].LH]][j]!=i){
                        // 多重入口
                        stringstream ss;
                        ss<<"Invalid Grammer: Multi-entry in analyzingTable!";
                        throw string(ss.str());
                    }
                    if(analyzingTable[VN_hash[G.P[i].LH]][j]==-1)
                        analyzingTable[VN_hash[G.P[i].LH]][j]=i;
                }
            }
        }
    }
}
void StatementParser::error_handle(const vector<token>& tokens,stack<node*>& st_node,unsigned long long* pos){
    // 语法错误处理函数
    for(;*pos<tokens.size();++(*pos)){
        // 对每个符号判断其是否在STMT的First中 从该符号继续向下推导
        for(int i=1;i<=G.VT.size();++i){
            if(First[VN_hash["STMT"]][i]){
                // 如果在First中 清空栈返回
                if(type_table[tokens[*pos].index]==VT_order[i]){
                    while(st_node.top()->self!="#")
                        st_node.pop();
                    st_node.push(new node);
                    st_node.top()->self="STMT";
                    st_node.top()->children.clear();
                    parserErrorList.push_back(*pos);
                    return;
                }
            }
        }
    }
    // 如果到达了文件末尾部
    (*pos)--;
    parserErrorList.push_back(*pos);
}
pair<node*,unsigned long long> StatementParser::parse(const vector<token>& tokens, const unsigned long long& pos){
    stack<node*> st_node;  // 符号栈
    node *inToken=NULL,*temp=NULL;
    node* sharp=new node;
    node* start=new node;
    pair<node*,int> ret={NULL,0},optemp{NULL,0};
    sharp->self="#";
    sharp->children.clear();
    start->self=G.S;
    start->children.clear();
    st_node.push(sharp);
    st_node.push(start);
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
        }else{
            // 单独判断一下是不是结束符
            if(type_table[tokens[i].index]=="}"&&i==tokens.size()-1){
                inToken->self="#";
                inToken->children.clear();
                ret.second=i;  // 直接返回当前位置给上层函数
            }else{
                // 其余情况正常处理即可
                inToken->self=type_table[tokens[i].index];
                inToken->children.clear();
            }
        }
        // 再按LL1分析的逻辑处理
        while(G.VT.find(st_node.top()->self)==G.VT.end()){
            // EXPR 和 BOOL 调用算符优先分析
            if(st_node.top()->self=="EXPR"||st_node.top()->self=="BOOL"){
                if(st_node.top()->self=="EXPR"){
                    optemp=ExprParser->parse(tokens,i);
                }else{
                    optemp=BoolParser->parse(tokens,i);
                }
                i=optemp.second;
                if(optemp.first==NULL){
                    // 语法错误 调用错误处理函数
                    error_handle(tokenList,st_node,&i);
                    if(i==tokens.size()-1){
                        return {NULL,i};
                    }
                    if(tokens[i].index==1){
                        // 标识符
                        inToken->self="ID";
                        inToken->children.push_back(new node);
                        inToken->children.back()->self=attr_table[tokens[i].attr].name;
                        inToken->children.back()->children.clear();
                    }else{
                        inToken->self=type_table[tokens[i].index];
                        inToken->children.clear();
                    }
                    continue;
                }
                st_node.top()->children.clear();
                for(auto& it:optemp.first->children)
                    st_node.top()->children.push_back(it);
                st_node.pop();
                // 更新一下inToken
                inToken->self="";
                inToken->children.clear();
                if(tokens[i].index==1){
                    // 标识符
                    inToken->self="ID";
                    inToken->children.push_back(new node);
                    inToken->children.back()->self=attr_table[tokens[i].attr].name;
                    inToken->children.back()->children.clear();
                }else{
                    // 单独判断一下是不是结束符
                    if(type_table[tokens[i].index]=="}"&&i==tokens.size()-1){
                        inToken->self="#";
                        inToken->children.clear();
                        ret.second=i;  // 直接返回当前位置给上层函数
                    }else{
                        // 其余情况正常处理即可
                        inToken->self=type_table[tokens[i].index];
                        inToken->children.clear();
                    }
                }
                continue;
            }
            // 其他情况正常做LL1分析
            // 栈顶不是终结符 一直按照分析表匹配处理
            if(analyzingTable[VN_hash[st_node.top()->self]][VT_hash[inToken->self]]==-1){
                // 语法错误 调用错误处理函数
                    i=optemp.second;
                    // cout<<type_table[tokens[i].index]<<endl;
                    error_handle(tokenList,st_node,&i);
                    if(i==tokens.size()-1){
                        return {NULL,i};
                    }
                    // inToken->self=type_table[tokens[i].index];
                    // inToken->children.clear();
                    continue;
            }else{
                // 匹配成功
                int index=analyzingTable[VN_hash[st_node.top()->self]][VT_hash[inToken->self]];
                if(G.P[index].RH[0]=="EMPTY"&&G.P[index].RH.size()==1){
                    // 如果是空产生式 直接出栈
                    st_node.pop();
                }else{
                    temp=st_node.top();
                    temp->tokenIndex=0; // 非终结符
                    st_node.pop();
                    for(int j=G.P[index].RH.size()-1;j>=0;--j){
                        // 逆序入栈 并置空子结点
                        st_node.push(new node);
                        st_node.top()->self=G.P[index].RH[j];
                        st_node.top()->children.clear();
                        temp->children.insert(temp->children.begin(),st_node.top());
                    }
                }
            }
        }
        // 栈顶是终结符 直接匹配出栈即可
        // 栈顶和指针都为结束符 分析结束
        if(st_node.top()->self=="#"&&inToken->self=="#"){
            // 匹配完成
            st_node.pop();
            break;
        }

        // 其他情况
        if(st_node.top()->self!=inToken->self){
            // 语法错误 调用错误处理函数
            error_handle(tokenList,st_node,&i);
            if(i==tokens.size()-1){
                return {NULL,i};
            }
            --i;
            continue;
        }else{
            // 匹配成功
            // 单独处理一下标识符
            if(st_node.top()->self=="ID"){
                // 这里需要把标识符的值传递给父节点
                st_node.top()->children.push_back(inToken->children[0]);
            }
            st_node.top()->tokenIndex=i;  // 方便语义分析错误处理
            st_node.pop();
        }
    }
    // // 遍历语法树 删除没有子结点的非终结符结点
    // deleteEmpty(start);
    ret.first=start;
    return ret;
}
// 调试用
void LL1Parser::print_First(){
    for(int i=1;i<=G.VN.size();++i){
        cout<<VN_order[i]<<": {";
        for(int j=1;j<=G.VT.size();++j){
            if(First[i][j]){
                cout<<VT_order[j]<<" ";
            }
        }
        cout<<"}"<<endl;
    }
}
void LL1Parser::print_Follow(){
    for(int i=1;i<=G.VN.size();++i){
        cout<<VN_order[i]<<": {";
        for(int j=1;j<=G.VT.size();++j){
            if(Follow[i][j]){
                cout<<VT_order[j]<<" ";
            }
        }
        cout<<"}"<<endl;
    }
}
void LL1Parser::print_analyzingTable(){
    cout<<"Productions:"<<endl;
    for(int i=0;i<G.P.size();++i){
        cout<<i<<": "<<G.P[i].LH<<"-> ";
        for(int j=0;j<G.P[i].RH.size();++j){
            cout<<G.P[i].RH[j]<<" ";
        }cout<<endl;
    }
    cout<<"Analyzing Table:"<<endl;
    cout<<"\t";
    for(int i=1;i<=G.VT.size();++i){
        cout<<VT_order[i]<<"\t";
    }
    cout<<endl;
    for(int i=1;i<=G.VN.size();++i){
        cout<<VN_order[i]<<"\t";
        for(int j=1;j<=G.VT.size();++j){
            cout<<analyzingTable[i][j]<<"\t";
        }cout<<endl;
    }
}   
