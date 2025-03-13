#pragma once

#include <memory>

#include "visitor.hpp"
#include <unordered_map>

namespace AST {

class GenIRVisitor : public Visitor {
 public:
  std::unique_ptr<std::string> ir_code;
  std::unordered_map<std::string, int> const_sym_table;

  void visit(CompUnit& node) override;
  void visit(FuncDef& node) override;
  void visit(FuncType& node) override;

  void visit(ConstDecl& node) override;
  void visit(ConstDef& node) override;
  // void visit(VarDecl& node) override;
  // void visit(VarDef& node) override;
  // void visit(BType& node) override;

  void visit(RetStmt& node) override;
  void visit(AssignStmt& node) override;

  // virtual void visit(Exp& node) = 0;      // TODO: should exp be pure virtual?

  void visit(NumberExp& node) override;
  void visit(LValExp& node) override;
  void visit(NegativeExp& node) override;
  void visit(LogicalNotExp& node) override;
  void visit(AddExp& node) override;
  void visit(SubExp& node) override;
  void visit(MulExp& node) override;
  void visit(DivExp& node) override;
  void visit(ModExp& node) override;
  void visit(LTExp& node) override;
  void visit(GTExp& node) override;
  void visit(LEExp& node) override;
  void visit(GEExp& node) override;
  void visit(EQExp& node) override;
  void visit(NEExp& node) override;
  void visit(LAndExp& node) override;
  void visit(LOrExp& node) override;
};

}