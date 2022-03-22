#include <ast.h>
#include <memory>
#include <map>

int BaseAST::id = 0;

std::string GenVar(int num) {
    return "%" + std::to_string(num);
}

std::string CompUnitAST::GenIR() const {
    return func_def->GenIR() + '\n';
}

std::string FuncDefAST::GenIR() const {
        std::string ret = "";
        ret += "fun @" + ident + "(): ";
        ret += func_type->GenIR() + " {\n";
        ret += block->GenIR() + "}\n";
        return ret;
}

std::string FuncTypeAST::GenIR() const {
    if (type == "int")
        return "i32";
    else 
        std::cout << "error: not implement!";
}

std::string BlockAST::GenIR() const {
    return "%entry:\n" + stmt->GenIR();
}

std::string StmtAST::GenIR() const {
    return exp->GenIR() + "  ret %" + std::to_string(id - 1) + "\n";
}

std::string ExpAST::GenIR() const {
    return tuple_exp->GenIR();
}

std::string PrimaryExpAST::GenIR() const {
    if (state == 1)
        return exp->GenIR();
    return "  %" + std::to_string(id++) + " = add 0, " + std::to_string(number) + "\n";
}

std::string UnaryExpAST::GenIR() const {
    if(state == 1)
        return primary_exp->GenIR();

    std::string ret = "";
    ret += unary_exp->GenIR();
    if(op == "!") {
        ret += "  %" + std::to_string(id) + " = ";
        ret += "eq %" + std::to_string(id - 1) + ", 0" + "\n";
        id += 1;
    }
    else if(op == "-") {
        ret += "  %" + std::to_string(id) + " = ";
        ret += "sub 0, %" + std::to_string(id - 1) + "\n";
        id += 1;
    }
    return ret;
}

std::string TupleExpAST::GenIR() const {
    if(state == 1)
        return dst->GenIR();
    std::string ret = "";
    ret += src->GenIR();
    int src = id - 1;
    ret += dst->GenIR();
    int dst = id - 1;

    std::string inst = "  " + GenVar(id++);
    if(op == "*")
        inst += " = mul %";
    else if (op == "/")
        inst += " = div %";
    else if (op == "%")
        inst += " = mod %";
    else if(op == "+")
        inst += " = add %";
    else if (op == "-")
        inst += " = sub %";
    else if (op == "==")
        inst += " = eq %";
    else if (op == "!=")
        inst += " = ne %";
    else if (op == ">")
        inst += " = gt %";
    else if (op == ">=")
        inst += " = ge %";
    else if (op == "<")
        inst += " = lt %";
    else if (op == "<=")
        inst += " = le %";
    else {
        ret += inst + " = ne %" + std::to_string(src) + ", 0\n";
        src = id - 1;
        ret += "  " + GenVar(id++) + " = ne %" + std::to_string(dst) + ", 0\n";
        dst = id - 1;
        inst = "  " + GenVar(id++);
        if (op == "&&")
            inst += " = and %";
        else if (op == "||")
            inst += " = or %";
    }

    ret += inst + std::to_string(src) + ", %" + std::to_string(dst) + "\n";
    return ret;
}
