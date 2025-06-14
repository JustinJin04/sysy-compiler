#pragma once

#include "visitor.hpp"
#include <unordered_map>
#include <variant>
#include "symtable.hpp"
#include <cassert>
#include <algorithm>

namespace AST {

class EvaluateVisitor;
class ConstArrayEvaluateVisitor;
class VarArrayEvaluateVisitor;
class LinkListVisitor;

class EvaluateVisitor : public Visitor {
 public:
  int result = 0;
  // std::unordered_map<std::string, std::variant<int, std::string>> sym_table;
  SymbolTables* sym_table_stack;

  EvaluateVisitor(SymbolTables* other_sym_table) {
    sym_table_stack = other_sym_table;
  }

  void reset() { result = 0; }

  // Currently only support evaluate expressions
  // later will support evaluate consteval and vareval
  void visit(NumberExp& node) override { result = node.number; }
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
    if (sym_table_stack == nullptr ||
        sym_table_stack->find(node.ident, SymbolTables::SymbolKind::CONST) ==
            false) {
      throw std::runtime_error("undefined const symbol: " + node.ident);
    }
    result = std::get<int>(
        sym_table_stack->get(node.ident, SymbolTables::SymbolKind::CONST));
  }
};

class LinkListVisitor : public Visitor {
 public:
  std::vector<int> result;
  SymbolTables* sym_table_stack;

  LinkListVisitor(SymbolTables* other_sym_table) {
    sym_table_stack = other_sym_table;
  }

  void visit(ArrayDims& dims) override {
    assert(dims.exp != nullptr);
    EvaluateVisitor evaluator(sym_table_stack);
    dims.exp->accept(evaluator);
    result.push_back(evaluator.result);
    auto ptr = dims.next_dim.get();
    while (ptr) {
      assert(ptr->exp != nullptr);
      ptr->exp->accept(evaluator);
      result.push_back(evaluator.result);
      ptr = ptr->next_dim.get();
    }
  }
};

class ConstArrayEvaluateVisitor : public Visitor {
 public:
  std::vector<int> result;
  SymbolTables* sym_table_stack;

