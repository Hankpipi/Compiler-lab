#include <ast.h>

int BaseAST::id = 0;
int BaseAST::elf_id = 0;
int BaseAST::block_id = 0;

std::string GenVar(int num) {
    return "%" + std::to_string(num);
}

std::string CompUnitAST::GenIR(BlockInfo* b) const {
    printf("%s\n", func_def->GenIR(b).c_str());
    return "";
}

std::string FuncDefAST::GenIR(BlockInfo* b) const {
    std::string ret = "";
    ret += "fun @" + ident + "(): ";
    ret += func_type->GenIR(b) + " {\n";
    printf("%s", ret.c_str());
    block->GenIR(b);
    printf("}\n");
    return "";
}

std::string FuncTypeAST::GenIR(BlockInfo* b) const {
    if (type == "int")
        return "i32";
    else assert(false);
}

std::string BlockAST::GenIR(BlockInfo* b) const {
    if(elf_id == 0) printf("%%entry:\n");
    BlockInfo* blk = new BlockInfo(++elf_id, b);
    return item->GenIR(blk);
}

std::string BlockItemStarAST::GenIR(BlockInfo* b) const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        if(son[i]->GenIR(b) == "ret")
            return "ret";
    return "";
}

std::string BlockItemAST::GenIR(BlockInfo* b) const {
    if(state <= 2)
        return item->GenIR(b);
    return "";
}

std::string StmtAST::GenIR(BlockInfo* b) const {
    if (state == 1) {
        std::string res = exp->GenIR(b);
        printf("  ret %s\n", res.c_str());
        return "ret";
    }
    else if(state == 2){
        std::string res = exp->GenIR(b);
        printf("  store %s, @%s\n", res.c_str(), b->query(var).c_str());
    }
    else if(state == 4)
        return exp->GenIR(b);
    else if(state == 5) {
        printf("  ret\n");
        return "ret";
    }
    else if(state == 6)
        return exp->GenIR(b);
    else if(state == 7) {
        int block_if = block_id, block_out = block_id + 1;
        block_id += 2;
        b->block_true = block_if, b->block_false = block_out;
        std::string res = exp->GenIR(b);
        b->block_true = b->block_false = -1;
        printf("  br %s, %%blk%d, %%blk%d\n", res.c_str(), block_if, block_out);
        printf("%%blk%d:\n", block_if);

        res = son[0]->GenIR(b);
        if(res != "ret")
            printf("  jump %%blk%d\n", block_out);
        printf("%%blk%d:\n", block_out);
    }
    else if (state == 8) {
        int block_if = block_id, block_else = block_id + 1, block_out = block_id + 2;
        block_id += 3;
        b->block_true = block_if, b->block_false = block_else;
        std::string res = exp->GenIR(b);
        b->block_true = b->block_false = -1;
        printf("  br %s, %%blk%d, %%blk%d\n", res.c_str(), block_if, block_else);
        printf("%%blk%d:\n", block_if);
        std::string res0 = son[0]->GenIR(b);
        if(res0 != "ret") printf("  jump %%blk%d\n", block_out);
        printf("%%blk%d:\n", block_else);

        std::string res1 = son[1]->GenIR(b);
        if(res1 != "ret") printf("  jump %%blk%d\n", block_out);
        printf("%%blk%d:\n", block_out);
    }
    return "";
}

std::string ExpAST::GenIR(BlockInfo* b) const {
    return tuple_exp->GenIR(b);
}

std::string PrimaryExpAST::GenIR(BlockInfo* b) const {
    if (state == 1)
        return item->GenIR(b);
    if (state == 2)
        return var;
    if (b->isconst(var))
        return b->query(var);
    string value = b->query(var);
    if (value != "") {
        std::string reg = GenVar(id++);
        printf("  %s = load @%s\n", reg.c_str(), value.c_str());
        return reg;
    }
    assert(false);
}

std::string UnaryExpAST::GenIR(BlockInfo* b) const {
    if(state == 1)
        return primary_exp->GenIR(b);

    std::string ret = unary_exp->GenIR(b), inst = "";
    if(op == "!") {
        inst += "  %" + std::to_string(id) + " = ";
        inst += "eq " + ret + ", 0" + "\n";
    }
    else if(op == "-") {
        inst += "  %" + std::to_string(id) + " = ";
        inst += "sub 0, " + ret + "\n";
    } 
    else if (op == "+")
        return ret;
    printf("%s", inst.c_str());
    return GenVar(id++);
}

