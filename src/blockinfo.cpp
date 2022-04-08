#include <blockinfo.h>

BlockInfo::BlockInfo(int id, BlockInfo* _fa) {
    fa = _fa? unique_ptr<BlockInfo>(_fa): NULL;
    block_in = block_out = block_next = -1;
    this->id = to_string(id);
}

void BlockInfo::insert(string key, string value, bool _is_const) {
    if(_is_const && table.find(key) != table.end())
        assert(false);
    is_const[key] = _is_const;
    table[key] = value;
}

string BlockInfo::query(string key) {
    for(auto& item: table) {
        if(item.first == key)
            return item.second;
    }
    if(fa) 
        return fa->query(key);
    assert(false);
}

bool BlockInfo::isconst(string key) {
    if(is_const.find(key) == is_const.end())
        return fa? fa->isconst(key) : false;
    return is_const[key];
}