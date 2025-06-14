#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "visitor.hpp"

namespace AST {
class Base;  // Base class for all AST nodes
class CompUnit;
// class CompUnitRoot;  // we use this class to represent the root of AST
//                      // since CompUnit is reserved by bison
class CompUnitItem;  // property of CompUnit
class FuncDef;       // inherit from CompUnitItem
// class FuncType;      // property of FuncDef
class Type;
class FuncFParam;
class FuncCallExp;
class BlockItem;  // property of Block

class Decl;       // inherit from BlockItem (local declaration)
class ConstDecl;  // inherit from Decl
class VarDecl;    // inherit from Decl
// class BType;      // property of Decl, FuncFParam
class ConstDef;  // property of ConstDecl
class VarDef;    // property of VarDecl

// class GlobalDecl; // inherit from CompUnitItem (global declaration)
// class GlobalConstDecl;  // inherit from GlobalDecl
// class GlobalVarDecl;    // inherit from GlobalDecl
// class GlobalConstDef;
// class GlobalVarDef;
class ArrayInitVal;
class ArrayDims;
class FuncFParamArr;

class Stmt;          // inherit from BlockItem
class RetStmt;       // inherit from Stmt
class AssignStmt;    // inherit from Stmt
class ExpStmt;       // inherit from Stmt
class BlockStmt;     // inherit from Stmt
class IfStmt;        // inherit from Stmt
class WhileStmt;     // inherit from Stmt
class BreakStmt;     // inherit from Stmt
class ContinueStmt;  // inherit from Stmt

class Exp;            // Base class for all expressions
class NumberExp;      // inherit from Exp
class LValExp;        // inherit from Exp
class UnaryExp;       // Base class for all unary expressions
class NegativeExp;    // inherit from UnaryExp
class LogicalNotExp;  // inherit from UnaryExp
class BinaryExp;      // Base class for all binary expressions
class AddExp;         // inherit from BinaryExp
class SubExp;         // inherit from BinaryExp
class MulExp;         // inherit from BinaryExp
class DivExp;         // inherit from BinaryExp
class ModExp;         // inherit from BinaryExp
class LTExp;          // inherit from BinaryExp
class GTExp;          // inherit from BinaryExp
class LEExp;          // inherit from BinaryExp
class GEExp;          // inherit from BinaryExp
class EQExp;          // inherit from BinaryExp
class NEExp;          // inherit from BinaryExp
class LAndExp;        // inherit from BinaryExp
class LOrExp;         // inherit from BinaryExp

class Visitor;

class Base {
 public:
  virtual ~Base() = default;

  virtual void accept(Visitor& v) = 0;
};

class CompUnit : public Base {
 public:
  std::unique_ptr<CompUnitItem> item;  // a list of items

  void accept(Visitor& v) override;
};

class CompUnitItem : virtual public Base {
 public:
  std::unique_ptr<CompUnitItem> next_compunit_item = nullptr;

  virtual void accept(Visitor& v) = 0;
};

class FuncFParam : public Base {
 public:
  // std::unique_ptr<BType> btype;
  std::unique_ptr<Type> btype;
  std::string ident;
  std::unique_ptr<FuncFParam> next_func_fparam = nullptr;

  virtual void accept(Visitor& v) override;
};

class FuncFParamArr : public FuncFParam {
 public:
  /**
   * for examples:
   * int a[]: array_dims = nullptr
   * int a[][2]: array_dims = {2}
   */
  std::unique_ptr<ArrayDims> array_dims = nullptr;

  void accept(Visitor& v) override;
};

class FuncDef : public CompUnitItem {
 public:
  // std::unique_ptr<FuncType> func_type;
  std::unique_ptr<Type> func_type;
  std::string ident;  // TOKEN name, also a symbol in symbol table to be
                      // inserted when this ast node finished construction
  std::unique_ptr<FuncFParam> func_fparam;  // a list of func_fparam

  std::unique_ptr<BlockItem> block_item;  // a list of block_item

  void accept(Visitor& v) override;
};

// class FuncType : public Base {
//  public:
//   std::string type_name;

//   void accept(Visitor& v) override;
// };

class BlockItem : virtual public Base {
 public:
  std::unique_ptr<BlockItem> next_block_item = nullptr;

  virtual void accept(Visitor& v) = 0;
};

class Decl : public BlockItem, public CompUnitItem {
 public:
  // std::unique_ptr<BType> btype;
  std::unique_ptr<Type> btype;

  virtual void accept(Visitor& v) = 0;
};

class ConstDecl : public Decl {
 public:
  std::unique_ptr<ConstDef> const_def;  // a list of const_def

  void accept(Visitor& v) override;
};

class ArrayInitVal : public Base {
 public:
  /**
   * 1. exp=nullptr, array_init_val_hierarchy.size() = 0 means zeroinit
   * 2. exp=nullptr, array_init_val_hierarchy.size() > 0 means array init
   * 3. exp!=nullptr, array_init_val_hierarchy.size() = 0 means a leafnode
   */

  std::unique_ptr<Exp> exp = nullptr;

  std::vector<std::unique_ptr<ArrayInitVal>> array_init_val_hierarchy;

  void accept(Visitor& v) override;
};

class ArrayDims : public Base {
 public:
  std::unique_ptr<Exp> exp;
  std::unique_ptr<ArrayDims> next_dim = nullptr;

