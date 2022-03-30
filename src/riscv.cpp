#include <riscv.h>

map<const char*, int, ptrCmp> table;
const char* reg[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
int st[105];
int cnt = 0, top = 0;

int CalcS(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_BINARY:
        return 12;
    case KOOPA_RVT_LOAD:
        return 4;
    default:
        break;
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
          // 访问基本块
            ret += CalcS(reinterpret_cast<koopa_raw_basic_block_t>(ptr)->insts);
            break;
        case KOOPA_RSIK_VALUE:
            ret += CalcS(reinterpret_cast<koopa_raw_value_t>(ptr));
        default:
            break;
    }
  }
  return ret;
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  printf("  .text\n");
  Visit(program.values);
  // 访问所有函数
  printf("  .globl main\n");
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
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {

  printf("%s:\n", func->name + 1);
  int frame_size = CalcS(func->bbs);
  printf("  addi sp, sp, -%d\n", frame_size);
  // 访问所有基本块
  Visit(func->bbs);
  printf("  addi sp, sp, %d\n", frame_size);
  printf("  ret\n");
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令
        Visit(kind.data.integer);
        break;
    case KOOPA_RVT_BINARY:
        if(cnt > 0 && st[cnt] == 1)
            Visit(kind.data.binary);
        break;
    case KOOPA_RVT_STORE:
        Visit(kind.data.store);
        break;
    case KOOPA_RVT_LOAD:
        if(cnt > 0 && st[cnt] == 1)
            Visit(kind.data.load);
        break;
    case KOOPA_RVT_ALLOC:
        break;
    default:
      assert(false);
  }
}

void Visit(const koopa_raw_return_t &value) {
    st[++cnt] = 1;
    Visit(value.value);
    printf("  lw a0, %d(sp)\n", top - 4);
    cnt -= 1;
}

void Visit(const koopa_raw_integer_t &value) {
    printf("  li %s, %d\n", reg[0], value.value);
    printf("  sw %s, %d(sp)\n", reg[0], top);
    top += 4;
}

void Visit(const koopa_raw_binary_t &binary) {
    Visit(binary.lhs);
    int ls = top - 4;
    Visit(binary.rhs);
    printf("  lw %s, %d(sp)\n", reg[0], ls);
    printf("  lw %s, %d(sp)\n", reg[1], top - 4);
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
            printf("  xori %s, %s, 1\n", reg[2], reg[2]);
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
    printf("  sw %s, %d(sp)\n", reg[2], top);
    top += 4;
}

void Visit(const koopa_raw_store_t &store) {
    // printf("In store %d %d\n", store.dest->kind.tag, store.value->kind.tag);
    st[++cnt] = 1;
    Visit(store.value);
    if (table.find(store.dest->name) == table.end())
        table[store.dest->name] = top - 4;
    else printf("  sw %s, %d(sp)\n", reg[2], table[store.dest->name]);
    --cnt;
}

void Visit(const koopa_raw_load_t &load) {
    printf("  lw %s, %d(sp)\n", reg[2], table[load.src->name]);
    printf("  sw %s, %d(sp)\n", reg[2], top);
    top += 4;
}
