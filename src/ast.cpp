#include <ast.h>

int BaseAST::id = 0;
std::map<std::string, std::string> table;
std::map<std::string, std::string> vartable;

std::string GenVar(int num) {
    return "%" + std::to_string(num);
}

std::string CompUnitAST::GenIR() const {
    printf("%s\n", func_def->GenIR().c_str());
    return "";
}

std::string FuncDefAST::GenIR() const {
    std::string ret = "";
    ret += "fun @" + ident + "(): ";
    ret += func_type->GenIR() + " {\n";
    printf("%s", ret.c_str());
    block->GenIR();
    printf("}\n");
    return "";
}

std::string FuncTypeAST::GenIR() const {
    if (type == "int")
        return "i32";
    else assert(false);
}

std::string BlockAST::GenIR() const {
    printf("%%entry:\n");
    item->GenIR();
    return "";
}

std::string BlockItemStarAST::GenIR() const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR();
    return "";
}

std::string BlockItemAST::GenIR() const {
    return item->GenIR();
}

std::string StmtAST::GenIR() const {
    if (state == 1) {
        std::string res = exp->GenIR();
        printf("  ret %s\n", res.c_str());
    }
    else {
        std::string res = exp->GenIR();
        printf("  store %s, @%s\n", res.c_str(), vartable[var].c_str());
    }
    return "";
}

std::string ExpAST::GenIR() const {
    return tuple_exp->GenIR();
}

std::string PrimaryExpAST::GenIR() const {
    if (state == 1)
        return item->GenIR();
    if (state == 2)
        return var;
    if (table.find(var) != table.end())
        return table[var];
    if (vartable.find(var) != vartable.end()) {
        std::string reg = GenVar(id++);
        printf("  %s = load @%s\n", reg.c_str(), vartable[var].c_str());
        return reg;
    }
    assert(false);
}

std::string UnaryExpAST::GenIR() const {
    if(state == 1)
        return primary_exp->GenIR();

    std::string ret = unary_exp->GenIR(), inst = "";
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

std::string TupleExpAST::GenIR() const {
    if(state == 1)
        return dst->GenIR();
    std::string ls = src->GenIR();
    std::string rs = dst->GenIR();

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

std::string DeclAST::GenIR() const {
    return sub_decl->GenIR();
}

std::string ConstDefStarAST::GenIR() const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR();
    return "";
}

std::string VarDefStarAST::GenIR() const {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR();
    return "";
}

std::string ConstDefAST::GenIR() const {
    assert(table.find(var) == table.end());
    table[var] = std::to_string(exp->calc());
    return "";
}

std::string VarDefAST::GenIR() const {
    assert(vartable.find(var) == vartable.end());
    vartable[var] = "my_" + var;
    printf("  @%s = alloc i32\n", vartable[var].c_str());
    if(state == 1) {
        std::string res = exp->GenIR();
        printf("  store %s, @%s\n", res.c_str(), vartable[var].c_str());
    }
    return "";
}

int ExpAST::calc() const {
    return tuple_exp->calc();
}

int PrimaryExpAST::calc() const {
    if(state == 1) return item->calc();
    if(state == 3)
        assert(table.find(var) != table.end());
    std::string num = (state == 2? var: table[var]);
    int ret = 0;
    for(int i = 0, n = num.length(); i < n; ++i)
        ret = ret * 10 + num[i] - 48;
    return ret;
}

int UnaryExpAST::calc() const {
    if(state == 1)
        return primary_exp->calc();
    int val = unary_exp->calc();
    if(op == "-") return -val;
    if(op == "!") return !val;
    return val;
}

int TupleExpAST::calc() const {
    if(state == 1)
        return dst->calc();
    int a = src->calc();
    int b = dst->calc();
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