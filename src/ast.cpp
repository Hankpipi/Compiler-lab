#include <ast.h>
#include <memory>
#include <map>

int BaseAST::id = 0;

void CompUnitAST::Dump() const {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
}
std::string CompUnitAST::GenIR() const {
    return func_def->GenIR() + '\n';
}


void FuncDefAST::Dump() const {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
}
std::string FuncDefAST::GenIR() const {
        std::string ret = "";
        ret += "fun @" + ident + "(): ";
        ret += func_type->GenIR() + " {\n";
        ret += block->GenIR() + "}\n";
        return ret;
}


void FuncTypeAST::Dump() const {
    std::cout << "FuncTypeAST { " << type << " }";
}
std::string FuncTypeAST::GenIR() const {
    if (type == "int")
        return "i32";
    else 
        std::cout << "error: not implement!";
}


void BlockAST::Dump() const {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
}
std::string BlockAST::GenIR() const {
    return "%entry:\n" + stmt->GenIR();
}


void StmtAST::Dump() const {}
std::string StmtAST::GenIR() const {
    return exp->GenIR() + "  ret %" + std::to_string(id - 1) + "\n";
}


void ExpAST::Dump() const {}
std::string ExpAST::GenIR() const {
    return unary_exp->GenIR();
}


void PrimaryExpAST::Dump() const {}
std::string PrimaryExpAST::GenIR() const {
    if (state == 1)
        return exp->GenIR();
    return "  %" + std::to_string(id++) + " = add 0, " + std::to_string(number) + "\n";
}


void UnaryExpAST::Dump() const {}
std::string UnaryExpAST::GenIR() const {
    if(state == 1)
        return primary_exp->GenIR();

    std::string ret = "";
    ret += unary_exp->GenIR();
    ret += unary_op->GenIR();
    return ret;
}


void UnaryOpAST::Dump() const {}
std::string UnaryOpAST::GenIR() const {
    std::string ret = "";
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