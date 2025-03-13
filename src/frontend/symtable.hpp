#pragma once
// serve as symbol table constuctor
#include <iostream>
#include <string>
#include <unordered_map>

#include "eval.hpp"
#include "visitor.hpp"

namespace AST {

class SymTableVisitor : public Visitor {
 public:
  std::unordered_map<std::string, int> const_sym_table;

  /**
   * Warning:
   * workaround: here we don't let funcdef to recursively visit subsequent
   * compunititem
   */
  void visit(CompUnit& node) override {
    auto item_ptr = node.item.get();
    while (item_ptr) {
      item_ptr->accept(*this);
      item_ptr = item_ptr->next_compunit_item.get();
    }
  }

  /**
   * Warning:
   * workaround: here we don't let funcdef to recursively visit subsequent
   * compunititem
   */
  void visit(FuncDef& node) override { node.block_item->accept(*this); }

  void visit(ConstDecl& node) override {
    auto const_def_ptr = node.const_def.get();
    while (const_def_ptr) {
      const_def_ptr->accept(*this);
      const_def_ptr = const_def_ptr->next_const_def.get();
    }
  }

  void visit(ConstDef& node) override {
    auto evaluate_visitor = EvaluateVisitor();
    node.const_init_val->accept(evaluate_visitor);
    const_sym_table[node.ident] = evaluate_visitor.result;
  }

  void visit(AssignStmt& node) override {
    // TODO
    auto evaluate_visitor = EvaluateVisitor();
    node.exp->accept(evaluate_visitor);
    const_sym_table[node.lval->ident] = evaluate_visitor.result;
  }

  // TODO: following functions shouldn't be called
  // Implement them to avoid compile error
  void visit(FuncType& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(RetStmt& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(NumberExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LValExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(NegativeExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LogicalNotExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(AddExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(SubExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(MulExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(DivExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(ModExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LTExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(GTExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LEExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(GEExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(EQExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(NEExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LAndExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LOrExp& node) override {
    throw std::runtime_error("raise not implemented error");
  }
};

}  // namespace AST