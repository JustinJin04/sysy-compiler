#pragma once

#include <memory>
#include "visitor.hpp"
#include <unordered_map>
#include <variant>
#include <vector>


namespace AST {

class SymbolTables {
 public:
  struct ConstArrInfo {
    std::string sym_name;
    std::vector<int> dims;
    std::vector<int> data;
  };
  // struct VarArrayInfo {
  //   std::vector<int> dims;
  //   std::vector<std:> data;
  // };



  std::vector<std::unordered_map<std::string, int>> const_sym_table_stack;
  std::vector<std::unordered_map<std::string, std::string>> var_sym_table_stack;
  std::vector<std::unordered_map<std::string, std::string>> func_sym_table_stack;

  std::vector<std::unordered_map<std::string, ConstArrInfo>> const_arr_sym_table_stack;

  enum class SymbolKind {
    CONST,
    VAR,
    FUNC,
    CONST_ARR
  };

  // insert a const symbol into symbol table
  void insert_to_top(const std::string& ident, int value) {
    const_sym_table_stack.back()[ident] = value;
  }
  // insert a var/func symbol into symbol table
  void insert_to_top(const std::string& ident, const std::string& value, SymbolKind type){
    if(type == SymbolKind::CONST) {
      assert(0);
    } else if(type == SymbolKind::VAR) {
      var_sym_table_stack.back()[ident] = value;
    } else if(type == SymbolKind::FUNC) {
      func_sym_table_stack.back()[ident] = value;
    }
  }

  // TODO: can we let symbol table to decide the symbol name of ident??????
  // for example, array name is arr, let symbol table decide whether arr_1, arr_2, arr_3, ... to store
  void insert_to_top(const std::string& ident, const std::vector<int>& dims, const std::vector<int>& data, SymbolKind kind);


  /**
   * find the first occurrence of ident in symbol table stack
   * return the symbol kind
   */
  SymbolKind find(const std::string& ident);


  bool find(const std::string& ident, SymbolKind type) {
    switch (type) {
      case SymbolKind::CONST: {
        for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return true;
          }
        }
        return false;
      }
      case SymbolKind::VAR: {
        for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return true;
          }
        }
        return false;
      }
      case SymbolKind::FUNC: {
        for(auto it = func_sym_table_stack.rbegin(); it != func_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return true;
          }
        }
        return false;
      }
    }
  }

  int find_layer_num(const std::string& ident, SymbolKind type) {
    switch (type) {
      case SymbolKind::CONST: {
        for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return const_sym_table_stack.rend() - it;
          }
        }
        return -1;
      }
      case SymbolKind::VAR: {
        for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return var_sym_table_stack.rend() - it;
          }
        }
        return -1;
      }
      case SymbolKind::FUNC: {
        for(auto it = func_sym_table_stack.rbegin(); it != func_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return func_sym_table_stack.rend() - it;
          }
        }
        return -1;
      }
    }
  }
  
  bool find_in_current(const std::string& ident, SymbolKind type) {
    switch (type) {
      case SymbolKind::CONST: {
        return const_sym_table_stack.back().find(ident) != const_sym_table_stack.back().end();
      }
      case SymbolKind::VAR: {
        return var_sym_table_stack.back().find(ident) != var_sym_table_stack.back().end();
      }
      case SymbolKind::FUNC: {
        return func_sym_table_stack.back().find(ident) != func_sym_table_stack.back().end();
      }
    }
  }

  
  int const_count = -1;
  int var_count = -1;
  int total_accurrences(const std::string& ident, SymbolKind type) {
    assert(type != SymbolKind::FUNC);
    if(type == SymbolKind::CONST) {
      const_count++;
      return const_count;
    } else {
      var_count++;
      return var_count;
    }
  }
  
  /**
   * for const scalar, return int
   * else, return symbol name
   */
  std::variant<int, std::string> get(const std::string& ident, SymbolKind type) {
    switch (type) {
      case SymbolKind::CONST: {
        for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return it->at(ident);
          }
        }
        throw std::runtime_error("undefined const symbol: " + ident);
      }
      case SymbolKind::VAR: {
        for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return it->at(ident);
          }
        }
        throw std::runtime_error("undefined var symbol: " + ident);
      }
      case SymbolKind::FUNC: {
        for(auto it = func_sym_table_stack.rbegin(); it != func_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return it->at(ident);
          }
        }
        throw std::runtime_error("undefined func symbol: " + ident);
      }
    }
  }

  int get_array(const std::string& ident, const std::vector<int> array_dims);


  void push_table(){
    const_sym_table_stack.push_back(std::unordered_map<std::string, int>());
    var_sym_table_stack.push_back(std::unordered_map<std::string, std::string>());
    func_sym_table_stack.push_back(std::unordered_map<std::string, std::string>());
  }
  
  void pop_table(){
    const_sym_table_stack.pop_back();
    var_sym_table_stack.pop_back();
    func_sym_table_stack.pop_back();
  }

};












};  // namespace AST