  void accept(Visitor& v) override;
};

class ConstDef : public Base {
 public:
  std::string ident;  // TOKEN name, should be inserted into symbol table
                      // When we later construct LValExp node, we check if the
                      // ident is in symbol table
  // For example:
  // int main () {
  //   const int a = 1 + 1;         // here 'a' is a TOKEN name. When we parse
  //   it, we insert <a, 2> into symble table return a;                    //
  //   here 'a' is a LValExp. We check if 'a' is in symbol table (with value
  //   computed)
  // }
  std::unique_ptr<ConstDef> next_const_def = nullptr;
  bool is_global = false;

  /**
   * a list of array_dim
   * for example:
   * const int a[2][3] = {{1, 2, 3}, {4, 5, 6}};
   * array_dims = {2, 3}
   * array_init_val = {{1, 2, 3}, {4, 5, 6}}
   */
  std::unique_ptr<ArrayDims> array_dims;
  std::unique_ptr<ArrayInitVal> const_init_val = nullptr;

  void accept(Visitor& v) override;
};

class VarDecl : public Decl {
 public:
  std::unique_ptr<VarDef> var_def;  // a list of var_def

  void accept(Visitor& v) override;
};

class VarDef : public Base {
 public:
  std::string ident;  // TOKEN name, should be inserted into symbol table
  // std::unique_ptr<Exp> var_init_val = nullptr;
  std::unique_ptr<VarDef> next_var_def = nullptr;
  bool is_global = false;

  std::unique_ptr<ArrayDims> array_dims = nullptr;
  std::unique_ptr<ArrayInitVal> var_init_val = nullptr;

  void accept(Visitor& v) override;
};

// class BType : public Base {
//  public:
//   std::string type_name;

//   void accept(Visitor& v) override;
// };
class Type : public Base {
 public:
  std::string type_name;

  void accept(Visitor& v) override;
};

/**
 * Stmt abstract class defination
 * Followed two inherited classes:
 * 1. RetStmt (include w/ and w/o exp)
 * 2. AssignStmt
 * 3. ExpStmt (including w/ and w/o exp)
 * 4. BlockStmt
 * 5. IfStmt
 */
class Stmt : public BlockItem {
 public:
  // void accept(Visitor& v) override {
  //   v.visit(*this);
  // }
  virtual void accept(Visitor& v) = 0;
};

class RetStmt : public Stmt {
 public:
  std::unique_ptr<Exp> exp;

  void accept(Visitor& v) override;
};

class AssignStmt : public Stmt {
 public:
  std::unique_ptr<LValExp> lval;
  std::unique_ptr<Exp> exp;
  void accept(Visitor& v) override;
};

class ExpStmt : public Stmt {
 public:
  std::unique_ptr<Exp> exp;
  void accept(Visitor& v) override;
};

class BlockStmt : public Stmt {
 public:
  std::unique_ptr<BlockItem> block_item;  // a list of block_item
  void accept(Visitor& v) override;
};

class IfStmt : public Stmt {
 public:
  std::unique_ptr<Exp> cond;
  std::unique_ptr<Stmt> then_body;
  std::unique_ptr<Stmt> else_body;

  void accept(Visitor& v) override;
};

class WhileStmt : public Stmt {
 public:
  std::unique_ptr<Exp> cond;
  std::unique_ptr<Stmt> body;

  void accept(Visitor& v) override;
};

class BreakStmt : public Stmt {
 public:
  void accept(Visitor& v) override;
};

class ContinueStmt : public Stmt {
 public:
  void accept(Visitor& v) override;
};

/**
 * Exp abstract class defination
 * Followed four inherited classes:
 * 1. NumberExp
 * 2. LValExp
 * 3. UnaryExp
 * 4. BinaryExp
 */
class Exp : public Base {
 public:
  std::unique_ptr<Exp> next_func_rparam =
      nullptr;  // used for func_call parameter list
  virtual void accept(Visitor& v) = 0;
};

class NumberExp : public Exp {
 public:
  int number;

  void accept(Visitor& v) override;
};

class LValExp : public Exp {
 public:
  std::string ident;  // TOKEN name, used to find the symbol in symbol table

  std::unique_ptr<ArrayDims> array_dims = nullptr;  // a list of array_dim
  void accept(Visitor& v) override;
};

class FuncCallExp : public Exp {
 public:
  std::string ident;
  // std::vector<std::unique_ptr<Exp>> args;
  std::unique_ptr<Exp> rparam;  // a list of args (Exp)
  void accept(Visitor& v) override;
};

/**
 * UnaryExp abstract class defination
 * Followed two inherited classes:
 * 1. NegativeExp
 * 2. LogicalNotExp
 */
class UnaryExp : public Exp {
 public:
  std::unique_ptr<Exp> operand;

  void accept(Visitor& v) override;
};

class NegativeExp : public UnaryExp {
 public:
  void accept(Visitor& v) override;
};

class LogicalNotExp : public UnaryExp {
 public:
  void accept(Visitor& v) override;
};

/**
 * BinaryExp abstract class defination
 * Followed 13 inherited classes:
 * 1. AddExp
 * 2. SubExp
 * 3. MulExp
 * 4. DivExp
 * 5. ModExp
 * 6. LTExp
 * 7. GTExp
 * 8. LEExp
 * 9. GEExp
 * 10. EQExp
 * 11. NEExp
 * 12. LAndExp
 * 13. LOrExp
 */
class BinaryExp : public Exp {
 public:
  std::unique_ptr<Exp> lhs;
  std::unique_ptr<Exp> rhs;

  void accept(Visitor& v) override;
};

class AddExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class SubExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class MulExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class DivExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class ModExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class LTExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class GTExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class LEExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class GEExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class EQExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class NEExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class LAndExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

class LOrExp : public BinaryExp {
 public:
  void accept(Visitor& v) override;
};

}  // namespace AST