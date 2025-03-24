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

  struct VarArrInfo {
    std::string sym_name;
    std::vector<int> dims;
    std::vector<std::variant<Exp*, int>> data;
  };

  struct PtrInfo {
    std::string sym_name;
    std::string type_name;
    int dims;
  };



  std::vector<std::unordered_map<std::string, int>> const_sym_table_stack;
  std::vector<std::unordered_map<std::string, std::string>> var_sym_table_stack;
  std::vector<std::unordered_map<std::string, std::string>> func_sym_table_stack;

  std::vector<std::unordered_map<std::string, ConstArrInfo>> const_arr_sym_table_stack;
  std::vector<std::unordered_map<std::string, VarArrInfo>> var_arr_sym_table_stack;

  std::vector<std::unordered_map<std::string, PtrInfo>> ptr_sym_table_stack;

  enum class SymbolKind {
    CONST,
    VAR,
    FUNC,
    CONST_ARR,
    VAR_ARR,
    PTR
  };

  // insert a const symbol into symbol table
  void insert_to_top(const std::string& ident, int value) {
    const_sym_table_stack.back()[ident] = value;
  }

  // insert a var/func into symbol table
  void insert_to_top(const std::string& ident, const std::string& value, SymbolKind type){
    if(type == SymbolKind::CONST) {
      assert(0);
    } else if(type == SymbolKind::VAR) {
      var_sym_table_stack.back()[ident] = value;
    } else if(type == SymbolKind::FUNC) {
      func_sym_table_stack.back()[ident] = value;
    } else {
      assert(0);
    }
  }

  // insert a ptr into symbol table
  void insert_to_top(const std::string& ident, const std::string& type_name, int dims, SymbolKind kind) {
    assert(kind == SymbolKind::PTR);
    PtrInfo info;
    info.sym_name = "%" + ident;
    info.type_name = type_name;
    info.dims = dims;
    ptr_sym_table_stack.back()[ident] = info;
  }

  // TODO: can we let symbol table to decide the symbol name of ident??????
  // for example, array name is arr, let symbol table decide whether arr_1, arr_2, arr_3, ... to store
  void insert_to_top(const std::string& ident, const std::vector<int>& dims, const std::vector<int>& data, SymbolKind kind) {
    assert(kind == SymbolKind::CONST_ARR);
    ConstArrInfo info;
    info.dims = dims;
    info.data = data;
    int count = total_accurrences(ident, kind);
    info.sym_name = "@" + ident + "_" + std::to_string(count + 1);
    const_arr_sym_table_stack.back()[ident] = info;
  }

  // used to store both global and local var array symbol
  void insert_to_top(const std::string& ident, const std::vector<int>& dims, 
          const std::vector<std::variant<Exp*, int>>& data, SymbolKind kind) {
    assert(kind == SymbolKind::VAR_ARR);
    VarArrInfo info;
    info.dims = dims;
    info.data = data;
    int count = total_accurrences(ident, kind);
    info.sym_name = "@" + ident + "_" + std::to_string(count + 1);
    var_arr_sym_table_stack.back()[ident] = info;
  }
  // void insert_to_top(const std::string& ident, const std::string& type_name, SymbolKind kind) {
  //   assert(kind == SymbolKind::PTR);
  //   PtrInfo info;
  //   // note that we cannot append count behind sym_name, since it has been declared at funcdef
  //   info.sym_name = "@" + ident;
  //   info.type_name = type_name;
  //   ptr_sym_table_stack.back()[ident] = info;
  // }

  /**
   * find the first occurrence of ident in symbol table stack
   * return the symbol kind
   */
  SymbolKind find(const std::string& ident) {
    int layer_num = const_sym_table_stack.size();
    for(int i=layer_num-1;i>=0;--i) {
      if(const_sym_table_stack[i].find(ident) != const_sym_table_stack[i].end()) {
        return SymbolKind::CONST;
      }
      if(var_sym_table_stack[i].find(ident) != var_sym_table_stack[i].end()) {
        return SymbolKind::VAR;
      }
      if(func_sym_table_stack[i].find(ident) != func_sym_table_stack[i].end()) {
        return SymbolKind::FUNC;
      }
      if(const_arr_sym_table_stack[i].find(ident) != const_arr_sym_table_stack[i].end()) {
        return SymbolKind::CONST_ARR;
      }
      if(var_arr_sym_table_stack[i].find(ident) != var_arr_sym_table_stack[i].end()) {
        return SymbolKind::VAR_ARR;
      }
      if(ptr_sym_table_stack[i].find(ident) != ptr_sym_table_stack[i].end()) {
        return SymbolKind::PTR;
      }
    }
    assert(0);
  }


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
      case SymbolKind::CONST_ARR: {
        for(auto it = const_arr_sym_table_stack.rbegin(); it != const_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return true;
          }
        }
        return false;
      }
      case SymbolKind::VAR_ARR: {
        for(auto it = var_arr_sym_table_stack.rbegin(); it != var_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return true;
          }
        }
        return false;
      }
      case SymbolKind::PTR: {
        for(auto it = ptr_sym_table_stack.rbegin(); it != ptr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return true;
          }
        }
        return false;
      }
      default: {
        assert(0);
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
      case SymbolKind::CONST_ARR: {
        for(auto it = const_arr_sym_table_stack.rbegin(); it != const_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return const_arr_sym_table_stack.rend() - it;
          }
        }
        return -1;
      }
      case SymbolKind::VAR_ARR: {
        for(auto it = var_arr_sym_table_stack.rbegin(); it != var_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return var_arr_sym_table_stack.rend() - it;
          }
        }
        return -1;
      }
      case SymbolKind::PTR: {
        for(auto it = ptr_sym_table_stack.rbegin(); it != ptr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return ptr_sym_table_stack.rend() - it;
          }
        }
        return -1;
      }
      default: {
        assert(0);
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
      case SymbolKind::CONST_ARR: {
        return const_arr_sym_table_stack.back().find(ident) != const_arr_sym_table_stack.back().end();
      }
      case SymbolKind::VAR_ARR: {
        return var_arr_sym_table_stack.back().find(ident) != var_arr_sym_table_stack.back().end();
      }
      case SymbolKind::PTR: {
        return ptr_sym_table_stack.back().find(ident) != ptr_sym_table_stack.back().end();
      }
      default: {
        assert(0);
      }
    }
  }

  
  int const_count = -1;
  int var_count = -1;
  int const_array_count = -1;
  int var_array_count = -1;
  int total_accurrences(const std::string& ident, SymbolKind type) {
    assert(type != SymbolKind::FUNC);
    if(type == SymbolKind::CONST) {
      const_count++;
      return const_count;
    } else if (type == SymbolKind::VAR) {
      var_count++;
      return var_count;
    } else if (type == SymbolKind::CONST_ARR) {
      const_array_count++;
      return const_array_count;
    } else if(type == SymbolKind::VAR_ARR) {
      var_array_count++;
      return var_array_count;
    } else {
      assert(0);
    }
  }
  
  /**
   * used to get const, var, func, const array symbol value
   * 
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
      case SymbolKind::CONST_ARR: {
        for(auto it = const_arr_sym_table_stack.rbegin(); it != const_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return it->at(ident).sym_name;
          }
        }
        throw std::runtime_error("undefined const array symbol: " + ident);
      }
      case SymbolKind::VAR_ARR: {
        for(auto it = var_arr_sym_table_stack.rbegin(); it != var_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return it->at(ident).sym_name;
          }
        }
        throw std::runtime_error("undefined var array symbol: " + ident);
      }
      case SymbolKind::PTR: {
        for(auto it = ptr_sym_table_stack.rbegin(); it != ptr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            return it->at(ident).sym_name;
          }
        }
        throw std::runtime_error("undefined ptr symbol: " + ident);
      }
      default: {
        assert(0);
      }
    }
  }

  /**
   * used to get const/var array
   */
  std::variant<Exp*, int> get_array(const std::string& ident, const std::vector<int> array_dims, SymbolKind kind) {
    switch (kind) {
      case SymbolKind::CONST_ARR: {
        for(auto it = const_arr_sym_table_stack.rbegin(); it!= const_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            auto& info = it->at(ident);
            int idx = 0;
            int multiplier = 1;
            for(int i=array_dims.size()-1;i>=0;--i){
              idx += array_dims[i] * multiplier;
              multiplier *= info.dims[i];
            }
            return info.data[idx];
          }
        }
        throw std::runtime_error("undefined const array symbol: " + ident);
      }
      case SymbolKind::VAR_ARR: {
        for(auto it = var_arr_sym_table_stack.rbegin(); it!= var_arr_sym_table_stack.rend(); it++) {
          if(it->find(ident) != it->end()) {
            auto& info = it->at(ident);
            int idx = 0;
            int multiplier = 1;
            for(int i=array_dims.size()-1;i>=0;--i){
              idx += array_dims[i] * multiplier;
              multiplier *= info.dims[i];
            }
            return info.data[idx];
          }
        }
        throw std::runtime_error("undefined var array symbol: " + ident);
      }
      default: {
        assert(0);
      }
    }
  }

  VarArrInfo get_var_arr_info(const std::string& ident) {
    for(auto it = var_arr_sym_table_stack.rbegin(); it!= var_arr_sym_table_stack.rend(); it++) {
      if(it->find(ident) != it->end()) {
        return it->at(ident);
      }
    }
    throw std::runtime_error("undefined var array symbol: " + ident);
  }

  PtrInfo get_ptr_info(const std::string& ident) {
    for(auto it = ptr_sym_table_stack.rbegin(); it!= ptr_sym_table_stack.rend(); it++) {
      if(it->find(ident) != it->end()) {
        return it->at(ident);
      }
    }
    throw std::runtime_error("undefined ptr symbol: " + ident);
  }


  void push_table(){
    const_sym_table_stack.push_back(std::unordered_map<std::string, int>());
    var_sym_table_stack.push_back(std::unordered_map<std::string, std::string>());
    func_sym_table_stack.push_back(std::unordered_map<std::string, std::string>());
    const_arr_sym_table_stack.push_back(std::unordered_map<std::string, ConstArrInfo>());
    var_arr_sym_table_stack.push_back(std::unordered_map<std::string, VarArrInfo>());
    ptr_sym_table_stack.push_back(std::unordered_map<std::string, PtrInfo>());
  }
  
  void pop_table(){
    const_sym_table_stack.pop_back();
    var_sym_table_stack.pop_back();
    func_sym_table_stack.pop_back();
    const_arr_sym_table_stack.pop_back();
    var_arr_sym_table_stack.pop_back();
    ptr_sym_table_stack.pop_back();
  }

};












};  // namespace AST


