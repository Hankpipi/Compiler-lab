#ifndef AST_H_
#define AST_H_
#include <iostream>
#include <memory>
#include <map>

// 所有 AST 的基类
class BaseAST {
 public:
  static int id;
  virtual ~BaseAST() = default;
  virtual void Dump() const {}
  virtual std::string GenIR() const {}
};

class CompUnitAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_def;
    void Dump() const override;
    std::string GenIR() const override;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump() const override;
    std::string GenIR() const override;
};

class FuncTypeAST : public BaseAST {
    public:
    std::string type;
    void Dump() const override;
    std::string GenIR() const override;
};

class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override;
    std::string GenIR() const override;
};

class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    void Dump() const override;
    std::string GenIR() const override;
};

class ExpAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> unary_exp;
    void Dump() const override;
    std::string GenIR() const override;
};

class PrimaryExpAST : public BaseAST {
 public:
    int state;
    int number;
    std::unique_ptr<BaseAST> exp;
    void Dump() const override;
    std::string GenIR() const override;
};

class UnaryExpAST : public BaseAST {
 public:
    int state;
    int number;
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    void Dump() const override;
    std::string GenIR() const override;
};

class UnaryOpAST : public BaseAST {
 public:
    std::string op;
    void Dump() const override;
    std::string GenIR() const override;
};

#endif
