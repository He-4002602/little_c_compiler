#include "treetools.h"
using namespace std;
void TreePrinter::print_tree_to_json(node* root,bool isLast){
    fout<<"{";
    if((root->children).empty()){
        fout<<"\"name\":\""<<root->self<<"\",";
        fout<<"\"tokenIndex\":"<<root->tokenIndex<<",";
        fout<<"\"children\":[]";
        fout<<"}";
        if(!isLast)fout<<",";
        return;
    }
    fout<<"\"name\":\""<<root->self<<"\",";
    fout<<"\"tokenIndex\":"<<root->tokenIndex<<",";
    fout<<"\"children\":[";
    for(int i=0;i<(root->children).size();++i)
        print_tree_to_json((root->children)[i],i==(root->children).size()-1);
    fout<<"]";
    fout<<"}";
    if(!isLast)fout<<",";
}

void TreePrinter::deleteEmpty(node* root){
    if(noDelete.find(root->self)!=noDelete.end()){
        // 直接返回
        return;
    }
    for(int i=0;i<root->children.size();++i){
        node* child=root->children[i];
        if(child->children.empty()&&noDelete.find(child->self)==noDelete.end()){
            // cout<<child->self<<" "<<child->tokenIndex<<endl;
            delete child;
            root->children.erase(root->children.begin()+i);
            --i;
        }
    }
    for(auto& it:root->children){
        deleteEmpty(it);
    }
}

TreePrinter::TreePrinter(const string& filename){
    fout.open(filename);
    if (!fout.is_open()) {
        throw runtime_error("Failed to open file");
    }
    vector<string> initializer={
        "ID","NUM","BCONST","M","int","bool",
        "+","-","*","/","&&","||","==",">",">=","<","<=","!=","!",
        "if","then","else","while","do","read","write","=",":="
    };
    noDelete.insert(initializer.begin(),initializer.end());
}
TreePrinter::~TreePrinter(){
    if (fout.is_open()) {
        fout.close();
    }
}
void TreePrinter::print_tree(node* root){
    deleteEmpty(root);
    print_tree_to_json(root,true);
}
void TreePrinter::close_fout(){
    fout.close();
}


unsigned long long TreeReader::read_tree_from_json(node* root,unsigned long long pos){
    root->self="";
    root->children.clear();
    unsigned long long i=pos;
    // name
    while(jsonStr[i]!=':')++i;
    for(i=i+2;jsonStr[i]!='\"';++i)
        root->self+=jsonStr[i];
    // tokenIndex
    while(jsonStr[i]!=':')++i;
    string tokenIndexStr="";
    for(i=i+1;jsonStr[i]!=',';++i)
        tokenIndexStr+=jsonStr[i];
    root->tokenIndex=stoull(tokenIndexStr);
    // children
    while(jsonStr[i]!='[')++i;
    ++i;
    node* temp;
    while(jsonStr[i]!=']'){
        temp=new node;
        i=read_tree_from_json(temp,i);
        root->children.push_back(temp);
    };
    while(jsonStr[i]!='}')++i;
    return i+1;
}

TreeReader::TreeReader(const string& filename){
    ifstream fin(filename);
    if (!fin.is_open()) {
        throw runtime_error("Failed to open file");
    }
    jsonStr="";
    if(get_file_size(filename.data())>jsonStr.max_size()){
        fin.close();
        stringstream ss;
        ss<<"The size of the file "<<filename<<" is "<<get_file_size(filename.data())
            <<" bytes, which exceeds the limit of "<<jsonStr.max_size();
        throw string(ss.str());
    }
    string buf;
    while(getline(fin,buf))
        jsonStr+=(buf+'\n');
    fin.close();
}
void TreeReader::read_tree(node* root){
    read_tree_from_json(root,0);
}