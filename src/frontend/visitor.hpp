#pragma once
#include "ast.hpp"

namespace AST {
class Base;  // Base class for all AST nodes
class CompUnit;
class CompUnitItem;  // property of CompUnit
class FuncDef;       // inherit from CompUnitItem
class FuncType;      // property of FuncDef
class BlockItem;     // property of Block

class Decl;       // inherit from CompUnitItem and BlockItem
class ConstDecl;  // inherit from Decl
class VarDecl;    // inherit from Decl
class BType;      // property of Decl, FuncFParam
class ConstDef;   // property of ConstDecl
class VarDef;     // property of VarDecl

class Stmt;        // inherit from BlockItem
class RetStmt;     // inherit from Stmt
class AssignStmt;  // inherit from Stmt

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


class Visitor {
 public:
  virtual ~Visitor() = default;
  virtual void visit(CompUnit& node) = 0;

  virtual void visit(FuncDef& node) = 0;
  virtual void visit(FuncType& node) = 0;

  // virtual void visit(ConstDecl& node) = 0;
  // virtual void visit(ConstDef& node) = 0;
  // virtual void visit(VarDecl& node) = 0;
  // virtual void visit(VarDef& node) = 0;
  // virtual void visit(BType& node) = 0;

  virtual void visit(RetStmt& node) = 0;
  // virtual void visit(AssignStmt& node) = 0;

  // virtual void visit(Exp& node) = 0;      // TODO: should exp be pure virtual?

  virtual void visit(NumberExp& node) = 0;
  // virtual void visit(LValExp& node) = 0;
  virtual void visit(NegativeExp& node) = 0;
  virtual void visit(LogicalNotExp& node) = 0;
  virtual void visit(AddExp& node) = 0;
  virtual void visit(SubExp& node) = 0;
  virtual void visit(MulExp& node) = 0;
  virtual void visit(DivExp& node) = 0;
  virtual void visit(ModExp& node) = 0;
  virtual void visit(LTExp& node) = 0;
  virtual void visit(GTExp& node) = 0;
  virtual void visit(LEExp& node) = 0;
  virtual void visit(GEExp& node) = 0;
  virtual void visit(EQExp& node) = 0;
  virtual void visit(NEExp& node) = 0;
  virtual void visit(LAndExp& node) = 0;
  virtual void visit(LOrExp& node) = 0;



  // only used for debug stmt
  void visit(ConstDecl& node) {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(ConstDef& node) {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(VarDecl& node) {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(VarDef& node) {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(BType& node) {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(AssignStmt& node) {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(Exp& node ) {
    std::cout<<"visit Exp"<<std::endl;
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LValExp& node) {
    throw std::runtime_error("raise not implemented error");
  }


};

}  // namespace AST