#ifndef AST_H_
#define AST_H_
#include <blockinfo.h>
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <map>

// 所有 AST 的基类
class BaseAST {
 public:
  static int id, elf_id, block_id;
  int state;
  std::string ident;
  std::vector<std::unique_ptr<BaseAST>> son;
  std::vector<std::string>items; // save array items
  std::vector<int> shape;
  virtual ~BaseAST() = default;
  virtual std::string GenIR(BlockInfo*) {return "";}
  virtual std::string GenIR(BlockInfo*, int) {return "";}
  virtual int calc(BlockInfo*) const { return 0;}
  virtual vector<std::string> getson(BlockInfo*) const { 
      return vector<std::string>();
  }
};

class CompUnitAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class FuncTypeAST : public BaseAST {
    public:
    std::string type;
    std::string GenIR(BlockInfo*) override;
};

class FuncFParamAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class FuncFParamsAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
    vector<std::string> getson(BlockInfo* b) const override;
};

class FuncRParamsAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class FuncDefAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> func_type;
    std::unique_ptr<BaseAST> block;
    std::string GenIR(BlockInfo*) override;
};

class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> item;
    std::string GenIR(BlockInfo*) override;
};

class BlockItemStarAST : public BaseAST {
    public:
    std::string GenIR(BlockInfo*) override;
};

class BlockItemAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> item;
    std::string GenIR(BlockInfo*) override;
};

class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::string GenIR(BlockInfo*) override;
};

class ExpAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> tuple_exp;
    std::string GenIR(BlockInfo*) override;
    int calc(BlockInfo*) const override;
};

class LValAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class PrimaryExpAST : public BaseAST {
 public:
    std::string var;
    std::unique_ptr<BaseAST> item;
    std::unique_ptr<LValAST> exp;
    std::string GenIR(BlockInfo*) override;
    int calc(BlockInfo*) const override;
};

class UnaryExpAST : public BaseAST {
 public:
    int number;
    std::string op;
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_exp;
    std::string GenIR(BlockInfo*) override;
    int calc(BlockInfo*) const override; 
};

class TupleExpAST : public BaseAST {
 public:
    std::string op;
    std::unique_ptr<BaseAST> src;
    std::unique_ptr<BaseAST> dst;
    std::string GenIR(BlockInfo*) override;
    int calc(BlockInfo*) const override;
};

class AndOrAST : public BaseAST {
 public:
    std::string op;
    std::unique_ptr<BaseAST> src;
    std::unique_ptr<BaseAST> dst;
    std::string GenIR(BlockInfo*) override;
    int calc(BlockInfo*) const override;
};

class DeclAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> sub_decl;
    std::string GenIR(BlockInfo*) override;
};

class ConstDefStarAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class DefAST : public BaseAST {
 public:
    std::string var;
    std::unique_ptr<BaseAST> exp;
    std::string GenAggragate(int, int, int);
    void ArrayInit(int, int, int, std::string);
};

class ConstDefAST : public DefAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class VarDefStarAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class VarDefAST : public DefAST {
 public:
    std::string GenIR(BlockInfo*) override;
};

class InitValAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*, int) override;
};

class ConstInitValAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*, int) override;
};

class InitValStarAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*, int) override;
};

class ConstExpAST : public BaseAST {
 public:
    std::string GenIR(BlockInfo*) override;
    std::string GenArrayDef(BlockInfo* b, int dep) const;
};

#endif
