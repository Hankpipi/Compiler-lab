#ifndef RISCV_H_
#define RISCV_H_
#include <map>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <koopa.h>
using namespace std;

void Visit(const koopa_raw_program_t &);
void Visit(const koopa_raw_slice_t &);
void Visit(const koopa_raw_function_t &);
void Visit(const koopa_raw_basic_block_t &);
int Visit(const koopa_raw_value_t &);
int Visit(const koopa_raw_integer_t &); 
int Visit(const koopa_raw_return_t &);
int Visit(const koopa_raw_binary_t &);
int Visit(const koopa_raw_store_t &);
int Visit(const koopa_raw_load_t &);
int CalcS(const koopa_raw_slice_t &);
int CalcS(const koopa_raw_value_t &);
int CalcS(const koopa_raw_type_t &, int);
int Visit(const koopa_raw_jump_t &);
int Visit(const koopa_raw_branch_t &);
int Visit(const koopa_raw_call_t &);
int Visit(const koopa_raw_func_arg_ref_t &);
void putargs(const koopa_raw_slice_t &);
int Visit(const koopa_raw_global_alloc_t &);
int Visit(const koopa_raw_get_elem_ptr_t &);
int Visit(const koopa_raw_get_ptr_t &);
int Visit(const koopa_raw_aggregate_t &);

struct ptrCmp {
	bool operator() (const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

#endif
