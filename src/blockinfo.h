#ifndef BLOCKINFO_H_
#define BLOCKINFO_H_

#include <map>
#include <stack>
#include <memory>
#include <cassert>
#include <iostream>
using namespace std;

class BlockInfo {
public:
    string id;
    bool created_by_fa;
    stack<int> block_entry, block_out;
    map<string, string> table;
    map<string, string> type;
    map<string, int> dim;
    unique_ptr<BlockInfo> fa;
    BlockInfo(int , BlockInfo*);
    string query(string);
    void insert(string, string, string);
    void insert(string, string, string, int);
    string qtype(string);
    int qdim(string);
};

#endif