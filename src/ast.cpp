#include <ast.h>
#include <memory>
#include <map>

int BaseAST::id = 0;

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
    else 
        std::cout << "error: not implement!";
}

std::string BlockAST::GenIR() const {
    printf("%%entry:\n");
    stmt->GenIR();
    return "";
}

std::string StmtAST::GenIR() const {
    std::string res = exp->GenIR();
    printf("  ret %s\n", res.c_str());
    return "";
}

std::string ExpAST::GenIR() const {
    return tuple_exp->GenIR();
}

std::string PrimaryExpAST::GenIR() const {
    if (state == 1)
        return exp->GenIR();
    return std::to_string(number);
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
