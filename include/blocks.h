#ifndef BLOCKS_H
#define BLOCKS_H
#include "cppinclude.h"
struct block{
    std::string label;  // 基本块标签
    std::vector<block*> preds;  // 前驱基本块
    std::vector<block*> succs;  // 后继基本块
    std::pair<unsigned long long, int> patchloc; // 回填位置 row col
};
extern block* entryBlock;  // 入口基本块
void init_blocks();  // 初始化基本块

class CFGPrinter {
public:
    CFGPrinter(block* entry, const std::string& filename);
    ~CFGPrinter();
    void print_cfg_to_json();
    void close_fout();

private:
    block* entryBlock;
    std::unordered_set<block*> visited;
    std::ofstream fout;

    void print_nodes(block* blk, bool isFirst);
    void print_edges(block* blk, bool isFirst);
};

#endif 