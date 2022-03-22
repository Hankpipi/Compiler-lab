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
    return add_exp->GenIR();
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

std::string MulExpAST::GenIR() const {
    if(state == 1)
        return unary_exp->GenIR();
    std::string ret = "";
    ret += mul_exp->GenIR();
    int src = id - 1;
    ret += unary_exp->GenIR();
    int dst = id - 1;

    std::string inst = "";
    if(op == "*")
        inst += "  " + GenVar(id++) + " = mul %";
    else if (op == "/")
        inst += "  " + GenVar(id++) + " = div %";
    else if (op == "%")
        inst += "  " + GenVar(id++) + " = mod %";

    ret += inst + std::to_string(src) + ", %" + std::to_string(dst) + "\n";
    return ret;
}

std::string AddExpAST::GenIR() const {
    if(state == 1)
        return mul_exp->GenIR();
    std::string ret = "";
    ret += add_exp->GenIR();
    int src = id - 1;
    ret += mul_exp->GenIR();
    int dst = id - 1;

    std::string inst = "";
    if(op == "+")
        inst += "  " + GenVar(id++) + " = add %";
    else if (op == "-")
        inst += "  " + GenVar(id++) + " = sub %";

    ret += inst + std::to_string(src) + ", %" + std::to_string(dst) + "\n";
    return ret;
}