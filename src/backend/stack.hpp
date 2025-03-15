#pragma once

#include <koopa.h>
#include "visitor.hpp"

namespace KOOPA {

class StackCalculatorVisitor : public Visitor {
 public:

  int stack_size = 0;
  
  void visit(const koopa_raw_function_t& func) override {
    stack_size = 0;
    for(int i=0;i<func->bbs.len;++i){
      auto ptr = func->bbs.buffer[i];
      visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
    }
    stack_size = (stack_size + 15) / 16 * 16;
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
        stack_size += 4;
        break;
      }
      case KOOPA_RVT_BINARY: {
        stack_size += 4;
        break;
      }
      case KOOPA_RVT_LOAD: {
        stack_size += 4;
        break;
      }
      default:{
        break;
      }
    }
  }

};








};  // namespace KOOPA