#include "semantic.h"
using namespace std;
vector<string> semanticErrorList;
vector<string> irCodeList;
int varCount=2;
unordered_map<string, string> idnameInIr; 
block* curIrBlock;  // 当前生成代码的基本块
vector<string> irHead = {
    "; ModuleID = 'syscall_io.ll'",
    "target triple = \"x86_64-pc-windows-msvc\"",
    "",
    "@.num_buffer = internal global [12 x i8] zeroinitializer",
    "@.input_buffer = internal global [12 x i8] zeroinitializer",
    "",
    "; 声明 Windows API",
    "declare i8* @GetStdHandle(i32)",
    "declare i1 @WriteConsoleA(i8*, i8*, i32, i32*, i8*)",
    "declare i1 @ReadConsoleA(i8*, i8*, i32, i32*, i8*)",
    "",
    "define void @write(i32 %id) {",
    "entry:",
    "  %buf = getelementptr [12 x i8], [12 x i8]* @.num_buffer, i32 0, i32 11",
    "  store i8 0, i8* %buf    ; null终止",
    "",
    "  %id_val = alloca i32",
    "  store i32 %id, i32* %id_val",
    "",
    "  %ptr = alloca i8*",
    "  store i8* %buf, i8** %ptr",
    "",
    "  br label %loop",
    "",
    "loop:",
    "  %curr_ptr = load i8*, i8** %ptr",
    "  %id_now = load i32, i32* %id_val",
    "",
    "  %rem = srem i32 %id_now, 10",
    "  %div = sdiv i32 %id_now, 10",
    "",
    "  %rem_char = add i32 %rem, 48",
    "  %rem_c = trunc i32 %rem_char to i8",
    "",
    "  %new_ptr = getelementptr i8, i8* %curr_ptr, i32 -1",
    "  store i8 %rem_c, i8* %new_ptr",
    "  store i8* %new_ptr, i8** %ptr",
    "",
    "  store i32 %div, i32* %id_val",
    "",
    "  %check = icmp eq i32 %div, 0",
    "  br i1 %check, label %print, label %loop",
    "",
    "print:",
    "  %start_ptr = load i8*, i8** %ptr",
    "  %h_out = call i8* @GetStdHandle(i32 -11) ; -11 == STD_OUTPUT_HANDLE",
    "  %len = ptrtoint i8* %buf to i64",
    "  %start = ptrtoint i8* %start_ptr to i64",
    "  %size = sub i64 %len, %start",
    "  %size32 = trunc i64 %size to i32",
    "",
    "  %written = alloca i32",
    "  call i1 @WriteConsoleA(i8* %h_out, i8* %start_ptr, i32 %size32, i32* %written, i8* null)",
    "",
    "  ; ==== 新增一段，专门输出换行符 ====",
    "  %newline = alloca [2 x i8]",
    "  %newline_ptr = getelementptr [2 x i8], [2 x i8]* %newline, i32 0, i32 0",
    "  store i8 10, i8* %newline_ptr        ; \'\\n\' (ASCII 10)",
    "  %newline_null = getelementptr i8, i8* %newline_ptr, i32 1",
    "  store i8 0, i8* %newline_null        ; null终止",
    "",
    "  call i1 @WriteConsoleA(i8* %h_out, i8* %newline_ptr, i32 1, i32* %written, i8* null)",
    "",
    "  ret void",
    "}",
    "",
    "define i32 @read() {",
    "entry:",
    "  %h_in = call i8* @GetStdHandle(i32 -10) ; -10 == STD_INPUT_HANDLE",
    "  %written = alloca i32",
    "",
    "  %buf = getelementptr [12 x i8], [12 x i8]* @.input_buffer, i32 0, i32 0",
    "  call i1 @ReadConsoleA(i8* %h_in, i8* %buf, i32 12, i32* %written, i8* null)",
    "",
    "  ; 解析字符串成数字",
    "  %result = alloca i32",
    "  store i32 0, i32* %result",
    "",
    "  %i = alloca i32",
    "  store i32 0, i32* %i",
    "",
    "  br label %loop",
    "",
    "loop:",
    "  %idx = load i32, i32* %i",
    "  %ch_ptr = getelementptr i8, i8* %buf, i32 %idx",
    "  %ch = load i8, i8* %ch_ptr",
    "  %ch_ext = sext i8 %ch to i32",
    "",
    "  ; 是否 \\r 或 \\n",
    "  %is_cr = icmp eq i32 %ch_ext, 13",
    "  %is_lf = icmp eq i32 %ch_ext, 10",
    "  %is_end = or i1 %is_cr, %is_lf",
    "  br i1 %is_end, label %done, label %cont",
    "",
    "cont:",
    "  ; ch - '0'",
    "  %digit = sub i32 %ch_ext, 48",
    "  %prev = load i32, i32* %result",
    "  %mul10 = mul i32 %prev, 10",
    "  %add = add i32 %mul10, %digit",
    "  store i32 %add, i32* %result",
    "",
    "  %next_idx = add i32 %idx, 1",
    "  store i32 %next_idx, i32* %i",
    "  br label %loop",
    "",
    "done:",
    "  %final = load i32, i32* %result",
    "  ret i32 %final",
    "}"
};
vector<string> mainHead = {
    "define dso_local noundef i32 @main() {",
    "  %1 = alloca i32, align 4",
    "  store i32 0, ptr %1, align 4"
};
vector<string> mainTail = {
    "  ret i32 0",
    "}"
};
bool type_check(node* root,int type){
    // 返回检查是否通过
    // 下传继承属性type
    // 声明语句处理
    if(root->self=="DECL"){
        // NAMES.type=children[0].type;
        root->children[0]->self=="int"
        ?type_check(root->children[1],INTEGER)
        :type_check(root->children[1],BOOL);
        return 1;
    }
    if(root->self=="NAMES"||root->self=="NAMES'"){
        // 对于NAMES' deletEmpty应该已经确保了子节点不为空 且删除了','
        for(int i=0;i<root->children.size();i++){
            // 向下传递type
            type_check(root->children[i],type);
        }
        return 1;
    }
    if(root->self=="NAME"){
        string idname=root->children[0]->children[0]->self;
        // CHECK(ID.type)
        if(attr_table[attr_hash_table[idname]].type!=0){
            // 重复声明变量
            stringstream ss;
            ss<<idname<<" redeclared "
            <<"("<<tokenList[root->children[0]->tokenIndex].row
            <<": "<<tokenList[root->children[0]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        attr_table[attr_hash_table[idname]].type=type;  // ID.type=NAMES.type
        return 1;
    }
    // 执行语句处理
    if(root->self=="STMT"&&root->children[0]->self=="ID"){
        // 赋值语句
        string idname=root->children[0]->children[0]->self;
        int idType=attr_table[attr_hash_table[idname]].type;
        // CHECK(ID.type)
        if(idType==0){
            // 未声明变量
            stringstream ss;
            ss<<idname<<" undeclared "
            <<"("<<tokenList[root->children[0]->tokenIndex].row
            <<": "<<tokenList[root->children[0]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        bool valFlag=type_check(root->children[1],idType);  // ASSIGN.type=ID.type
        attr_table[attr_hash_table[idname]].isValued=valFlag;  // ID.isValued=ASSIGN.isValued
        if(!valFlag){
            // 赋值语句不合法
            stringstream ss;
            ss<<"Invalid assignment to "<<idname
            <<" ("<<(idType==INTEGER?"int":"bool")<<") "
            <<"("<<tokenList[root->children[0]->tokenIndex].row
            <<": "<<tokenList[root->children[0]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        return 1;   
    }
    if(root->self=="ASSIGN"){
        // 赋值语句
        string op=root->children[0]->self;
        if(type==INTEGER&&op==":="
            || type==BOOL&&op=="="){
            // 赋值符号与类型不匹配
            return 0;  // ASSIGN.isValued=false
        }
        bool ret=true;
        for(int i=1;i<root->children.size();i++){
            // 文法区分了右值
            // 因此语义检查无需判断右值是否符合类型
            if(!type_check(root->children[i],type)){
                // 赋值语句不合法
                ret=false;  // ASSIGN.isValued=false
            }
        }
        return ret;  // ASSIGN.isValued=true
    }
    if(root->self=="STMT"
        &&(root->children[0]->self=="if"
        || root->children[0]->self=="while")){
        // if while语句
        string idname=root->children[1]->children[0]->self;
        int idType=attr_table[attr_hash_table[idname]].type;
        // CHECK(ID.type)
        if(idType==0){
            // 未声明变量
            stringstream ss;
            ss<<idname<<" undeclared "
            <<"("<<tokenList[root->children[1]->tokenIndex].row
            <<": "<<tokenList[root->children[1]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        if(idType==INTEGER){
            // 不合法类型
            stringstream ss;
            ss<<"An integer variable"
            <<" ("<<idname<<") "
            <<"is given after "
            <<root->children[0]->self
            <<", while a boolean variable is expected "
            <<"("<<tokenList[root->children[1]->tokenIndex].row
            <<": "<<tokenList[root->children[1]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        // CHECK(ID.isValued)
        if(!attr_table[attr_hash_table[idname]].isValued){
            // 未赋值变量
            stringstream ss;
            ss<<idname<<" uninitialized "
            <<"("<<tokenList[root->children[1]->tokenIndex].row
            <<": "<<tokenList[root->children[1]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        bool ret=true;
        for(int i=2;i<root->children.size();i++){
            if(!type_check(root->children[i],type)){
                ret=false;
            }
        }
        return ret;
    }
    if(root->self=="STMT"
        &&(root->children[0]->self=="read"
        || root->children[0]->self=="write")){
        // read write语句
        string idname=root->children[1]->children[0]->self;
        int idType=attr_table[attr_hash_table[idname]].type;
        // CHECK(ID.type)
        if(idType==0){
            // 未声明变量
            stringstream ss;
            ss<<idname<<" undeclared "
            <<"("<<tokenList[root->children[1]->tokenIndex].row
            <<": "<<tokenList[root->children[1]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        if(idType==BOOL){
            // 不合法类型
            stringstream ss;
            ss<<"A boolean variable"
            <<" ("<<idname<<") "
            <<"is given after "
            <<root->children[0]->self
            <<", while an integer variable is expected "
            <<"("<<tokenList[root->children[1]->tokenIndex].row
            <<": "<<tokenList[root->children[1]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        // CHECK(ID.isValued)
        if(root->children[0]->self=="write"
            &&!attr_table[attr_hash_table[idname]].isValued){
            // 未赋值变量
            stringstream ss;
            ss<<idname<<" uninitialized "
            <<"("<<tokenList[root->children[1]->tokenIndex].row
            <<": "<<tokenList[root->children[1]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        // ID.isValued=true
        if(root->children[0]->self=="read"){
            // read语句
            attr_table[attr_hash_table[idname]].isValued=true;
        }
        // 后续没有需要检查的子树 直接返回
        return 1;
    }
    // 算术表达式处理
    if(root->self=="EXPR"
        &&root->children[0]->self=="ID"){
        string idname=root->children[0]->children[0]->self;
        int idType=attr_table[attr_hash_table[idname]].type;
        // CHECK(ID.type)
        if(idType==0){
            // 未声明变量
            stringstream ss;
            ss<<idname<<" undeclared "
            <<"("<<tokenList[root->children[0]->tokenIndex].row
            <<": "<<tokenList[root->children[0]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        if(idType==BOOL){
            // 不合法类型
            stringstream ss;
            ss<<"A boolean variable"
            <<" ("<<idname<<") "
            <<"is given in an arithmetic expression, "
            <<"while an integer variable is expected "
            <<"("<<tokenList[root->children[0]->tokenIndex].row
            <<": "<<tokenList[root->children[0]->tokenIndex].col<<")";
            return 0;
        }
        // CHECK(ID.isValued)
        if(!attr_table[attr_hash_table[idname]].isValued){
            // 未赋值变量
            stringstream ss;
            ss<<idname<<" uninitialized "
            <<"("<<tokenList[root->children[0]->tokenIndex].row
            <<": "<<tokenList[root->children[0]->tokenIndex].col<<")";
            semanticErrorList.push_back(ss.str());
            return 0;
        }
        // 后续没有需要检查的子树 直接返回
        return 1;
    }
    // 其余情况依次遍历子树
    bool ret=true;
    for(int i=0;i<root->children.size();i++){
        if(!type_check(root->children[i],type)){
            // 类型检查不通过
            ret=false;
        }
    }
    return ret;
}

void generate_ir(node* root);
void generate_decl(node* root){
    string idname=root->children[0]->children[0]->self;
    int idType=attr_table[attr_hash_table[idname]].type;
    string dtype = (idType==INTEGER?"i32":"i8");
    string width = (idType==INTEGER?"4":"1");
    string vname = "%"+to_string(varCount++);
    idnameInIr[idname]=vname;
    stringstream ss;
    ss<<vname<<" = alloca "<<dtype<<", align "
    <<width;
    irCodeList.push_back(ss.str());
    return ;
}

struct expr_ret{
    string idname;  // 空返回数 非空返回变量名
    int value;  // 常量值
};

expr_ret generate_expr(node* root){
    if(root->children.size()==1
        &&root->children[0]->self=="EXPR"){
        // 删掉了多余括号的情况
        // 直接返回子树
        return generate_expr(root->children[0]);
    }
    if(root->children[0]->self=="NUM"){
        // 常量
        return {"", stoi(root->children[0]->children[0]->self)};
    }
    if(root->children[0]->self=="ID"){
        // 变量
        // 需要在这里load 确保ptr类型不会混淆
        // %x = load i32, ptr %id, align 4
        string idname=root->children[0]->children[0]->self;
        string vname="%"+to_string(varCount++);
        stringstream ss;
        ss<<vname<<" = load i32, ptr "
        <<idnameInIr[idname]<<", align 4";
        irCodeList.push_back(ss.str());
        // 返回变量名
        return {vname, 0};
    }
    if(root->children[0]->self=="M"){
        // 负号 转换为0-right
        root->children[0]->self="-";
        node *exprNode=new node,*numNode=new node,*zeroNode=new node;
        root->children.insert(root->children.begin(),exprNode);
        exprNode->self="EXPR";
        exprNode->children.clear();
        exprNode->children.push_back(numNode);
        numNode->self="NUM";
        numNode->children.clear();
        numNode->children.push_back(zeroNode);
        zeroNode->self="0";
        zeroNode->children.clear();
    }
    // 遍历左子树
    expr_ret lret=generate_expr(root->children[0]);
    // 遍历右子树
    expr_ret rret=generate_expr(root->children[2]);
    if(lret.idname==""
        &&rret.idname==""){
        // 常量表达式
        if(root->children[1]->self=="+")
            return {"", lret.value+rret.value};
        if(root->children[1]->self=="-")
            return {"", lret.value-rret.value};
        if(root->children[1]->self=="*")
            return {"", lret.value*rret.value};
        if(root->children[1]->self=="/")
            return {"", lret.value/rret.value};
        // 应该没有其他结果 这里先报个错以防万一
        cout<<"Error detected in exprGen"<<endl;
        exit(1);
    }
    // 剩下的情况包含变量
    // 依次检查是否是变量并处理
    string lval, rval, res;  // 左值 右值 结果值
    lval = lret.idname==""
            ?to_string(lret.value)
            :lret.idname;
    rval = rret.idname==""
            ?to_string(rret.value)
            :rret.idname;
    // 结果赋值给新变量
    // %x = add/sub/mul/sdiv i32 left, right
    res="%"+to_string(varCount++);
    stringstream ss;
    ss<<res<<" = ";
    switch(root->children[1]->self[0]){
        case '+':
            ss<<"add i32 "<<lval<<", "<<rval;
            break;
        case '-':
            ss<<"sub i32 "<<lval<<", "<<rval;
            break;
        case '*':
            ss<<"mul i32 "<<lval<<", "<<rval;
            break;
        case '/':
            ss<<"sdiv i32 "<<lval<<", "<<rval;
            break;
        default:
            // 应该没有其他结果 这里先报个错以防万一
            cout<<"Error detected in exprGen"<<endl;
            exit(1);
    }
    irCodeList.push_back(ss.str());
    return {res, 0};
}

struct bool_ret{
    string idname;  // 空返回数 非空返回变量名
    bool value;  // 常量值
    block* curBlock;  // 当前基本块
};
unordered_map<string,string> boolOpMap={
    {"==", "eq"},
    {"!=", "ne"},
    {"<", "slt"},
    {">", "sgt"},
    {"<=", "sle"},
    {">=", "sge"}
};
bool_ret generate_bool(node* root,block* curBlock);
bool_ret generate_not(node* root,block* curBlock){
    // 先判断子树的值能否确定
    bool_ret ret=generate_bool(root->children[1],curBlock);
    if(ret.idname==""){
        // 常量
        return {"", !ret.value, curBlock};
    }
    // 子树返回了变量
    if(ret.curBlock!=curBlock){
        // 更新控制流
        curBlock=ret.curBlock;
    }
    // 生成语句
    // %x = xor i1 %id, true
    string vname="%"+to_string(varCount++);
    stringstream ss;
    ss<<vname<<" = xor i1 "<<ret.idname<<", true";
    irCodeList.push_back(ss.str());
    // 返回结果
    return {vname, 0, curBlock};  // 返回当前基本块
}
bool_ret generate_or(node* root,block* curBlock){
    // 先判断左子树的值能否确定
    bool_ret lret=generate_bool(root->children[0],curBlock);
    if(lret.idname==""&&lret.value==true){
        // 左子树为常量true
        // 无条件true
        return {"", true, curBlock};
    }
    if(lret.idname==""){
        // 左子树为常量false
        // 返回右子树结果
        return generate_bool(root->children[2],curBlock);
    }
    // 左子树返回了变量
    if(lret.curBlock!=curBlock){
        // 更新控制流
        curBlock=lret.curBlock;
    }
    // 创建新的基本块分支
    block *trueBlock=new block,*falseBlock=new block;
    // 添加前驱
    trueBlock->preds.push_back(curBlock);
    falseBlock->preds.push_back(curBlock);
    // 添加后继
    curBlock->succs.push_back(trueBlock);
    curBlock->succs.push_back(falseBlock);
    // 为falseBlock分配标签 和变量共享标号
    falseBlock->label=to_string(varCount++);
    // 生成条件跳转
    stringstream ss;
    ss<<"br i1 "<<lret.idname<<", label %";
    // trueBlock的标签待定 更新curBlock的patchloc
    curBlock->patchloc={irCodeList.size(),ss.str().size()};
    ss<<", label %"<<falseBlock->label;
    irCodeList.push_back(ss.str());
    // 遍历右子树
    irCodeList.push_back(falseBlock->label+":");
    bool_ret rret=generate_bool(root->children[2],falseBlock);
    rret.curBlock->succs.push_back(trueBlock);
    trueBlock->preds.push_back(rret.curBlock);
    trueBlock->label=to_string(varCount++);
    // 回填
    irCodeList[curBlock->patchloc.first].insert(
        curBlock->patchloc.second,
        trueBlock->label);
    // 跳转到trueBlock
    irCodeList.push_back("br label %"+trueBlock->label);
    irCodeList.push_back(trueBlock->label+":");
    // 返回结果
    string res="%"+to_string(varCount++);
    ss.str("");
    ss.clear();
    ss<<res<<" = phi i1 [ true, %"
    <<curBlock->label<<" ], [ "
    <<(rret.idname==""?(rret.value?"true":"false")
        :rret.idname)
    <<", %"<<rret.curBlock->label<<" ]";
    irCodeList.push_back(ss.str());
    return {res, 0, trueBlock};  // 返回trueBlock
}
bool_ret generate_and(node* root,block* curBlock){
    // 先判断左子树的值能否确定
    bool_ret lret=generate_bool(root->children[0],curBlock);
    if(lret.idname==""&&lret.value==false){
        // 左子树为常量false
        // 无条件false
        return {"", false, curBlock};
    }
    if(lret.idname==""){
        // 左子树为常量true
        // 返回右子树结果
        return generate_bool(root->children[2],curBlock);
    }
    // 左子树返回了变量
    if(lret.curBlock!=curBlock){
        // 更新控制流
        curBlock=lret.curBlock;
    }
    // 创建新的基本块分支
    block *trueBlock=new block,*falseBlock=new block;
    // 添加前驱
    trueBlock->preds.push_back(curBlock);
    falseBlock->preds.push_back(curBlock);
    // 添加后继
    curBlock->succs.push_back(trueBlock);
    curBlock->succs.push_back(falseBlock);
    // 为trueBlock分配标签 和变量共享标号
    trueBlock->label=to_string(varCount++);
    // 生成条件跳转
    stringstream ss;
    ss<<"br i1 "<<lret.idname<<", label %"
    <<trueBlock->label<<", label %";
    // falseBlock的标签待定 更新curBlock的patchloc
    curBlock->patchloc={irCodeList.size(),ss.str().size()};
    irCodeList.push_back(ss.str());
    // 遍历右子树
    irCodeList.push_back(trueBlock->label+":");
    bool_ret rret=generate_bool(root->children[2],trueBlock);
    falseBlock->label=to_string(varCount++);
    rret.curBlock->succs.push_back(falseBlock);
    falseBlock->preds.push_back(rret.curBlock);
    // 回填
    irCodeList[curBlock->patchloc.first].insert(
        curBlock->patchloc.second,
        falseBlock->label);
    // 跳转到falseBlock
    irCodeList.push_back("br label %"+falseBlock->label);
    irCodeList.push_back(falseBlock->label+":");
    // 返回结果
    string res="%"+to_string(varCount++);
    ss.str("");
    ss.clear();
    ss<<res<<" = phi i1 [ false, %"
    <<curBlock->label<<" ], [ "
    <<(rret.idname==""?(rret.value?"true":"false")
        :rret.idname)
    <<", %"<<rret.curBlock->label<<" ]";
    irCodeList.push_back(ss.str());
    return {res, 0, falseBlock};  // 返回falseBlock
}
bool_ret generate_bool(node* root,block* curBlock){
    if(root->children[0]->self=="BCONST"){
        // 常量
        return {"", root->children[0]->children[0]->self=="true", curBlock};
    }
    if(root->children[0]->self=="!"){
        // 逻辑非
        return generate_not(root,curBlock);
    }
    if(root->children[1]->self=="&&"){
        // 逻辑与
        return generate_and(root,curBlock);
    }
    if(root->children[1]->self=="||"){
        // 逻辑或
        return generate_or(root,curBlock);
    }
    // 其余情况是关系表达式
    // 遍历左子树
    expr_ret lret=generate_expr(root->children[0]);
    // 遍历右子树
    expr_ret rret=generate_expr(root->children[2]);
    if(lret.idname==""
        &&rret.idname==""){
        // 常量表达式
        if(root->children[1]->self=="==")
            return {"", lret.value==rret.value, curBlock};
        if(root->children[1]->self=="!=")
            return {"", lret.value!=rret.value, curBlock};
        if(root->children[1]->self=="<")
            return {"", lret.value<rret.value, curBlock};
        if(root->children[1]->self==">")
            return {"", lret.value>rret.value, curBlock};
        if(root->children[1]->self=="<=")
            return {"", lret.value<=rret.value, curBlock};
        if(root->children[1]->self==">=")
            return {"", lret.value>=rret.value, curBlock};
        // 应该没有其他结果 这里先报个错以防万一
        cout<<"Error detected in exprGen"<<endl;
        exit(1);
    }
    // 剩下的情况包含变量
    // 依次检查是否是变量并处理
    string lval, rval, res;  // 左值 右值 结果值
    lval = lret.idname==""
            ?to_string(lret.value)
            :lret.idname;
    rval = rret.idname==""
            ?to_string(rret.value)
            :rret.idname;
    if(boolOpMap.find(root->children[1]->self)==boolOpMap.end()){
        // 应该没有其他结果 这里先报个错以防万一
        cout<<"Error detected in exprGen"<<endl;
        exit(1);
    }
    // 结果赋值给新变量
    res="%"+to_string(varCount++);
    stringstream ss;
    ss<<res<<" = icmp "
    <<boolOpMap[root->children[1]->self]<<" i32 "
    <<lval<<", "<<rval;
    irCodeList.push_back(ss.str());
    return {res, 0, curBlock};
}

void generate_int_assign(node* root){
    // 获取左值
    string idname=root->children[0]->children[0]->self;
    // 获取右值
    expr_ret ret=generate_expr(root->children[1]->children[1]);
    // 赋值
    stringstream ss;
    ss<<"store i32 "
    <<(ret.idname==""?to_string(ret.value)  // 常量
        :ret.idname)  // 变量 exprGen保证返回的是IR变量名
    <<", ptr "<<idnameInIr[idname]
    <<", align 4";
    irCodeList.push_back(ss.str());
    return ;
}

block* generate_bool_assign(node* root,block* curBlock){
    // 获取左值
    string idname=root->children[0]->children[0]->self;
    // 获取右值
    bool_ret ret=generate_bool(root->children[1]->children[1],curIrBlock);
    curIrBlock=ret.curBlock;
    if(ret.idname==""){
        // 常量直接赋值
        stringstream ss;
        ss<<"store i8 "
        <<(ret.value?"1":"0")
        <<", ptr "<<idnameInIr[idname]
        <<", align 1";
        irCodeList.push_back(ss.str());
        return curBlock;
    }
    // 变量先对齐宽度再赋值
    string vname="%"+to_string(varCount++);
    stringstream ss;
    ss<<vname<<" = zext i1 "
    <<ret.idname<<" to i8";
    irCodeList.push_back(ss.str());
    ss.str("");
    ss.clear();
    ss<<"store i8 "
    <<vname
    <<", ptr "<<idnameInIr[idname]
    <<", align 1";
    irCodeList.push_back(ss.str());
    return curBlock;
}

block* generate_if_else(node* root,block* curBlock){
    // 文法决定不需要解析条件 直接取ID的值
    // %x = load i8, ptr %id, align 1
    string idname=root->children[1]->children[0]->self;
    string vname="%"+to_string(varCount++);
    stringstream ss;
    ss<<vname<<" = load i8, ptr "
    <<idnameInIr[idname]<<", align 1";
    irCodeList.push_back(ss.str());
    ss.str("");
    ss.clear();
    // %cond = trunc i8 %id to i1
    string vname1="%"+to_string(varCount++);
    ss<<vname1<<" = trunc i8 "
    <<vname<<" to i1";
    irCodeList.push_back(ss.str());
    // 创建if块分支
    block *ifBlock=new block;
    ifBlock->preds.push_back(curBlock);
    curBlock->succs.push_back(ifBlock);
    // 为ifBlock分配标签 和变量共享标号
    ifBlock->label=to_string(varCount++);
    // 生成条件跳转
    ss.str("");
    ss.clear();
    ss<<"br i1 "<<vname1<<", label %"
    <<ifBlock->label<<", label %";
    // false跳转到哪待定 更新curBlock的patchloc
    curBlock->patchloc={irCodeList.size(),ss.str().size()};
    irCodeList.push_back(ss.str());
    // 更新控制流
    curIrBlock=ifBlock;
    // 遍历if子树
    irCodeList.push_back(ifBlock->label+":");
    generate_ir(root->children[3]);  // if ID then STMT
    // 创建finalBlock
    block *finalBlock=new block;
    // ifblock和finalBlock一定有前驱后继关系
    finalBlock->preds.push_back(curIrBlock);
    curIrBlock->succs.push_back(finalBlock);
    // 判断是否有else AST中已经删除了空的ELSE结点
    if(root->children.size()==4){
        // 单if 直接分配标签
        finalBlock->label=to_string(varCount++);
        // 存在直接前驱后继关系
        finalBlock->preds.push_back(curBlock);
        curBlock->succs.push_back(finalBlock);
        // 回填
        irCodeList[curBlock->patchloc.first].insert(
            curBlock->patchloc.second,
            finalBlock->label);
        // 跳转到finalBlock
        irCodeList.push_back("br label %"+finalBlock->label);
        irCodeList.push_back(finalBlock->label+":");
        return finalBlock;
    }
    // 有else
    // 创建else块分支
    block *elseBlock=new block;
    elseBlock->preds.push_back(curBlock);
    curBlock->succs.push_back(elseBlock);
    // 为elseBlock分配标签 和变量共享标号
    elseBlock->label=to_string(varCount++);
    // 回填
    irCodeList[curBlock->patchloc.first].insert(
        curBlock->patchloc.second,
        elseBlock->label);
    // 更新控制流
    curBlock=curIrBlock;
    curIrBlock=elseBlock;
    // 此时finalBlock的标签待定
    // 生成跳转
    irCodeList.push_back("br label %");
    curBlock->patchloc={irCodeList.size()-1,irCodeList.back().size()};
    // 遍历else子树
    irCodeList.push_back(elseBlock->label+":");  
    generate_ir(root->children[4]->children[1]);  // ELSE -> else STMT
    // 此时更新finalBlock的标签及前驱
    finalBlock->label=to_string(varCount++);
    curIrBlock->succs.push_back(finalBlock);
    finalBlock->preds.push_back(curIrBlock);
    // 回填
    irCodeList[curBlock->patchloc.first].insert(
        curBlock->patchloc.second,
        finalBlock->label);
    // 跳转到finalBlock
    irCodeList.push_back("br label %"+finalBlock->label);
    irCodeList.push_back(finalBlock->label+":");
    return finalBlock;
}

block* generate_while(node* root,block* curBlock){
    // 先创建并跳转到循环头
    block* headBlock=new block;
    headBlock->preds.push_back(curBlock);
    curBlock->succs.push_back(headBlock);
    headBlock->label=to_string(varCount++);
    irCodeList.push_back("br label %"+headBlock->label);
    irCodeList.push_back(headBlock->label+":");
    // 更新控制流并生成headBlock
    curBlock=headBlock;
    // 文法决定不需要解析条件 直接取ID的值
    // %x = load i8, ptr %id, align 1
    string idname=root->children[1]->children[0]->self;
    string vname="%"+to_string(varCount++);
    stringstream ss;
    ss<<vname<<" = load i8, ptr "
    <<idnameInIr[idname]<<", align 1";
    irCodeList.push_back(ss.str());
    ss.str("");
    ss.clear();
    // %cond = trunc i8 %id to i1
    string vname1="%"+to_string(varCount++);
    ss<<vname1<<" = trunc i8 "
    <<vname<<" to i1";
    irCodeList.push_back(ss.str());
    // 创建循环主体块
    block *loopBlock=new block;
    loopBlock->preds.push_back(curBlock);
    curBlock->succs.push_back(loopBlock);
    loopBlock->succs.push_back(curBlock);
    curBlock->preds.push_back(loopBlock);
    // 为loopBlock分配标签 和变量共享标号
    loopBlock->label=to_string(varCount++);
    // 生成条件跳转
    ss.str("");
    ss.clear();
    ss<<"br i1 "<<vname1<<", label %"
    <<loopBlock->label<<", label %";
    // false跳转到哪待定 更新curBlock的patchloc
    curBlock->patchloc={irCodeList.size(),ss.str().size()};
    irCodeList.push_back(ss.str());
    // 更新控制流
    curIrBlock=loopBlock;
    // 遍历循环主体子树
    irCodeList.push_back(loopBlock->label+":");
    generate_ir(root->children[3]);  // while ID do STMT
    // 创建finalBlock
    block *finalBlock=new block;
    finalBlock->preds.push_back(curIrBlock);
    curIrBlock->succs.push_back(finalBlock);
    finalBlock->preds.push_back(headBlock);
    headBlock->succs.push_back(finalBlock);
    finalBlock->label=to_string(varCount++);
    // 回填
    irCodeList[curBlock->patchloc.first].insert(
        curBlock->patchloc.second,
        finalBlock->label);
    // 跳转回headBlock
    irCodeList.push_back("br label %"+headBlock->label);
    // 打印finalBlock标签
    irCodeList.push_back(finalBlock->label+":");
    return finalBlock;
}

void generate_io(node* root){
    string idname=root->children[1]->children[0]->self;
    string mode=root->children[0]->self;
    if(mode=="read"){
        // read
        // %x = call i32 @read()
        // store i32 %x, ptr %id, align 4
        string vname="%"+to_string(varCount++);
        stringstream ss;
        ss<<vname<<" = call i32 @read()";
        irCodeList.push_back(ss.str());
        ss.str("");
        ss.clear();
        ss<<"store i32 "<<vname
        <<", ptr "<<idnameInIr[idname]
        <<", align 4";
        irCodeList.push_back(ss.str());
        // 更新控制流
        block* readBlock=new block;
        readBlock->preds.push_back(curIrBlock);
        curIrBlock->succs.push_back(readBlock);
        readBlock->succs.push_back(curIrBlock);
        curIrBlock->preds.push_back(readBlock);
        // 吃一个默认分配编号
        readBlock->label=to_string(varCount++);  
        readBlock->label+="(read)";
        return ;
    }
    // write
    // %x = load i32, ptr %id, align 4
    // call void @write(i32 %x)
    string vname="%"+to_string(varCount++);
    stringstream ss;
    ss<<vname<<" = load i32, ptr "
    <<idnameInIr[idname]<<", align 4";
    irCodeList.push_back(ss.str());
    ss.str("");
    ss.clear();
    ss<<"call i32 @write(i32 "
    <<vname<<")";
    irCodeList.push_back(ss.str());
    // 更新控制流
    block* writeBlock=new block;
    writeBlock->preds.push_back(curIrBlock);
    curIrBlock->succs.push_back(writeBlock);
    writeBlock->succs.push_back(curIrBlock);
    curIrBlock->preds.push_back(writeBlock);
    // 吃一个默认分配编号
    writeBlock->label=to_string(varCount++);
    writeBlock->label+="(write)";
    return ;
}

void generate_ir(node* root){
    // 声明语句
    if(root->self=="NAME"){
        generate_decl(root);
        return ;
    }
    // 整型赋值语句
    if(root->self=="STMT"
        &&root->children[1]->self=="ASSIGN"
        &&root->children[1]->children[0]->self=="="){
        generate_int_assign(root);
        return ;
    }
    // 布尔赋值语句
    if(root->self=="STMT"
        &&root->children[1]->self=="ASSIGN"
        &&root->children[1]->children[0]->self==":="){
        curIrBlock=generate_bool_assign(root,curIrBlock);
        return ;
    }
    // if-else
    if(root->self=="STMT"
        &&root->children[0]->self=="if"){
        curIrBlock=generate_if_else(root,curIrBlock);
        return ;
    }
    // while
    if(root->self=="STMT"
        &&root->children[0]->self=="while"){
        curIrBlock=generate_while(root,curIrBlock);
        return ;
    }
    // IO
    if(root->self=="STMT"
        &&(root->children[0]->self=="read"
            || root->children[0]->self=="write")){
        generate_io(root);
    }
    for(int i=0;i<root->children.size();i++){
        // 递归遍历子树
        generate_ir(root->children[i]);
    }
}
void semantic_analyze(node* root){
    // 类型检查
    if(!type_check(root,0)){
        // 类型检查不通过
        return ;
    }
    // 基本块初始化
    init_blocks();
    curIrBlock=entryBlock;
    // 中间代码生成
    generate_ir(root);
    // 打印中间代码
    print_ir();
}

void print_ir(){
    ofstream fout("out.ll");
    if(!fout){
        cout<<"Error: Cannot open file out.ll"<<endl;
        return ;
    }
    // 打印头部
    for(auto& it:irHead){
        fout<<it<<endl;
    }
    // 打印主函数头部
    for(auto& it:mainHead){
        fout<<it<<endl;
    }
    // 打印生成的中间代码
    for(auto& it:irCodeList){
        if(it.back()==':'){
            // 是标签的行 不缩进
            fout<<it<<endl;
        }else{
            // 不是标签的行 缩进
            fout<<"  "<<it<<endl;
        }
    }
    // 打印主函数尾部
    for(auto& it:mainTail){
        fout<<it<<endl;
    }
    fout.close();
    return ;
}