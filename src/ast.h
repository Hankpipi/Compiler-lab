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
  virtual void GenIR() const {}
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
    void GenIR() const override {
        func_def->GenIR();
        printf("\n");
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
    void GenIR() const override {
        std::cout << "fun @" << ident << "(): ";
        func_type->GenIR();
        std::cout << " {\n";
        block->GenIR();
        std::cout << "}\n";
    }
};

class FuncTypeAST : public BaseAST {
    public:
    std::string type;
    void Dump() const override {
        std::cout << "FuncTypeAST { " << type << " }";
    }
     void GenIR() const override {
        if (type == "int")
            std::cout << "i32";
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
    void GenIR() const override {
        std::cout << "%entry:\n";
        stmt->GenIR();
    }
};

class StmtAST : public BaseAST {
    public:
    int number;
    void Dump() const override {
        std::cout << "StmtAST { " << number << " }";
    }
    void GenIR() const override {
        std::cout << "  ret " << number << "\n";
    }
};

#endif
