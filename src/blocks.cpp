#include "blocks.h"
using namespace std;
block* entryBlock;
void init_blocks(){
    entryBlock=new block();
    entryBlock->label="0";
    entryBlock->patchloc={0,0};
    entryBlock->preds.clear();
    entryBlock->succs.clear();
}
CFGPrinter::CFGPrinter(block* entry, const std::string& filename) 
    : entryBlock(entry) {
    fout.open(filename);
}

CFGPrinter::~CFGPrinter() {
    if (fout.is_open()) fout.close();
}

void CFGPrinter::print_cfg_to_json() {
    fout << "{";
    fout << "\"nodes\":[";
    visited.clear();
    print_nodes(entryBlock, true);
    fout << "],";

    fout << "\"edges\":[";
    visited.clear();
    print_edges(entryBlock, true);
    fout << "]";
    fout << "}";
}

void CFGPrinter::print_nodes(block* blk, bool isFirst) {
    if (!blk || visited.count(blk)) return;
    visited.insert(blk);

    if (!isFirst) fout << ",";
    fout << "{";
    fout << "\"label\":\"" << blk->label << "\",";
    fout << "\"patchloc\":[" << blk->patchloc.first << "," << blk->patchloc.second << "]";
    fout << "}";

    for (auto succ : blk->succs) {
        print_nodes(succ, false);
    }
}

void CFGPrinter::print_edges(block* blk, bool isFirst) {
    if (!blk || visited.count(blk)) return;
    visited.insert(blk);

    for (auto succ : blk->succs) {
        if (!isFirst) fout << ",";
        fout << "{";
        fout << "\"from\":\"" << blk->label << "\",";
        fout << "\"to\":\"" << succ->label << "\"";
        fout << "}";
        isFirst = false;
    }


    for (auto succ : blk->succs) {
        print_edges(succ, isFirst);
    }
}
void CFGPrinter::close_fout() {
    fout.close();
}