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
    int exp_true, exp_false;
    stack<int> block_entry, block_out;
    map<string, string> table;
    map<string, bool> is_const;
    unique_ptr<BlockInfo> fa;
    BlockInfo(int , BlockInfo*);
    string query(string key);
    void insert(string key, string value, bool is_const);
    bool isconst(string key);
};

#endif