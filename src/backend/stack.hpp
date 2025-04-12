#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <algorithm>
#include <unordered_map>

namespace KOOPA {

class StackCalculatorVisitor : public Visitor {
 public:

  // final output
  int stack_size = 0;

  int local_var_size = 0;
  int ra_size = 0;
  int arg_size = 0;
  
  void visit(const koopa_raw_function_t& func) override {
    for(int i=0;i<func->bbs.len;++i){
      auto ptr = func->bbs.buffer[i];
      visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
    }
    stack_size = (local_var_size + ra_size + arg_size + 15) / 16 * 16;
  }

  void visit(const koopa_raw_basic_block_t& bb) override {
    for(int i=0;i<bb->insts.len;++i){
      auto ptr = bb->insts.buffer[i];
      visit(reinterpret_cast<koopa_raw_value_t>(ptr));
    }
  }

  void visit(const koopa_raw_value_t& inst) override {
    switch (inst->kind.tag) {
      case KOOPA_RVT_ALLOC: {
        // stack_size += 4;
        local_var_size += 4;
        break;
      }
      case KOOPA_RVT_BINARY: {
        // stack_size += 4;
        local_var_size += 4;
        break;
      }
      case KOOPA_RVT_LOAD: {
        // stack_size += 4;
        local_var_size += 4;
        break;
      }
      case KOOPA_RVT_CALL: {
        ra_size = 4;
        arg_size = std::max(arg_size, std::max(0, (int)(inst->kind.data.call.args.len - 8) * 4));
        if(inst->kind.data.call.callee->ty->data.function.ret) {
          local_var_size += 4;
        }
      }
      default:{
        break;
      }
    }
  }

};


class FuncStack{
 public:
  int size;
  int offset; // from size - 4 to 0
  std::unordered_map<koopa_raw_value_t, int> value_to_offset;
  bool has_ra = false;

  FuncStack(int _stack_size=16) {
    size = _stack_size;
    offset = _stack_size - 4;
  }

  // call when genASM visit function
  void reset(int new_size){
    assert(new_size % 16 == 0);
    size = new_size;
    offset = new_size - 4;
    value_to_offset.clear();
    has_ra = false;
  }

  void insert(const koopa_raw_value_t& value) {
    value_to_offset[value] = offset;
    offset -= 4;
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
    assert(offset == size - 4);
    has_ra = true;
    offset -= 4;
  }

  int get_offset_ra(){
    assert(has_ra);
    return size - 4;
  }
};





};  // namespace KOOPA