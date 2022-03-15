#ifndef AST_H_
#define AST_H_
#include <iostream>
#include <memory>
#include <map>

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const {}
  virtual std::string GenIR() const {}
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_def;
    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }
    std::string GenIR() const override {
        return func_def->GenIR() + '\n';
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }
    std::string GenIR() const override {
        std::string ret = "";
        ret += "fun @" + ident + "(): ";
        ret += func_type->GenIR() + " {\n";
        ret += block->GenIR() + "}\n";
        return ret;
    }
};

class FuncTypeAST : public BaseAST {
    public:
    std::string type;
    void Dump() const override {
        std::cout << "FuncTypeAST { " << type << " }";
    }
    std::string GenIR() const override {
        if (type == "int")
            return "i32";
        else 
            std::cout << "error: not implement!";
    }
};

class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override {
        std::cout << "BlockAST { ";
        stmt->Dump();
        std::cout << " }";
    }
    std::string GenIR() const override {
        return "%entry:\n" + stmt->GenIR();
    }
};

class StmtAST : public BaseAST {
    public:
    int number;
    void Dump() const override {
        std::cout << "StmtAST { " << std::to_string(number) << " }";
    }
    std::string GenIR() const override {
        return "  ret " + std::to_string(number) + "\n";
    }
};

#endif
