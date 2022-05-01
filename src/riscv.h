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
int Visit(const koopa_raw_value_t &value);
int Visit(const koopa_raw_integer_t &integer); 
int Visit(const koopa_raw_return_t &value);
int Visit(const koopa_raw_binary_t &binary);
int Visit(const koopa_raw_store_t &store);
int Visit(const koopa_raw_load_t &load);
int CalcS(const koopa_raw_slice_t &slice);
int CalcS(const koopa_raw_value_t &value);
int Visit(const koopa_raw_jump_t &jump);
int Visit(const koopa_raw_branch_t &branch);
int Visit(const koopa_raw_call_t &value);
int Visit(const koopa_raw_func_arg_ref_t &value);
void putargs(const koopa_raw_slice_t &slice);
int Visit(const koopa_raw_global_alloc_t &value);

struct ptrCmp {
	bool operator() (const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

#endif
