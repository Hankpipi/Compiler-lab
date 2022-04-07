#include <ast.h>

int BaseAST::id = 0;
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
    if(block_id == 0)
        printf("%%entry:\n");
    BlockInfo* blk = new BlockInfo(++block_id);
    blk->fa = unique_ptr<BlockInfo>(b);
    item->GenIR(blk);
    return "";
}

std::string BlockItemStarAST::GenIR(BlockInfo* b) const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR(b);
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
    }
    else if(state == 2){
        std::string res = exp->GenIR(b);
        printf("  store %s, @%s\n", res.c_str(), b->query(var).c_str());
    }
    else if(state == 4 || state == 6)
        return exp->GenIR(b);
    else if(state == 5)
        printf("  ret\n");
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
    else {
        inst += " = ne " + ls + ", 0\n";
        ls = GenVar(id - 1);
        inst += "  " + GenVar(id++) + " = ne " + rs + ", 0\n";
        rs = GenVar(id - 1);
        inst += "  " + GenVar(id++);
        if (op == "&&")
            inst += " = and ";
        else if (op == "||")
            inst += " = or ";
    }

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
    if (op == "&&") return a && b;
    if (op == "||") return a || b;
    else assert(false);
}