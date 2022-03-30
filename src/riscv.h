#ifndef RISCV_H_
#define RISCV_H_
#include <map>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <koopa.h>
using namespace std;

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_integer_t &integer); 
void Visit(const koopa_raw_return_t &value);
void Visit(const koopa_raw_binary_t &binary);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load);

struct ptrCmp {
	bool operator() (const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

#endif
