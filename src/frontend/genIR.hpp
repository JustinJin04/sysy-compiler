#pragma once

#include <memory>

#include "visitor.hpp"
#include <unordered_map>
#include <variant>
#include <stack>
#include "symtable.hpp"
#include "prune.hpp"

namespace AST {


class GenIRVisitor : public Visitor {
 public:
  std::unique_ptr<std::string> ir_code;
  // int refers to const symbol, while string refers to variable symbol
  // std::unordered_map<std::string, std::variant<int, std::string>> sym_table;
  SymbolTables sym_table_stack;

 private:
  int tempCounter = 0;
  std::stack<std::string> tempCounterSt;

  std::string get_new_counter() {
    int retval = tempCounter;
    tempCounter += 1;
    return "%" + std::to_string(retval);
  }
  void push_result(std::string result) {
    tempCounterSt.push(result);
  }
  std::string pop_last_result() {
    std::string ret = tempCounterSt.top();
    tempCounterSt.pop();
    return ret;
  }

    // used to add label after ret
  // int ret_label_counter = 0;
  // used to add label before each basic block
  int block_label_counter = 0;

 public:

  void visit(CompUnit& node) override;
  void visit(FuncDef& node) override;
  void visit(FuncType& node) override;

  void visit(ConstDecl& node) override;
  void visit(ConstDef& node) override;
  void visit(VarDecl& node) override;
  void visit(VarDef& node) override;
  // void visit(BType& node) override;

  void visit(RetStmt& node) override;
  void visit(AssignStmt& node) override;
  void visit(ExpStmt& node) override;
  void visit(BlockStmt& node) override;

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