#include <riscv.h>

map<const char*, int, ptrCmp> table;
map<const char*, int, ptrCmp> global_table;
const char* reg[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
int F = 0, argstop = 0;
int S = 0, R = 0, A = 0;
map<koopa_raw_value_t, int> vis;

void putargs(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    int res = 0;
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        res = Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        assert(false);
    }
    if(i < 8)
        printf("  lw a%zu, %d(sp)\n", i, res);
    else {
        printf("  lw t1, %d(sp)\n", res);
        printf("  sw t1, %d(sp)\n", argstop);
        argstop += 4;
    }
  }
}

int CalcS(const koopa_raw_value_t &value) {
    const auto &kind = value->kind;
    if(kind.tag == KOOPA_RVT_BINARY) return 12;
    if(kind.tag == KOOPA_RVT_LOAD) return 4;
    if(kind.tag == KOOPA_RVT_STORE) return 4;
    if(kind.tag == KOOPA_RVT_BRANCH) return 4;
    if(kind.tag == KOOPA_RVT_INTEGER) return 4;
    if(kind.tag == KOOPA_RVT_FUNC_ARG_REF) return 4;
    if(kind.tag == KOOPA_RVT_CALL) {
        R = 4;
        const auto& call = kind.data.call;
        A = max(A, max((int)call.args.len - 8, 0) * 4);
        int ret = call.args.len * 4;
        if(call.callee->ty->data.function.ret->tag == KOOPA_RTT_INT32)
            ret += 4;
        return ret;
    }
    return 0;
}

int CalcS(const koopa_raw_slice_t &slice) {
  int ret = 0;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
        case KOOPA_RSIK_BASIC_BLOCK:
            ret += CalcS(reinterpret_cast<koopa_raw_basic_block_t>(ptr)->insts);
            break;
        case KOOPA_RSIK_VALUE:
            ret += CalcS(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            assert(false);
    }
  }
  return ret;
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
    puts("  .data");
    Visit(program.values);
    puts("");
    Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
    if(func->bbs.len == 0) return ;
    A = R = argstop = 0;
    table.clear();
    printf("  .text\n");
    printf("  .globl %s\n", func->name + 1);
    printf("%s:\n", func->name + 1);
    F = CalcS(func->bbs);
    F = (F + A + R + 15) / 16 * 16;
    F = F + A + R;
    if(F) printf("  addi sp, sp, -%d\n", F);
    if(R) printf("  sw ra, %d(sp)\n", F - 4);
    Visit(func->bbs);
    puts("");
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
    if(bb->name[1] == 'b')
        printf("%s:\n", bb->name + 1);
    // 访问所有指令
    Visit(bb->insts);
}

// 访问指令
int Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  if(vis.find(value) != vis.end())
    return vis[value];
  const auto &kind = value->kind;
  int ret = 0;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        ret = Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令
        ret = Visit(kind.data.integer);
        break;
    case KOOPA_RVT_BINARY:
        ret = Visit(kind.data.binary);
        break;
    case KOOPA_RVT_STORE:
        ret = Visit(kind.data.store);
        break;
    case KOOPA_RVT_LOAD:
        ret = Visit(kind.data.load);
        break;
    case KOOPA_RVT_ALLOC:
        ret = 0;
        break;
    case KOOPA_RVT_JUMP:
        ret = Visit(kind.data.jump);
        break;
    case KOOPA_RVT_BRANCH:
        ret = Visit(kind.data.branch);
        break;
    case KOOPA_RVT_CALL:
        ret = Visit(kind.data.call);
        break;
    case KOOPA_RVT_FUNC_ARG_REF:
        ret = Visit(kind.data.func_arg_ref);
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        printf("  .globl %s\n", value->name + 1);
        printf("%s:\n", value->name + 1);
        global_table[value->name + 1] = 1;
        ret = Visit(kind.data.global_alloc);
        break;
    default:
        assert(false);
  }
  vis[value] = ret;
  return ret;
}

int Visit(const koopa_raw_return_t &value) {
    if(value.value) {
        int res = Visit(value.value);
        printf("  lw a0, %d(sp)\n", res);
    }
    if(R) printf("  lw ra, %d(sp)\n", F - 4);
    if(F) printf("  addi sp, sp, %d\n", F);
    printf("  ret\n");
    return 0;
}

