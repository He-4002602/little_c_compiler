#ifndef TREETOOLS_H
#define TREETOOLS_H
#include "cppinclude.h"
#include "preprocessor.h"
struct node{
    unsigned long long tokenIndex;  // token在list中的索引 非终结符0
    std::string self;
    std::vector<node*> children;
};
class TreePrinter{
    private:
        std::ofstream fout;
        std::unordered_set<std::string> noDelete;
        void print_tree_to_json(node* root,bool isLast);
        void deleteEmpty(node* root);

    public:
        explicit TreePrinter(const std::string& filename);
        ~TreePrinter();
        void print_tree(node* root);
        void close_fout();
};
class TreeReader{
    private:
        std::string jsonStr;
        unsigned long long read_tree_from_json(node* root,unsigned long long pos);
    public:
        explicit TreeReader(const std::string& filename);
        void read_tree(node* root);
};
#endif