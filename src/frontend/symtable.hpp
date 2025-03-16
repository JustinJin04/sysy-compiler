#pragma once

#include <memory>
#include "visitor.hpp"
#include <unordered_map>
#include <variant>
#include <vector>


namespace AST {

class SymbolTables {
 public:
  std::vector<std::unordered_map<std::string, int>> const_sym_table_stack;
  std::vector<std::unordered_map<std::string, std::string>> var_sym_table_stack;

  // insert a const symbol into symbol table
  void insert_to_top(const std::string& ident, int value) {
    const_sym_table_stack.back()[ident] = value;
  }
  // insert a var symbol into symbol table
  void insert_to_top(const std::string& ident, const std::string& value){
    var_sym_table_stack.back()[ident] = value;
  }

  bool find(const std::string& ident, bool is_const){
    if(is_const) {
      for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
        if(it->find(ident) != it->end()) {
          return true;
        }
      }
      return false;
    } else {
      for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
        if(it->find(ident) != it->end()) {
          return true;
        }
      }
      return false;
    }
  }

  int find_layer_num(const std::string& ident, bool is_const){
    if(is_const) {
      for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
        if(it->find(ident) != it->end()) {
          return const_sym_table_stack.rend() - it;
        }
      }
      return -1;
    } else {
      for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
        if(it->find(ident) != it->end()) {
          return var_sym_table_stack.rend() - it;
        }
      }
      return -1;
    }
  }

  bool find_in_current(const std::string& ident, bool is_const){
    if(is_const) {
      return const_sym_table_stack.back().find(ident) != const_sym_table_stack.back().end();
    } else {
      return var_sym_table_stack.back().find(ident) != var_sym_table_stack.back().end();
    }
  }

  int const_count = -1;
  int var_count = -1;
  int total_accurrences(const std::string& ident, bool is_const) {
    // static int const_count = 0;
    // static int var_count = 0;
    if(is_const) {
      const_count++;
      return const_count;
    } else {
      var_count++;
      return var_count;
    }
    // int count = 0;
    // if(is_const) {
    //   for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
    //     if(it->find(ident) != it->end()) {
    //       count++;
    //     }
    //   }
    //   return count;
    // } else {
    //   for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
    //     if(it->find(ident) != it->end()) {
    //       count++;
    //     }
    //   }
    //   return count;
    // }
  }

  std::variant<int, std::string> get(const std::string& ident, bool is_const){
    if(is_const){
      for(auto it = const_sym_table_stack.rbegin(); it != const_sym_table_stack.rend(); it++) {
        if(it->find(ident) != it->end()) {
          return it->at(ident);
        }
      }
      throw std::runtime_error("undefined const symbol: " + ident);
    } else {
      for(auto it = var_sym_table_stack.rbegin(); it != var_sym_table_stack.rend(); it++) {
        if(it->find(ident) != it->end()) {
          return it->at(ident);
        }
      }
      throw std::runtime_error("undefined var symbol: " + ident);
    }
  }

  void push_table(){
    const_sym_table_stack.push_back(std::unordered_map<std::string, int>());
    var_sym_table_stack.push_back(std::unordered_map<std::string, std::string>());
  }
  void pop_table(){
    const_sym_table_stack.pop_back();
    var_sym_table_stack.pop_back();
  }

};












};  // namespace AST