  ConstArrayEvaluateVisitor(SymbolTables* other_sym_table) {
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

  int constdef_recur(const std::vector<int>& shape, int layer,
                     ArrayInitVal* node, EvaluateVisitor& evaluator) {
    int list_size = node->array_init_val_hierarchy.size();
    int push_num = 0;

    auto compute_layer = [](const std::vector<int>& shape, int size) {
      std::stack<std::vector<int>> st;
      st.push(shape);
      while (size > 0) {
        if (st.top().size() == 0) {
          st.pop();
          size -= 1;
          continue;
        }
        auto top_vec = st.top();
        st.pop();
        int first_dim = top_vec[0];
        top_vec.erase(top_vec.begin());
        for (int i = 0; i < first_dim; ++i) {
          st.push(top_vec);
        }
      }
      return static_cast<int>(shape.size() - st.top().size());
    };

    for (int i = 0; i < list_size; ++i) {
      if (node->array_init_val_hierarchy[i]->exp) {
        evaluator.reset();
        node->array_init_val_hierarchy[i]->exp->accept(evaluator);
        result.push_back(evaluator.result);
        push_num += 1;
      } else {
        int new_layer =
            std::max(compute_layer(shape, result.size()), layer + 1);
        push_num +=
            constdef_recur(shape, new_layer,
                           node->array_init_val_hierarchy[i].get(), evaluator);
      }
    }
    int total_num = 1;
    for (int i = layer; i < shape.size(); ++i) {
      total_num *= shape[i];
    }
    while (push_num < total_num) {
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
    assert(node.const_init_val != nullptr &&
           node.const_init_val->exp == nullptr);
    result.clear();
    constdef_recur(shape, 0, node.const_init_val.get(), evaluator);
  }
};

class VarArrayEvaluateVisitor : public Visitor {
 public:
  std::vector<std::variant<Exp*, int>> result;
  SymbolTables* sym_table_stack;

  VarArrayEvaluateVisitor(SymbolTables* other_sym_table) {
    sym_table_stack = other_sym_table;
  }

  int vardef_recur(const std::vector<int>& shape, int layer,
                   ArrayInitVal* node) {
    int list_size = node->array_init_val_hierarchy.size();
    std::cout << "layer: " << layer << " list_size: " << list_size << std::endl;
    int push_num = 0;

    auto compute_layer = [](const std::vector<int>& shape, int size) {
      std::stack<std::vector<int>> st;
      st.push(shape);
      while (size > 0) {
        if (st.top().size() == 0) {
          st.pop();
          size -= 1;
          continue;
        }
        auto top_vec = st.top();
        st.pop();
        int first_dim = top_vec[0];
        top_vec.erase(top_vec.begin());
        for (int i = 0; i < first_dim; ++i) {
          st.push(top_vec);
        }
      }
      return static_cast<int>(shape.size() - st.top().size());
    };

    for (int i = 0; i < list_size; ++i) {
      if (node->array_init_val_hierarchy[i]->exp) {
        // Note there are two possible exp
        // one is that exp is bind to a name
        // the other is that exp is a number
        // we must store it as number if it is a number
        // otherwise, we store the Exp*
        // result.push_back(node->array_init_val_hierarchy[i]->exp.get());
        // TODO: this is a temporary solution
        try {
          EvaluateVisitor evaluator(sym_table_stack);
          node->array_init_val_hierarchy[i]->exp->accept(evaluator);
          result.push_back(evaluator.result);
        } catch (std::runtime_error& e) {
          result.push_back(node->array_init_val_hierarchy[i]->exp.get());
        }
        push_num += 1;
      } else {
        int new_layer =
            std::max(compute_layer(shape, result.size()), layer + 1);
        push_num += vardef_recur(shape, new_layer,
                                 node->array_init_val_hierarchy[i].get());
      }
    }
    int total_num = 1;
    for (int i = layer; i < shape.size(); ++i) {
      total_num *= shape[i];
    }
    while (push_num < total_num) {
      result.push_back(0);
      push_num += 1;
    }
    return total_num;
  }

  // flattern the array_init_val_hierarchy into std::vector<std::variant<Exp*,
  // int>>
  void visit(VarDef& node) override {
    auto shape_visitor = LinkListVisitor(sym_table_stack);
    node.array_dims->accept(shape_visitor);
    auto& shape = shape_visitor.result;
    result.clear();
    assert(node.var_init_val);
    vardef_recur(shape, 0, node.var_init_val.get());
  }
};

/**
 * for examples:
 * int a[] => *i32
 * int a[][2] => *[i32, 2]
 * int a[][2][3] => *[[i32, 3], 2]
 */
class ArrTypeEvaluateVisitor : public Visitor {
 public:
  std::string type_name;
  int dims;
  SymbolTables* sym_table_stack;

  ArrTypeEvaluateVisitor(SymbolTables* other_sym_table) {
    sym_table_stack = other_sym_table;
  }

  void type_recur(const std::vector<int>& shape, int layer) {
    int num_dims = shape.size();
    if (layer == num_dims) {
      type_name += "i32";
      return;
    }
    type_name += "[";
    type_recur(shape, layer + 1);
    type_name += ", " + std::to_string(shape[layer]) + "]";
  }

  void visit(FuncFParamArr& node) override {
    type_name = "*";
    dims = 1;
    if (node.array_dims == nullptr) {
      type_name += "i32";
      return;
    }
    auto shape_visitor = LinkListVisitor(sym_table_stack);
    node.array_dims->accept(shape_visitor);
    auto& shape = shape_visitor.result;
    dims += shape.size();
    type_recur(shape, 0);
  }

  // void gen_array_type_recur(const std::vector<int>& shape, int layer) {
  //   int num_dims = shape.size();
  //   if (layer == num_dims) {
  //     result += "i32";
  //     return;
  //   }
  //   result += "[";
  //   gen_array_type_recur(shape, layer + 1);
  //   result += ", " + std::to_string(shape[layer]) + "]";
  // }
};

}  // namespace AST