std::string AndOrAST::GenIR(BlockInfo* b) const {
    if(state == 1)
        return dst->GenIR(b);
    std::string ls = src->GenIR(b);

    printf("  %s = ne %s, 0\n", GenVar(id++).c_str(), ls.c_str());
    ls = GenVar(id - 1);
    if(b->block_true != -1 && op == "||") {
        printf("  br %s, %%blk%d, %%blk%d\n", ls.c_str(), b->block_true, block_id);
        printf("%%blk%d:\n", block_id++);
    }
    else if(b->block_false != -1 && op == "&&") {
        printf("  %%%d = eq %s, 0\n", id++, ls.c_str());
        printf("  br %%%d, %%blk%d, %%blk%d\n", id - 1, b->block_false, block_id);
        printf("%%blk%d:\n", block_id++);
    }

    std::string rs = dst->GenIR(b);
    printf("  %s = ne %s, 0\n", GenVar(id++).c_str(), rs.c_str());
    rs = GenVar(id - 1);
    printf("  %s", GenVar(id++).c_str());
    if (op == "&&")
        printf(" = and ");
    else if (op == "||")
        printf(" = or ");

    printf("%s, %s\n", ls.c_str(), rs.c_str());
    return GenVar(id - 1);
}

int AndOrAST::calc(BlockInfo* blk) const {
    if(state == 1)
        return dst->calc(blk);
    int a = src->calc(blk);
    int b = dst->calc(blk);
    if (op == "&&") return a && b;
    if (op == "||") return a || b;
    else assert(false);
}

std::string TupleExpAST::GenIR(BlockInfo* b) const {
    if(state == 1)
        return dst->GenIR(b);
    std::string ls = src->GenIR(b);
    std::string rs = dst->GenIR(b);

    std::string inst = "  " + GenVar(id++);
    if(op == "*")
        inst += " = mul ";
    else if (op == "/")
        inst += " = div ";
    else if (op == "%")
        inst += " = mod ";
    else if(op == "+")
        inst += " = add ";
    else if (op == "-")
        inst += " = sub ";
    else if (op == "==")
        inst += " = eq ";
    else if (op == "!=")
        inst += " = ne ";
    else if (op == ">")
        inst += " = gt ";
    else if (op == ">=")
        inst += " = ge ";
    else if (op == "<")
        inst += " = lt ";
    else if (op == "<=")
        inst += " = le ";
    else 
        assert(false);

    inst += ls + ", " + rs + "\n";
    printf("%s", inst.c_str());
    return GenVar(id - 1);
}

std::string DeclAST::GenIR(BlockInfo* b) const {
    return sub_decl->GenIR(b);
}

std::string ConstDefStarAST::GenIR(BlockInfo* b) const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR(b);
    return "";
}

std::string VarDefStarAST::GenIR(BlockInfo* b) const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR(b);
    return "";
}

std::string ConstDefAST::GenIR(BlockInfo* b) const {
    assert(b->table.find(var) == b->table.end());
    b->insert(var, std::to_string(exp->calc(b)), 1);
    return "";
}

std::string VarDefAST::GenIR(BlockInfo* b) const {
    assert(b->table.find(var) == b->table.end());
    std::string value = var + "_" + b->id;
    b->insert(var, value, 0);
    printf("  @%s = alloc i32\n", value.c_str());
    if(state == 1) {
        std::string res = exp->GenIR(b);
        printf("  store %s, @%s\n", res.c_str(), value.c_str());
    }
    return "";
}

int ExpAST::calc(BlockInfo* b) const {
    return tuple_exp->calc(b);
}

int PrimaryExpAST::calc(BlockInfo* b) const {
    if(state == 1) return item->calc(b);
    std::string num = (state == 2? var: b->query(var));
    int ret = 0;
    for(int i = 0, n = num.length(); i < n; ++i)
        ret = ret * 10 + num[i] - 48;
    return ret;
}

int UnaryExpAST::calc(BlockInfo* b) const {
    if(state == 1)
        return primary_exp->calc(b);
    int val = unary_exp->calc(b);
    if(op == "-") return -val;
    if(op == "!") return !val;
    return val;
}

int TupleExpAST::calc(BlockInfo* blk) const {
    if(state == 1)
        return dst->calc(blk);
    int a = src->calc(blk);
    int b = dst->calc(blk);
    if (op == "*") return a * b;
    if (op == "/") return a / b;
    if (op == "%") return a % b;
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "==") return a == b;
    if (op == "!=") return a != b;
    if (op == ">") return a > b;
    if (op == ">=") return a >= b;
    if (op == "<") return a < b;
    if (op == "<=") return a <= b;
    else assert(false);
}