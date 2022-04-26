%code requires {
  #include <memory>
  #include <string>
  #include <ast.h>
}
%glr-parser
%expect 2
%expect-rr 0

%{

#include <iostream>
#include <memory>
#include <string>
#include <ast.h>

// 声明 lexer 函数和错误处理函数
int yylex();

void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT UNARY_OP MUL_OP REL_OP EQ_OP OR_OP AND_OP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt PrimaryExp Exp UnaryExp AddExp MulExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDefStar ConstDef VarDefStar VarDef BlockItemStar BlockItem
%type <ast_val> FuncFParams FuncFParam FuncRParams CompUnitStar
%type <int_val> Number

%%

CompUnit
  : CompUnitStar {
    ast = unique_ptr<BaseAST>($1);
  }
  ;

CompUnitStar
  : FuncDef {
    auto ast = new CompUnitAST();
    ast->son.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | Decl {
    auto ast = new CompUnitAST();
    ast->son.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;    
  }
  | CompUnitStar FuncDef {
    $1->son.push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  | CompUnitStar Decl {
    $1->son.push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | FuncType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($6);
    ast->son.push_back(unique_ptr<BaseAST>($4));
    $$ = ast;
  }
  ;

FuncFParams
  : FuncFParam {
    auto ast = new FuncFParamsAST();
    ast->son.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | FuncFParams ',' FuncFParam {
    $1->son.push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }

FuncFParam
  : INT IDENT {
    auto ast = new FuncFParamAST();
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  }

FuncRParams
  : Exp {
    auto ast = new FuncRParamsAST();
    ast->son.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | FuncRParams ',' Exp {
    $1->son.push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = string("int");
    $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast->type = string("void");
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemStar '}' {
    auto ast = new BlockAST();
    ast->item = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BlockItemStar
  : BlockItemStar BlockItem {
    $1->son.push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  | {
    auto ast = new BlockItemStarAST();
    $$ = ast;
  }
  ;

BlockItem   
  : Decl {
    auto ast = new BlockItemAST();
    ast->state = 1;
    ast->item = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->state = 2;
    ast->item = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->state = 1;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT '=' Exp ';' {
    auto ast = new StmtAST();
    ast->state = 2;
    ast->var = *unique_ptr<string>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->state = 3;
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->state = 4;
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->state = 5;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->state = 6;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->state = 7;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->son.push_back(unique_ptr<BaseAST>($5));
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->state = 8;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->son.push_back(unique_ptr<BaseAST>($5));
    ast->son.push_back(unique_ptr<BaseAST>($7));
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->state = 9;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->son.push_back(unique_ptr<BaseAST>($5));
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast->state = 10;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast->state = 11;
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->state = 1;
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } 
  | UNARY_OP UnaryExp {
    auto ast = new UnaryExpAST();
    ast->state = 2;
    ast->op = *unique_ptr<string>($1);
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->state = 3;
    ast->op = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')' {
    auto ast = new UnaryExpAST();
    ast->state = 4;
    ast->op = *unique_ptr<string>($1);
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->tuple_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->state = 1;
    ast->item = unique_ptr<BaseAST>($2);
    $$ = ast;
  } 
  | Number {
    auto ast = new PrimaryExpAST();
    ast->state = 2;
    ast->var = to_string($1);
    $$ = ast;
  }
  | IDENT {
    auto ast = new PrimaryExpAST();
    ast->state = 3;
    ast->var = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new TupleExpAST();
    ast->state = 1;
    ast->dst = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp MUL_OP UnaryExp {
    auto ast = new TupleExpAST();
    ast->state = 2;
    ast->src = unique_ptr<BaseAST>($1);
    ast->op = *unique_ptr<string>($2);
    ast->dst = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new TupleExpAST();
    ast->state = 1;
    ast->dst = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp UNARY_OP MulExp {
    auto ast = new TupleExpAST();
    ast->state = 2;
    ast->src = unique_ptr<BaseAST>($1);
    ast->op = *unique_ptr<string>($2);
    ast->dst = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new AndOrAST();
    ast->state = 1;
    ast->dst = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp OR_OP LAndExp {
    auto ast = new AndOrAST();
    ast->state = 2;
    ast->src = unique_ptr<BaseAST>($1);
    ast->op = *unique_ptr<string>($2);
    ast->dst = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new AndOrAST();
    ast->state = 1;
    ast->dst = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp AND_OP EqExp {
    auto ast = new AndOrAST();
    ast->state = 2;
    ast->src = unique_ptr<BaseAST>($1);
    ast->op = *unique_ptr<string>($2);
    ast->dst = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new TupleExpAST();
    ast->state = 1;
    ast->dst = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQ_OP RelExp {
    auto ast = new TupleExpAST();
    ast->state = 2;
    ast->src = unique_ptr<BaseAST>($1);
    ast->op = *unique_ptr<string>($2);
    ast->dst = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
  
RelExp
  : AddExp {
    auto ast = new TupleExpAST();
    ast->state = 1;
    ast->dst = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp REL_OP AddExp {
    auto ast = new TupleExpAST();
    ast->state = 2;
    ast->src = unique_ptr<BaseAST>($1);
    ast->op = *unique_ptr<string>($2);
    ast->dst = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

Decl
  : CONST INT ConstDefStar ';' {
    auto ast = new DeclAST();
    ast->sub_decl = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | INT VarDefStar ';' {
    auto ast = new DeclAST();
    ast->sub_decl = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

ConstDefStar   
  : ConstDef {
    auto ast = new ConstDefStarAST();
    ast->son.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | ConstDefStar ',' ConstDef {
    $1->son.push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

ConstDef  
  : IDENT '=' Exp {
    auto ast = new ConstDefAST();
    ast->var = *unique_ptr<string>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

VarDefStar   
  : VarDef {
    auto ast = new VarDefStarAST();
    ast->son.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | VarDefStar ',' VarDef {
    $1->son.push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

VarDef  
  : IDENT '=' Exp {
    auto ast = new VarDefAST();
    ast->state = 1;
    ast->var = *unique_ptr<string>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT {
    auto ast = new VarDefAST();
    ast->state = 2;
    ast->var = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
