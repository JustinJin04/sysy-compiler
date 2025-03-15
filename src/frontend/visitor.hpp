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
class ExpStmt;     // inherit from Stmt
class BlockStmt;   // inherit from Stmt

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
  virtual void visit(CompUnit& node) {
    throw std::runtime_error("CompUnit is not implemented");
  }

  virtual void visit(FuncDef& node) {
    throw std::runtime_error("FuncDef is not implemented");
  }
  virtual void visit(FuncType& node) {
    throw std::runtime_error("FuncType is not implemented");
  }

  virtual void visit(ConstDecl& node) {
    throw std::runtime_error("ConstDecl is not implemented");
  }
  virtual void visit(ConstDef& node) {
    throw std::runtime_error("ConstDef is not implemented");
  }
  virtual void visit(VarDecl& node) {
    throw std::runtime_error("VarDecl is not implemented");
  }
  virtual void visit(VarDef& node) {
    throw std::runtime_error("VarDef is not implemented");
  }
  virtual void visit(BType& node) {
    throw std::runtime_error("BType is not implemented");
  }

  virtual void visit(RetStmt& node) {
    throw std::runtime_error("RetStmt is not implemented");
  }
  virtual void visit(AssignStmt& node) {
    throw std::runtime_error("AssignStmt is not implemented");
  }
  virtual void visit(ExpStmt& node) {
    throw std::runtime_error("ExpStmt is not implemented");
  }
  virtual void visit(BlockStmt& node) {
    throw std::runtime_error("BlockStmt is not implemented");
  }
  // virtual void visit(Exp& node) = 0;      // TODO: should exp be pure virtual?

  virtual void visit(NumberExp& node) {
    throw std::runtime_error("NumberExp is not implemented");
  }
  virtual void visit(LValExp& node) {
    throw std::runtime_error("LValExp is not implemented");
  }
  virtual void visit(NegativeExp& node) {
    throw std::runtime_error("NegativeExp is not implemented");
  }
  virtual void visit(LogicalNotExp& node) {
    throw std::runtime_error("LogicalNotExp is not implemented");
  }
  virtual void visit(AddExp& node) {
    throw std::runtime_error("AddExp is not implemented");
  }
  virtual void visit(SubExp& node) {
    throw std::runtime_error("SubExp is not implemented");
  }
  virtual void visit(MulExp& node) {
    throw std::runtime_error("MulExp is not implemented");
  }
  virtual void visit(DivExp& node) {
    throw std::runtime_error("DivExp is not implemented");
  }
  virtual void visit(ModExp& node) {
    throw std::runtime_error("ModExp is not implemented");
  }
  virtual void visit(LTExp& node) {
    throw std::runtime_error("LTExp is not implemented");
  }
  virtual void visit(GTExp& node) {
    throw std::runtime_error("GTExp is not implemented");
  }
  virtual void visit(LEExp& node) {
    throw std::runtime_error("LEExp is not implemented");
  }
  virtual void visit(GEExp& node) {
    throw std::runtime_error("GEExp is not implemented");
  }
  virtual void visit(EQExp& node) {
    throw std::runtime_error("EQExp is not implemented");
  }
  virtual void visit(NEExp& node) {
    throw std::runtime_error("NEExp is not implemented");
  }
  virtual void visit(LAndExp& node) {
    throw std::runtime_error("LAndExp is not implemented");
  }
  virtual void visit(LOrExp& node) {
    throw std::runtime_error("LOrExp is not implemented");
  }


};

}  // namespace AST