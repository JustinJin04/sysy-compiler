#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <algorithm>
#include <unordered_map>
#include <cassert>
#include "utils.hpp"
#include <iostream>

namespace KOOPA {



class StackCalculatorVisitor : public Visitor {
 public:

  // final output
  int stack_size = 0;

  int local_var_size = 0;
  int ra_size = 0;
  int arg_size = 0;
  
  void visit(const koopa_raw_function_t& func) override;

  void visit(const koopa_raw_basic_block_t& bb) override;

  void visit(const koopa_raw_value_t& inst) override;

};


class FuncStack{
 public:
  int size;
  int offset; // from size(empty) to 0(full)
  std::unordered_map<koopa_raw_value_t, int> value_to_offset;
  bool has_ra = false;

  FuncStack(int _stack_size=16) {
    size = _stack_size;
    offset = _stack_size;
  }

  // call when genASM visit function
  void reset(int new_size){
    assert(new_size % 16 == 0);
    size = new_size;
    offset = new_size;
    value_to_offset.clear();
    has_ra = false;
  }

  void insert(const koopa_raw_value_t& value) {
    int value_size = 0;
    if(value->ty->tag == KOOPA_RTT_POINTER) {
      value_size = get_type_width(value->ty->data.pointer.base);
    } else if(value->ty->tag == KOOPA_RTT_INT32) {
      value_size = 4;
    } else {
      assert(0);
    }
    // std::cout<<"value_size: " << value_size << std::endl;
    offset -= value_size;
    value_to_offset[value] = offset;
  }

  int get_offset(const koopa_raw_value_t& value){
    auto it = value_to_offset.find(value);
    if(it != value_to_offset.end()){
      return it->second;
    } else {
      throw std::runtime_error("Value not found in stack");
    }
  }

  bool find(const koopa_raw_value_t& value){
    return value_to_offset.find(value) != value_to_offset.end();
  }

  void insert_ra(){
    assert(offset == size);
    offset -= 4;
    has_ra = true;
  }

  int get_offset_ra(){
    assert(has_ra);
    return size - 4;
  }


};





};  // namespace KOOPA