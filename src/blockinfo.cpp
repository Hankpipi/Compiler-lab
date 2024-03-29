#include <blockinfo.h>

BlockInfo::BlockInfo(int id, BlockInfo* _fa) {
    if(_fa) {
        fa = unique_ptr<BlockInfo>(_fa);
        if(!_fa->block_entry.empty()) {
            block_entry.push(_fa->block_entry.top());
            block_out.push(_fa->block_out.top());
        }
    }
    created_by_fa = false;
    this->id = to_string(id);
}

void BlockInfo::insert(string key, string value, string _type) {
    if(_type == "int_const" && table.find(key) != table.end())
        assert(false);
    type[key] = _type;
    table[key] = value;
    dim[key] = 0;
}

void BlockInfo::insert(string key, string value, string _type, int _dim) {
    if(_type == "int_const" && table.find(key) != table.end())
        assert(false);
    type[key] = _type;
    table[key] = value;
    dim[key] = _dim;
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

string BlockInfo::qtype(string key) {
    if(type.find(key) == type.end())
        return fa? fa->qtype(key) : "";
    return type[key];
}

int BlockInfo::qdim(string key) {
    if(dim.find(key) == dim.end())
        return fa? fa->qdim(key) : 0;
    return dim[key];
}