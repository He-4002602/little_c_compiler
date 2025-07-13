#include "semantic.h"
using namespace std;

// 获取当前可执行文件的路径（Unicode版本）
wstring get_executable_directory() {
    wchar_t buffer[MAX_PATH];
    wstring path = L"";

    // Windows API 获取当前可执行文件的路径（Unicode版本）
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    path = buffer;
    path = path.substr(0, path.find_last_of(L"\\/")); // 提取目录部分

    return path;
}

// 将相对路径转换为绝对路径
wstring convert_to_absolute_path(const wstring& relative_path) {
    wchar_t full_path[MAX_PATH];
    if (_wfullpath(full_path, relative_path.c_str(), MAX_PATH)) {
        return wstring(full_path);
    } else {
        wcerr << L"Failed to convert relative path to absolute path." << endl;
        return L"";
    }
}

void set_working_directory_to_executable() {
    wstring exe_dir = get_executable_directory();

    // 设置当前工作目录为程序的目录
    if (SetCurrentDirectoryW(exe_dir.c_str())) {
        // wcout << L"Current working directory has been changed to: " << exe_dir << endl;
    } else {
        wcerr << L"Failed to change directory to: " << exe_dir << endl;
    }
}

int main(int argc,char* argv[]){
    if(argc!=5&&argc!=4&&argc!=3&&argc!=2){
        cerr<<"The correct input form is:"<<endl;
        cerr<<argv[0]<<" <source program> [output file] [--visAST] [--visCFG]"<<endl;
        return 0;
    }
    string sourceName=argv[1],tableName="../config/WordsTable.csv",outName;
    wstring temp=convert_to_absolute_path(wstring(sourceName.begin(),sourceName.end()));
    sourceName=string(temp.begin(),temp.end());
    if(argc>=3 && argv[2][0]!='-'){
        outName=argv[2];
    }else{
        outName="out.exe";
    }
    temp=convert_to_absolute_path(wstring(outName.begin(),outName.end()));
    outName=string(temp.begin(),temp.end());
    set_working_directory_to_executable();
    try{
        
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
        TreePrinter treePrinter("AST.json");
        treePrinter.print_tree(astRoot);
        treePrinter.close_fout();
        semantic_analyze(astRoot);
        if(!semanticErrorList.empty()){
            for(auto& e:semanticErrorList){
                cout<<e<<endl;
            }
            exit(1);
        }
        CFGPrinter cfgPrinter(entryBlock,"CFG.json");
        cfgPrinter.print_cfg_to_json();
        cfgPrinter.close_fout();
        vector<string> commandStr={
            "clang -O2 out.ll -o "+outName,
            "",
            "",
            "del /F AST.json",
            "del /F CFG.json",
            "del /F out.ll"
        };
        for(int i=2;i<argc;i++){
            if(string(argv[i])=="--visAST")
                commandStr[1]="python vis_ast.py";
            if(string(argv[i])=="--visCFG")
                commandStr[2]="python vis_cfg.py";
        }
        for(auto& it:commandStr){
            if(it.empty()){
                continue;
            }
            system(it.c_str());
        }
    }catch(string& e){
        cout<<e<<endl;
    }
    return 0;
}