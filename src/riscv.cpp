#include <riscv.h>
#define printf ++lines, printf

map<const char*, int, ptrCmp> table;
map<const char*, int, ptrCmp> global_table;
map<const char*, int, ptrCmp> block_line;
const char* reg[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
int F = 0, argstop = 0, lines = 0;
int S = 0, R = 0, A = 0;
map<koopa_raw_value_t, int> vis;
map<koopa_raw_value_t, int> vis_cnt;

void store_op(const char *src, const char *dst, int bias) {
    if(bias >= -2048 && bias <= 2047)
        printf("  sw %s, %d(%s)\n", src, bias, dst);
    else {
        printf("  li t3, %d\n", bias);
        printf("  add t3, t3, %s\n", dst);
        printf("  sw %s, 0(t3)\n", src);
    }
}

void load_op(const char *dst, const char *src, int bias) {
    if(bias >= -2048 && bias <= 2047)
        printf("  lw %s, %d(%s)\n", dst, bias, src);
    else {
        printf("  li t3, %d\n", bias);
        printf("  add t3, t3, %s\n", src);
        printf("  lw %s, 0(t3)\n", dst);
    }
}

void putargs(const koopa_raw_slice_t &slice) {
  argstop = 0;
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
        load_op(reg[7 + i], "sp", res);
    else {
        load_op("t1", "sp", res);
        store_op("t1", "sp", argstop);
        argstop += 4;
    }
  }
}

int CalcS(const koopa_raw_type_t &value, int dep) {
    if(value->tag == KOOPA_RTT_INT32) return 4;
    if(value->tag == KOOPA_RTT_ARRAY) {
        int tmp = CalcS(value->data.array.base, dep + 1);
        if(dep > 0) tmp *= value->data.array.len;
        return tmp;
    }
    if(value->tag == KOOPA_RTT_POINTER)
        return dep > 0? 4: CalcS(value->data.pointer.base, dep + 1);
    assert(false);
}

int CalcS(const koopa_raw_value_t &value) {
    if(vis_cnt.find(value) != vis_cnt.end()) 
        return vis_cnt[value]? 4: 0;

    const auto &kind = value->kind;
    int ret = 0;
    if(kind.tag == KOOPA_RVT_BINARY) 
        ret = CalcS(kind.data.binary.lhs) + CalcS(kind.data.binary.rhs) + 4;
    else if(kind.tag == KOOPA_RVT_LOAD) ret = CalcS(kind.data.load.src) + 4;
    else if(kind.tag == KOOPA_RVT_STORE) ret = CalcS(kind.data.store.value);
    else if(kind.tag == KOOPA_RVT_BRANCH) ret = CalcS(kind.data.branch.cond);
    else if(kind.tag == KOOPA_RVT_INTEGER) ret = 4;
    else if(kind.tag == KOOPA_RVT_FUNC_ARG_REF) ret = 4;
    else if(kind.tag == KOOPA_RVT_ALLOC) {
        if(value->ty->tag == KOOPA_RTT_POINTER)
            ret = CalcS(value->ty, 0);
    }
    else if(kind.tag == KOOPA_RVT_GET_ELEM_PTR) 
        ret = CalcS(kind.data.get_elem_ptr.src) + CalcS(kind.data.get_elem_ptr.index) + 4;
    else if(kind.tag == KOOPA_RVT_GET_PTR) 
        ret = CalcS(kind.data.get_ptr.src) + CalcS(kind.data.get_ptr.index) + 4;
    else if(kind.tag == KOOPA_RVT_CALL) {
        R = 4;
        const auto& call = kind.data.call;
        A = max(A, max((int)call.args.len - 8, 0) * 4);
        ret = call.args.len * 4;
        if(call.callee->ty->data.function.ret->tag == KOOPA_RTT_INT32)
            ret += 4;
    }
    vis_cnt[value] = ret;
    return ret;
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
    if(F) {
        if(F >= -2048 && F <= 2047)
            printf("  addi sp, sp, -%d\n", F);
        else {
            printf("  li t0, -%d\n", F);
            printf("  add sp, sp, t0\n");
        }
    }
    if(R) store_op("ra", "sp", F - 4);
    Visit(func->bbs);
    puts("");
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
    if(bb->name[1] == 'b') {
        printf("%s:\n", bb->name + 1);
        block_line[bb->name] = lines;
    }
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
        ret = Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
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
        ret = CalcS(value->ty, 0);
        table[value->name] = A;
        A += ret;
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
    case KOOPA_RVT_GET_ELEM_PTR:
        ret = Visit(kind.data.get_elem_ptr);
        break;
    case KOOPA_RVT_GET_PTR:
        ret = Visit(kind.data.get_ptr);
        break;
    case KOOPA_RVT_AGGREGATE:
        ret = Visit(kind.data.aggregate);
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
        load_op("a0", "sp", res);
    }
    if(R) load_op("ra", "sp", F - 4);
    if(F) {
        if(F >= -2048 && F <= 2047)
            printf("  addi sp, sp, %d\n", F);
        else {
            printf("  li t0, %d\n", F);
            printf("  add sp, sp, t0\n");
        }
    }
    printf("  ret\n");
    return 0;
}

