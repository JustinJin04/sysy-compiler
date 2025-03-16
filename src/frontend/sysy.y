%code requires {
  #include <memory>
  #include <string>
  #include <ast.hpp>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <ast.hpp>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<AST::CompUnit> &ast, const char *s);

using namespace std;


// helper function for downcasting and wrap with unique_ptr
template<typename TARGET>
std::unique_ptr<TARGET> cast_uptr(AST::Base* base) {
  TARGET* target = dynamic_cast<TARGET*>(base);
  if (target == nullptr) {
    throw std::runtime_error("cast_uptr failed");
  }
  return std::unique_ptr<TARGET>(target);
}

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<AST::CompUnit> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  AST::Base* ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> Decl ConstDecl BType ConstDef ConstInitVal VarDecl VarDef InitVal
%type <ast_val> FuncDef FuncType
%type <ast_val> Block BlockItem Stmt
%type <ast_val> Exp LVal PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp ConstExp
%type <int_val> Number
%type <ast_val> BlockItemList ConstDefList VarDefList
%type <ast_val> MatchedStmt OpenStmt



%%

CompUnit
  : FuncDef {
    auto comp_unit_ast = std::make_unique<AST::CompUnit>();
    comp_unit_ast->item = cast_uptr<AST::FuncDef>($1);
    ast = std::move(comp_unit_ast);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto func_def_ast = new AST::FuncDef();
    func_def_ast->func_type = cast_uptr<AST::FuncType>($1);
    func_def_ast->ident = *std::unique_ptr<std::string>($2);
    if($5){
      func_def_ast->block_item = cast_uptr<AST::BlockItem>($5);    // since we don't have block, we use block item instead
    } else {
      func_def_ast->block_item = nullptr;
    }
    $$ = func_def_ast;
  }
  ;

FuncType
  : INT {
    auto func_type_ast = new AST::FuncType();
    func_type_ast->type_name = "int";
    $$ = func_type_ast;  
  }
  ;

Block
  : '{' BlockItemList '}' {// Note that BlockItemList is a link list of type BlockItem
    $$ = $2;
  }
  | '{' '}' {
    $$ = nullptr;
  }
  ;
BlockItemList
  : BlockItem {
    $$ = $1;
  }
  | BlockItem BlockItemList {
    // BUG1: here we have to return a ptr without unique_ptr wrapper
    // Otherwise if we call dynamic_cast to a unique_ptr, it will cause segmentation fault
    // BUG2: order of them matters a lot. Please check
    dynamic_cast<AST::BlockItem*>($1)->next_block_item = cast_uptr<AST::BlockItem>($2);
    $$ = $1;
  }
  ;
BlockItem
  : Stmt {
    $$ = $1;
  }
  | Decl {
    $$ = $1;
  }
  ;

Decl
  : ConstDecl {
    $$ = $1;
  }
  | VarDecl {
    $$ = $1;
  }
  ;
ConstDecl
  : CONST BType ConstDefList ';' { // Note that ConstDefList is a link list of type ConstDef
    auto const_decl_ast = new AST::ConstDecl();
    const_decl_ast->btype = cast_uptr<AST::BType>($2);
    const_decl_ast->const_def = cast_uptr<AST::ConstDef>($3);
    $$ = const_decl_ast;
  }
  ;
ConstDefList
  : ConstDef {
    $$ = $1;
  }
  | ConstDef ',' ConstDefList {
    dynamic_cast<AST::ConstDef*>($1)->next_const_def = cast_uptr<AST::ConstDef>($3);
    $$ = $1;
  }
  ;
ConstDef
  : IDENT '=' ConstInitVal {
    auto const_def_ast = new AST::ConstDef();
    const_def_ast->ident = *std::unique_ptr<std::string>($1);
    const_def_ast->const_init_val = cast_uptr<AST::Exp>($3);   // We don't differentiate between const and var here 
                                                               // It's the visitor's duty to check wether it's a const
                                                               // and add to symbol table
    $$ = const_def_ast;
  }
  ;
ConstInitVal
  : ConstExp {
    $$ = $1;
  }
  ;
ConstExp
  : Exp {
    $$ = $1;
  }
VarDecl
  : BType VarDefList ';' {
    auto var_decl_ast = new AST::VarDecl();
    var_decl_ast->btype = cast_uptr<AST::BType>($1);
    var_decl_ast->var_def = cast_uptr<AST::VarDef>($2);
    $$ = var_decl_ast;
  }
  ;
VarDefList
  : VarDef {
    $$ = $1;
  }
  | VarDef ',' VarDefList {
    dynamic_cast<AST::VarDef*>($1)->next_var_def = cast_uptr<AST::VarDef>($3);
    $$ = $1;
  }
  ;
VarDef
  : IDENT {
    auto var_def_ast = new AST::VarDef();
    var_def_ast->ident = *std::unique_ptr<std::string>($1);
    $$ = var_def_ast;
  }
  | IDENT '=' InitVal {
    auto var_def_ast = new AST::VarDef();
    var_def_ast->ident = *std::unique_ptr<std::string>($1);
    var_def_ast->var_init_val = cast_uptr<AST::Exp>($3);
    $$ = var_def_ast;
  }
  ;
InitVal
  : Exp {
    $$ = $1;
  }
  ;
BType
  : INT {
    auto btype_ast = new AST::BType();
    btype_ast->type_name = "int";
    $$ = btype_ast;
  }
  ;

Stmt
  : MatchedStmt {
    $$ = $1;
  }
  | OpenStmt {
    $$ = $1;
  }
  ;

MatchedStmt
  : IF '(' Exp ')' MatchedStmt ELSE MatchedStmt {
    auto if_stmt_ast = new AST::IfStmt();
    if_stmt_ast->cond = cast_uptr<AST::Exp>($3);
    if_stmt_ast->then_body = cast_uptr<AST::Stmt>($5);
    if_stmt_ast->else_body = cast_uptr<AST::Stmt>($7);
    $$ = if_stmt_ast;
  }
  | LVal '=' Exp ';' {
    auto assign_stmt_ast = new AST::AssignStmt();
    assign_stmt_ast->lval = cast_uptr<AST::LValExp>($1);
    assign_stmt_ast->exp = cast_uptr<AST::Exp>($3);
    $$ = assign_stmt_ast;
  }
  | RETURN Exp ';' {
    auto ret_stmt_ast = new AST::RetStmt();
    ret_stmt_ast->exp = cast_uptr<AST::Exp>($2);
    $$ = ret_stmt_ast;
  }
  | RETURN ';' {
    auto ret_stmt_ast = new AST::RetStmt();
    ret_stmt_ast->exp = nullptr;
    $$ = ret_stmt_ast;
  }
  | Block { // block statement
    auto block_stmt_ast = new AST::BlockStmt();
    if($1){
      block_stmt_ast->block_item = cast_uptr<AST::BlockItem>($1);
    } else {
      block_stmt_ast->block_item = nullptr;
    }
    $$ = block_stmt_ast;
  }
  | ';' { // empty exp statement
    auto exp_stmt_ast = new AST::ExpStmt();
    exp_stmt_ast->exp = nullptr;
    $$ = exp_stmt_ast;
  }
  | Exp ';' {// non-empty exp statement
    auto exp_stmt_ast = new AST::ExpStmt();
    exp_stmt_ast->exp = cast_uptr<AST::Exp>($1);
    $$ = exp_stmt_ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto while_stmt_ast = new AST::WhileStmt();
    while_stmt_ast->cond = cast_uptr<AST::Exp>($3);
    while_stmt_ast->body = cast_uptr<AST::Stmt>($5);
    $$ = while_stmt_ast;
  }
  | BREAK ';' {
    auto break_stmt_ast = new AST::BreakStmt();
    $$ = break_stmt_ast;
  }
  | CONTINUE ';' {
    auto continue_stmt_ast = new AST::ContinueStmt();
    $$ = continue_stmt_ast;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    auto if_stmt_ast = new AST::IfStmt();
    if_stmt_ast->cond = cast_uptr<AST::Exp>($3);
    if_stmt_ast->then_body = cast_uptr<AST::Stmt>($5);
    if_stmt_ast->else_body = nullptr;
    $$ = if_stmt_ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE OpenStmt {
    auto if_stmt_ast = new AST::IfStmt();
    if_stmt_ast->cond = cast_uptr<AST::Exp>($3);
    if_stmt_ast->then_body = cast_uptr<AST::Stmt>($5);
    if_stmt_ast->else_body = cast_uptr<AST::Stmt>($7);
    $$ = if_stmt_ast;
  }
  ;

Exp
  : LOrExp {
    $$ = $1;
  }
  ;
LOrExp
  : LAndExp {
    $$ = $1;
  }
  | LOrExp '|' '|' LAndExp {
    auto lor_exp_ast = new AST::LOrExp();
    lor_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    lor_exp_ast->rhs = cast_uptr<AST::Exp>($4);
    $$ = lor_exp_ast;
  }
  ;
LAndExp
  : EqExp {
    $$ = $1;
  }
  | LAndExp '&' '&' EqExp {
    auto land_exp_ast = new AST::LAndExp();
    land_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    land_exp_ast->rhs = cast_uptr<AST::Exp>($4);
    $$ = land_exp_ast;
  }
  ;
EqExp
  : RelExp {
    $$ = $1;
  }
  | EqExp '=' '=' RelExp {
    auto eq_exp_ast = new AST::EQExp();
    eq_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    eq_exp_ast->rhs = cast_uptr<AST::Exp>($4);
    $$ = eq_exp_ast;
  }
  | EqExp '!' '=' RelExp {
    auto ne_exp_ast = new AST::NEExp();
    ne_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    ne_exp_ast->rhs = cast_uptr<AST::Exp>($4);
    $$ = ne_exp_ast;
  }
  ;
RelExp
  : AddExp {
    $$ = $1;
  }
  | RelExp '<' AddExp {
    auto lt_exp_ast = new AST::LTExp();
    lt_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    lt_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = lt_exp_ast;
  }
  | RelExp '>' AddExp {
    auto gt_exp_ast = new AST::GTExp();
    gt_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    gt_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = gt_exp_ast;
  }
  | RelExp '<' '=' AddExp {
    auto le_exp_ast = new AST::LEExp();
    le_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    le_exp_ast->rhs = cast_uptr<AST::Exp>($4);
    $$ = le_exp_ast;
  }
  | RelExp '>' '=' AddExp {
    auto ge_exp_ast = new AST::GEExp();
    ge_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    ge_exp_ast->rhs = cast_uptr<AST::Exp>($4);
    $$ = ge_exp_ast;
  }
  ;
AddExp
  : MulExp {
    $$ = $1;
  }
  | AddExp '+' MulExp {
    auto add_exp_ast = new AST::AddExp();
    add_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    add_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = add_exp_ast;
  }
  | AddExp '-' MulExp {
    auto sub_exp_ast = new AST::SubExp();
    sub_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    sub_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = sub_exp_ast;
  }
  ;
MulExp
  : UnaryExp {
    $$ = $1;
  }
  | MulExp '*' UnaryExp {
    auto mul_exp_ast = new AST::MulExp();
    mul_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    mul_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = mul_exp_ast;
  }
  | MulExp '/' UnaryExp {
    auto div_exp_ast = new AST::DivExp();
    div_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    div_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = div_exp_ast;
  }
  | MulExp '%' UnaryExp {
    auto mod_exp_ast = new AST::ModExp();
    mod_exp_ast->lhs = cast_uptr<AST::Exp>($1);
    mod_exp_ast->rhs = cast_uptr<AST::Exp>($3);
    $$ = mod_exp_ast;
  }
  ;
UnaryExp
  : PrimaryExp {
    $$ = $1;
  }
  | '+' UnaryExp {
    $$ = $2;
  }
  | '-' UnaryExp {
    auto neg_exp_ast = new AST::NegativeExp();
    neg_exp_ast->operand = cast_uptr<AST::Exp>($2);
    $$ = neg_exp_ast;
  }
  | '!' UnaryExp {
    auto not_exp_ast = new AST::LogicalNotExp();
    not_exp_ast->operand = cast_uptr<AST::Exp>($2);
    $$ = not_exp_ast;
  }
  ;
PrimaryExp
  : '(' Exp ')' {
    $$ = $2;
  }
  | LVal {
    $$ = $1;
  }
  | Number {
    auto num_exp_ast = new AST::NumberExp();
    num_exp_ast->number = $1;
    $$ = num_exp_ast;
  }
  ;
LVal
  : IDENT {
    auto lval_exp_ast = new AST::LValExp();
    lval_exp_ast->ident = *std::unique_ptr<std::string>($1);
    $$ = lval_exp_ast;
  }
  ;
Number
  : INT_CONST {
    $$ = $1;
  }
  ;



%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<AST::CompUnit> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