int Visit(const koopa_raw_integer_t &value) {
    printf("  li %s, %d\n", reg[2], value.value);
    printf("  sw %s, %d(sp)\n", reg[2], A);
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_binary_t &binary) {
    int ls = Visit(binary.lhs);
    int rs = Visit(binary.rhs);
    printf("  lw %s, %d(sp)\n", reg[0], ls);
    printf("  lw %s, %d(sp)\n", reg[1], rs);
    switch (binary.op) {
        case KOOPA_RBO_NOT_EQ:
            printf("  xor %s, %s, %s\n", reg[2], reg[0], reg[1]);
            printf("  snez %s, %s\n", reg[2], reg[2]);
            break;
        case KOOPA_RBO_EQ:
            printf("  xor %s, %s, %s\n", reg[2], reg[0], reg[1]);
            printf("  seqz %s, %s\n", reg[2], reg[2]);
            break;
        case KOOPA_RBO_GT:
            printf("  sgt %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_LT:
            printf("  slt %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_GE:
            printf("  slt %s, %s, %s\n", reg[3], reg[0], reg[1]);
            printf("  xori %s, %s, 1\n", reg[2], reg[3]);
            break;
        case KOOPA_RBO_LE:
            printf("  sgt %s, %s, %s\n", reg[3], reg[0], reg[1]);
            printf("  xori %s, %s, 1\n", reg[2], reg[3]);
            break;
        case KOOPA_RBO_ADD:
            printf("  add %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_SUB:
            printf("  sub %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_MUL:
            printf("  mul %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_DIV:
            printf("  div %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_MOD:
            printf("  rem %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_AND:
            printf("  and %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_OR:
            printf("  or %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_XOR:
            printf("  xor %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_SHL:
            printf("  sll %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_SHR:
            printf("  srl %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        case KOOPA_RBO_SAR:
            printf("  sra %s, %s, %s\n", reg[2], reg[0], reg[1]);
            break;
        default:
            assert(false);
    }
    printf("  sw %s, %d(sp)\n", reg[2], A);
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_store_t &store) {
    int res = Visit(store.value);
    if(global_table.find(store.dest->name + 1) == global_table.end()) {
        if (table.find(store.dest->name) == table.end())
            table[store.dest->name] = res;
        else printf("  sw %s, %d(sp)\n", reg[2], table[store.dest->name]);
    }
    else {
        printf("  lw t1, %d(sp)\n", res);
        printf("  la t0, %s\n", store.dest->name + 1);
        printf("  sw t1, 0(t0)\n");
    }
    return res;
}

int Visit(const koopa_raw_load_t &load) {
    if(table.find(load.src->name) != table.end()) {
        printf("  lw %s, %d(sp)\n", reg[2], table[load.src->name]);
        printf("  sw %s, %d(sp)\n", reg[2], A);
    }
    else {
        printf("  la t0, %s\n", load.src->name + 1);
        printf("  lw t0, 0(t0)\n");
        printf("  sw t0, %d(sp)\n", A);
    }
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_jump_t &jump) {
    printf("  j %s\n", jump.target->name + 1);
    return 0;
}

int Visit(const koopa_raw_branch_t &branch) {
    int res = Visit(branch.cond);
    printf("  lw t0, %d(sp)\n", res);
    printf("  bnez t0, %s\n", branch.true_bb->name + 1);
    printf("  j %s\n", branch.false_bb->name + 1);
    return 0;
}

int Visit(const koopa_raw_call_t &value) {
    putargs(value.args);
    printf("  call %s\n", value.callee->name + 1);
    if(value.callee->ty->data.function.ret->tag != KOOPA_RTT_UNIT)
        printf("  sw a0, %d(sp)\n", A), A += 4;
    return A - 4;
}

int Visit(const koopa_raw_func_arg_ref_t &value) {
    if(value.index < 8)
        printf("  sw a%zu, %d(sp)\n", value.index, A);
    else {
        printf("  lw t1, %lu(sp)\n", F + (value.index - 8) * 4);
        printf("  sw t1, %d(sp)\n", A);
    }
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_global_alloc_t &alloc) {
    if(alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT) printf("  .zero 4\n");
    else printf("  .word %d\n", alloc.init->kind.data.integer.value);
    return 0;
}