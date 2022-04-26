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
    unique_ptr<BlockInfo> fa;
    BlockInfo(int , BlockInfo*);
    string query(string key);
    void insert(string key, string value, string type);
    string qtype(string key);
};

#endif