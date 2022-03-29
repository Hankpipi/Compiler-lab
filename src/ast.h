#ifndef AST_H_
#define AST_H_
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <map>

// 所有 AST 的基类
class BaseAST {
 public:
  static int id;
  std::vector<std::unique_ptr<BaseAST>> son;
  virtual ~BaseAST() = default;
  virtual std::string GenIR() const {return "";}
  virtual int calc() const { return 0;}
};

class CompUnitAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_def;
    std::string GenIR() const override;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    std::string GenIR() const override;
};

class FuncTypeAST : public BaseAST {
    public:
    std::string type;
    std::string GenIR() const override;
};

class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> item;
    std::string GenIR() const override;
};

class BlockItemStarAST : public BaseAST {
    public:
    std::string GenIR() const override;
};

class BlockItemAST : public BaseAST {
    public:
    int state;
    std::unique_ptr<BaseAST> item;
    std::string GenIR() const override;
};

class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::string GenIR() const override;
};

class ExpAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> tuple_exp;
    std::string GenIR() const override;
    int calc() const override;
};

class PrimaryExpAST : public BaseAST {
 public:
    int state;
    std::string var;
    std::unique_ptr<BaseAST> item;
    std::string GenIR() const override;
    int calc() const override;
};

class UnaryExpAST : public BaseAST {
 public:
    int state, number;
    std::string op;
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_exp;
    std::string GenIR() const override;
    int calc() const override; 
};

class TupleExpAST : public BaseAST {
 public:
    int state;
    std::string op;
    std::unique_ptr<BaseAST> src;
    std::unique_ptr<BaseAST> dst;
    std::string GenIR() const override;
    int calc() const override;
};

class DeclAST : public BaseAST {
 public:
    int state;
    std::unique_ptr<BaseAST> sub_decl;
    std::string GenIR() const override;
};

class ConstDeclAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> def;
    std::string GenIR() const override;
};

class ConstDefStarAST : public BaseAST {
 public:
    std::string GenIR() const override;
};

class ConstDefAST : public BaseAST {
 public:
    std::string var;
    std::unique_ptr<BaseAST> exp;
    std::string GenIR() const override;
};

#endif
