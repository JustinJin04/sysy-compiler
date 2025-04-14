#include "stack.hpp"

#include "utils.hpp"

#include <iostream>

namespace KOOPA {
void StackCalculatorVisitor::visit(const koopa_raw_function_t& func) {
  for (int i = 0; i < func->bbs.len; ++i) {
    auto ptr = func->bbs.buffer[i];
    visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
  }
  stack_size = (local_var_size + ra_size + arg_size + 15) / 16 * 16;
}

void StackCalculatorVisitor::visit(const koopa_raw_basic_block_t& bb) {
  for (int i = 0; i < bb->insts.len; ++i) {
    auto ptr = bb->insts.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr));
  }
}

void StackCalculatorVisitor::visit(const koopa_raw_value_t& inst) {
  switch (inst->kind.tag) {
    case KOOPA_RVT_ALLOC: {
      assert(inst->ty->tag == KOOPA_RTT_POINTER);
      local_var_size += get_type_width(inst->ty->data.pointer.base);
      // std::cout<<"alloc " << inst->ty->tag<<" "<<local_var_size << std::endl;
      break;
    }
    case KOOPA_RVT_BINARY: {
      local_var_size += 4;
      break;
    }
    case KOOPA_RVT_LOAD: {
      local_var_size += 4;
      break;
    }
    case KOOPA_RVT_CALL: {
      ra_size = 4;
      arg_size = std::max(
          arg_size, std::max(0, (int)(inst->kind.data.call.args.len - 8) * 4));
      if (inst->kind.data.call.callee->ty->data.function.ret) {
        local_var_size += 4;
      }
      break;
    }
    case KOOPA_RVT_GET_PTR: {
      local_var_size += 4;
      break;
    }
    case KOOPA_RVT_GET_ELEM_PTR: {
      local_var_size += 4;
      break;
    }
    default: {
      break;
    }
  }
}

};  // namespace KOOPA