int Visit(const koopa_raw_integer_t &value) {
    printf("  li %s, %d\n", reg[2], value.value);
    store_op(reg[2], "sp", A);
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_binary_t &binary) {
    int ls = Visit(binary.lhs);
    int rs = Visit(binary.rhs);
    load_op(reg[0], "sp", ls);
    load_op(reg[1], "sp", rs);
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
    store_op(reg[2], "sp", A);
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_store_t &store) {
    int res = Visit(store.value);
    if(!store.dest->name) {
        int dest = Visit(store.dest);
        load_op("t1", "sp", res);
        load_op("t0", "sp", dest);
        store_op("t1", "t0", 0);
        return res;
    }
    if(global_table.find(store.dest->name + 1) == global_table.end()) {
        if (table.find(store.dest->name) == table.end())
            table[store.dest->name] = res;
        else {
            load_op("t0", "sp", res);
            store_op("t0", "sp", table[store.dest->name]);
        }
    }
    else {
        load_op("t1", "sp", res);
        printf("  la t0, %s\n", store.dest->name + 1);
        store_op("t1", "t0", 0);
    }
    return res;
}

int Visit(const koopa_raw_load_t &load) {
    if(!load.src->name) {
        int src = Visit(load.src);
        load_op("t1", "sp", src);
        load_op("t0", "t1", 0);
    }
    else {
        if(table.find(load.src->name) != table.end())
            load_op("t0", "sp", table[load.src->name]);
        else {
            printf("  la t0, %s\n", load.src->name + 1);
            load_op("t0", "t0", 0);
        }
    }
    store_op("t0", "sp", A);
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_jump_t &jump) {
    printf("  j %s\n", jump.target->name + 1);
    return 0;
}

int Visit(const koopa_raw_branch_t &branch) {
    int res = Visit(branch.cond);
    load_op("t0", "sp", res);
    if(lines <= 2048) {
        printf("  bnez t0, %s\n", branch.true_bb->name + 1);
        printf("  j %s\n", branch.false_bb->name + 1);
    }
    else {
        printf("  bnez t0, mid_%s\n", branch.true_bb->name + 1);
        printf("  j %s\n", branch.false_bb->name + 1);
        printf("mid_%s:\n", branch.true_bb->name + 1);
        printf("  j %s\n", branch.true_bb->name + 1);
    }
    return 0;
}

int Visit(const koopa_raw_call_t &value) {
    putargs(value.args);
    printf("  call %s\n", value.callee->name + 1);
    if(value.callee->ty->data.function.ret->tag != KOOPA_RTT_UNIT)
        store_op("a0", "sp", A), A += 4;
    return A - 4;
}

