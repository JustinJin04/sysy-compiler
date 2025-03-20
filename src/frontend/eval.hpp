#pragma once

#include "visitor.hpp"
#include <unordered_map>
#include <variant>
#include "symtable.hpp"

namespace AST {

class EvaluateVisitor : public Visitor {
 public:
  int result = 0;
  // std::unordered_map<std::string, std::variant<int, std::string>> sym_table;
  SymbolTables* sym_table_stack;

  EvaluateVisitor(SymbolTables* other_sym_table=nullptr){
    sym_table_stack = other_sym_table;
  }

  // Currently only support evaluate expressions
  // later will support evaluate consteval and vareval
  void visit(NumberExp& node) override {
    result = node.number;
  }
  void visit(NegativeExp& node) override {
    node.operand->accept(*this);
    result = -result;
  }
  void visit(LogicalNotExp& node) override {
    node.operand->accept(*this);
    result = !result;
  }
  void visit(AddExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs + rhs;
  }
  void visit(SubExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs - rhs;
  }
  void visit(MulExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs * rhs;
  }
  void visit(DivExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs / rhs;
  }
  void visit(ModExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs % rhs;
  }
  void visit(LTExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs < rhs;
  }
  void visit(GTExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs > rhs;
  }
  void visit(LEExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs <= rhs;
  }
  void visit(GEExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs >= rhs;
  }
  void visit(EQExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs == rhs;
  }
  void visit(NEExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs != rhs;
  }
  void visit(LAndExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs && rhs;
  }
  void visit(LOrExp& node) override {
    node.lhs->accept(*this);
    int lhs = result;
    node.rhs->accept(*this);
    int rhs = result;
    result = lhs || rhs;
  }
  void visit(LValExp& node) override {
    if(sym_table_stack == nullptr || sym_table_stack->find(node.ident, SymbolTables::SymbolKind::CONST) == false) {
      throw std::runtime_error("undefined const symbol: " + node.ident);
    }
    result = std::get<int>(sym_table_stack->get(node.ident, SymbolTables::SymbolKind::CONST));
  }
  

};
















}  // namespace AST

