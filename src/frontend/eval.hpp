#pragma once

#include "visitor.hpp"
#include <unordered_map>
#include <variant>
#include "symtable.hpp"
#include <cassert>

namespace AST {

class EvaluateVisitor;
class ArrayEvaluateVisitor;
class LinkListVisitor;

class EvaluateVisitor : public Visitor {
 public:
  int result = 0;
  // std::unordered_map<std::string, std::variant<int, std::string>> sym_table;
  SymbolTables* sym_table_stack;

  EvaluateVisitor(SymbolTables* other_sym_table=nullptr){
    sym_table_stack = other_sym_table;
  }

  void reset() {
    result = 0;
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

class ArrayEvaluateVisitor : public Visitor {
 public:
  std::vector<int> result;
  SymbolTables* sym_table_stack;

  ArrayEvaluateVisitor(SymbolTables* other_sym_table=nullptr){
    sym_table_stack = other_sym_table;
  }

  /**
   * Examples:
   * int arr[2][3][4] = {1, 2, 3, 4, {5}, {6}, {7, 8}};
   *   ==>>
   * int arr[2][3][4] = {
   *   {{1, 2, 3, 4}, {5, 0, 0, 0}, {6, 0, 0, 0}},
   *   {{7, 8, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
   * };
   */

  int constdef_recur(const std::vector<int>& shape, int layer, ArrayInitVal* node, EvaluateVisitor& evaluator, std::vector<int>& result) {
    int num_dims = shape.size();
    int list_size = node->array_init_val_hierarchy.size();
    int push_num = 0;

    auto compute_layer = [](const std::vector<int>& shape, int size){
      std::stack<std::vector<int>> st;
      st.push(shape);
      while(size > 0){
        if(st.top().size() == 0){
          st.pop();
          size -= 1;
          continue;
        }
        auto top_vec = st.top();
        st.pop();
        int first_dim = top_vec[0];
        top_vec.erase(top_vec.begin());
        for(int i=0;i<first_dim;++i){
          st.push(top_vec);
        }
      }
      return shape.size() - st.top().size();
    };

    for(int i=0;i<list_size;++i) {
      if(node->array_init_val_hierarchy[i]->exp) {
        evaluator.reset();
        node->array_init_val_hierarchy[i]->exp->accept(evaluator);
        result.push_back(evaluator.result);
        push_num += 1;
      } else {
        int new_layer = compute_layer(shape, result.size());
        push_num += constdef_recur(shape, new_layer, node->array_init_val_hierarchy[i].get(), evaluator, result);
      }
    }
    int total_num = 1;
    for(int i=layer;i<shape.size();++i){
      total_num *= shape[i];
    }
    while(push_num < total_num){
      result.push_back(0);
      push_num += 1;
    }
    return total_num;

  }

  void visit(ConstDef& node) override {
    auto shape_visitor = LinkListVisitor(sym_table_stack);
    node.array_dims->accept(shape_visitor);
    auto& shape = shape_visitor.result;
    EvaluateVisitor evaluator(sym_table_stack);
    assert(node.const_init_val != nullptr && node.const_init_val->exp == nullptr);
    constdef_recur(shape, 0, node.const_init_val.get(), evaluator, result);
  }
};

class LinkListVisitor : public Visitor {
 public:
  std::vector<int> result; 
  SymbolTables* sym_table_stack;

  LinkListVisitor(SymbolTables* other_sym_table=nullptr){
    sym_table_stack = other_sym_table;
  }

  void visit(ArrayDims& dims) override;
};










}  // namespace AST