int Visit(const koopa_raw_func_arg_ref_t &value) {
    if(value.index < 8)
        store_op(reg[7 + value.index], "sp", A);
    else {
        load_op("t1", "sp", F + (value.index - 8) * 4);
        store_op("t1", "sp", A);
    }
    A += 4;
    return A - 4;
}

int Visit(const koopa_raw_global_alloc_t &alloc) {
    if(alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT) {
        int res = CalcS(alloc.init->ty, 1);
        printf("  .zero %d\n", res);
    }
    else if(alloc.init->kind.tag == KOOPA_RVT_INTEGER)
        printf("  .word %d\n", alloc.init->kind.data.integer.value);
    else if(alloc.init->kind.tag == KOOPA_RVT_AGGREGATE)
        Visit(alloc.init);
    return 0;
}

int Visit(const koopa_raw_aggregate_t &agg) {
    for (size_t i = 0; i < agg.elems.len; ++i) {
        auto ptr = reinterpret_cast<koopa_raw_value_t>(agg.elems.buffer[i]);;
        if(ptr->kind.tag == KOOPA_RVT_ZERO_INIT) {
            int res = CalcS(ptr->ty, 1);
            printf("  .zero %d\n", res);
        }
        else if(ptr->kind.tag == KOOPA_RVT_INTEGER)
            printf("  .word %d\n", ptr->kind.data.integer.value);
        else if(ptr->kind.tag == KOOPA_RVT_AGGREGATE)
            Visit(ptr);
    }
    return 0;
}

int Visit(const koopa_raw_get_elem_ptr_t &ptr) {
    if(ptr.src->name) {
        if(global_table.find(ptr.src->name + 1) == global_table.end()) {
            if(table.find(ptr.src->name) == table.end())
                assert(false);
            int tmp = table[ptr.src->name];
            if(tmp >= -2048 && tmp <= 2047)
                printf("  addi t0, sp, %d\n", tmp);
            else {
                printf("  li t0, %d\n", tmp);
                printf("  add t0, t0, sp\n");
            }
        }
        else 
            printf("  la t0, %s\n", ptr.src->name + 1);
    }
    else {
        int res = Visit(ptr.src);
        load_op("t0", "sp", res);
    }

    if(ptr.index->kind.tag == KOOPA_RVT_INTEGER)
        printf("  li t1, %d\n", ptr.index->kind.data.integer.value);
    else {
        int res = Visit(ptr.index);
        load_op("t1", "sp", res);
    }
    int ptr_size = CalcS(ptr.src->ty, -1);
    printf("  li t2, %d\n", ptr_size);
    puts("  mul t1, t1, t2");
    puts("  add t0, t0, t1");
    store_op("t0", "sp", A);
    A += 4;
    return A - 4;       
}

int Visit(const koopa_raw_get_ptr_t &ptr) {
    if(ptr.src->name) {
        if(global_table.find(ptr.src->name + 1) == global_table.end()) {
            if(table.find(ptr.src->name) == table.end())
                assert(false);
            int tmp = table[ptr.src->name];
            if(tmp >= -2048 && tmp <= 4096)
                printf("  addi t0, sp, %d\n", tmp);
            else {
                printf("  li t0, %d\n", tmp);
                printf("  add t0, t0, sp\n");
            }
        }
        else 
            printf("  la t0, %s\n", ptr.src->name + 1);
    }
    else {
        int res = Visit(ptr.src);
        load_op("t0", "sp", res);
    }

    if(ptr.index->kind.tag == KOOPA_RVT_INTEGER)
        printf("  li t1, %d\n", ptr.index->kind.data.integer.value);
    else {
        int res = Visit(ptr.index);
        load_op("t1", "sp", res);
    }
    int ptr_size = CalcS(ptr.src->ty, 0);
    printf("  li t2, %d\n", ptr_size);
    puts("  mul t1, t1, t2");
    puts("  add t0, t0, t1");
    store_op("t0", "sp", A);
    A += 4;
    return A - 4;
}
