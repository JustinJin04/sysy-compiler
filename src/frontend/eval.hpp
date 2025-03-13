#pragma once

#include "visitor.hpp"
#include <unordered_map>

namespace AST {

class EvaluateVisitor : public Visitor {
 public:
  int result = 0;
  std::unordered_map<std::string, int> const_sym_table;

  EvaluateVisitor(std::unordered_map<std::string, int> other_const_sym_table = {}){
    const_sym_table = other_const_sym_table;
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

  // TODO: following functions shouldn't be called
  // Implement them to avoid compile error
  void visit(CompUnit& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(FuncDef& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(FuncType& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(RetStmt& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(ConstDecl& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(ConstDef& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(AssignStmt& node) override {
    throw std::runtime_error("raise not implemented error");
  }
  void visit(LValExp& node) override {
    // throw std::runtime_error("raise not implemented error");
    if(const_sym_table.find(node.ident) == const_sym_table.end()) {
      throw std::runtime_error("undefined symbol: " + node.ident);
    }
    result = const_sym_table[node.ident];
  }
};
















}  // namespace AST

