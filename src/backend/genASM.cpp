#include "genASM.hpp"

#include <cassert>
#include <string>

#include "aggregate.hpp"
#include "prepareOperand.hpp"
#include "stack.hpp"
#include "utils.hpp"

namespace KOOPA {

void GenASMVisitor::visit(const koopa_raw_program_t& raw) {
  // 遍历global value
  assert(raw.values.kind == KOOPA_RSIK_VALUE);
  for (int i = 0; i < raw.values.len; ++i) {
    auto ptr = raw.values.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr));
  }

  // 遍历global func
  assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
  for (int i = 0; i < raw.funcs.len; ++i) {
    auto ptr = raw.funcs.buffer[i];
    visit(reinterpret_cast<koopa_raw_function_t>(ptr));
  }
}

void GenASMVisitor::visit(const koopa_raw_value_t& raw_value) {
  const auto& kind = raw_value->kind;
  // std::cout << "kind.tag: " << kind.tag << std::endl;
  switch (kind.tag) {
    case KOOPA_RVT_ALLOC: {
      /**1. bind value to stack_offset
       * 2. move stack_offset
       */
      // value_to_offset[raw_value] = stack_offset;
      // stack_offset += 4;
      func_stack.insert(raw_value);
      break;
    }
    case KOOPA_RVT_GLOBAL_ALLOC: {
      code_stream << "  .data" << std::endl;
      code_stream << "  .global " + std::string(raw_value->name).substr(1)
                  << std::endl;
      code_stream << std::string(raw_value->name).substr(1) + ":" << std::endl;
      if (kind.data.global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT) {
        assert(raw_value->ty->tag == KOOPA_RTT_POINTER);
        assert(raw_value->ty->data.pointer.base->tag == KOOPA_RTT_INT32);
        code_stream << "  .zero 4" << std::endl;
      } else if (kind.data.global_alloc.init->kind.tag == KOOPA_RVT_INTEGER) {
        code_stream << "  .word " +
                           std::to_string(kind.data.global_alloc.init->kind.data
                                              .integer.value)
                    << std::endl;
      } else if (kind.data.global_alloc.init->kind.tag == KOOPA_RVT_AGGREGATE) {
        AggregateVisitor aggregate_visitor;
        aggregate_visitor.visit(
            kind.data.global_alloc.init->kind.data.aggregate);
        auto& init_values = aggregate_visitor.init_values;
        for (int i = 0; i < init_values.size(); ++i) {
          code_stream << "  .word " + std::to_string(init_values[i])
                      << std::endl;
        }
      } else {
        assert(0);
      }
      break;
    }
    case KOOPA_RVT_LOAD: {
      /**1. load value.src to register
       * 2. bind value to stack_offset
       * 3. store register to stack_offset(sp)
       * 4. move stack_offset
       */

      auto& src = raw_value->kind.data.load.src;
      assert(src->ty->tag == KOOPA_RTT_POINTER);
      // assert(src->ty->data.pointer.base->tag == KOOPA_RTT_INT32);
      /**
       * lw t0, offset(sp)
       */
      auto load_reg_name = reg_pool.getReg();
      if(func_stack.find(src)){
        int stack_offset = func_stack.get_offset(src);
        if(stack_offset < 2048 && stack_offset >= -2048) {
          code_stream << "  lw " + load_reg_name + ", " +
                            std::to_string(stack_offset) + "(sp)"
                      << std::endl;
        } else {
          auto tmp_reg = reg_pool.getReg();
          code_stream << "  li " + tmp_reg + ", " +
                            std::to_string(stack_offset)
                      << std::endl;
          code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
          code_stream << "  lw " + load_reg_name + ", 0(" + tmp_reg + ")"
                      << std::endl;
          reg_pool.freeReg(tmp_reg);
        }
        if(src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || src->kind.tag == KOOPA_RVT_GET_PTR) {
          code_stream << "  lw " + load_reg_name + ", 0(" + load_reg_name + ")"
                      << std::endl;
        }
      } else if (src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        code_stream << "  la " + load_reg_name + ", " +
                           std::string(src->name).substr(1)
                    << std::endl;
        code_stream << "  lw " + load_reg_name + ", 0(" + load_reg_name + ")"
                    << std::endl;
      } else {
        assert(0);
      }
      store_func_stack(raw_value, load_reg_name);

      reg_pool.freeReg(load_reg_name);
      break;
    }
    case KOOPA_RVT_STORE: {
      visit(kind.data.store);
      break;
    }
    case KOOPA_RVT_GET_PTR: {
      /**
       * 1. load src address (not the content in src address, so don't use
       * prepare operand) to register
       * 2. load dest content to register
       * 3. calculate ptr offset (*dest * width(src->base))
       * 4. add offset to src
       * 5. store to stack
       *
       * for examples:
       * @arr = alloc *[i32, 3]
       * %ptr1 = load @arr
       * %ptr2 = getptr %ptr1, 1
       *
       * @arr at 24(sp)
       * %ptr1 at 20(sp)
       */
      // assert(0);
      auto& src_value = raw_value->kind.data.get_ptr.src;
      auto& index_value = raw_value->kind.data.get_ptr.index;
      assert(src_value->ty->tag == KOOPA_RTT_POINTER);

      // int stack_offset = func_stack.get_offset(src_value);
      int array_offset = get_type_width(src_value->ty->data.pointer.base);
      auto temp_reg_0 = reg_pool.getReg();
      auto prepare_operand_visitor =
          PrepareOperandVisitor(&func_stack, &reg_pool);
      prepare_operand_visitor.visit(index_value);
      auto& index_reg_name = prepare_operand_visitor.load_reg_name;
      auto temp_reg_2 = reg_pool.getReg();

      // start to gen code
      if (func_stack.find(src_value)) {
        assert(src_value->kind.tag != KOOPA_RVT_ALLOC);
        int offset = func_stack.get_offset(src_value);
        if (offset < 2048 && offset >= -2048) {
          code_stream << "  lw " + temp_reg_0 + ", " +
                             std::to_string(offset) + "(sp)"
                      << std::endl;
        } else {
          auto tmp_reg = reg_pool.getReg();
          code_stream << "  li " + tmp_reg + ", " + std::to_string(offset)
                      << std::endl;
          code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
          code_stream << "  lw " + temp_reg_0 + ", 0(" + tmp_reg + ")" << std::endl;
          reg_pool.freeReg(tmp_reg);
        }

      } else if(src_value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        // assert(src_value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC);
        code_stream << "  la " + temp_reg_0 + ", " +
                           std::string(src_value->name).substr(1)
                    << std::endl;
      } else {
        assert(0);
      }
      code_stream << prepare_operand_visitor.asm_code;
      code_stream << "  li " + temp_reg_2 + ", " + std::to_string(array_offset)
                  << std::endl;
      code_stream << "  mul " + index_reg_name + ", " + index_reg_name + ", " +
                         temp_reg_2
                  << std::endl;
      code_stream << "  add " + temp_reg_0 + ", " + temp_reg_0 + ", " +
                         index_reg_name
                  << std::endl;
      store_func_stack(raw_value, temp_reg_0);

      // release temp_reg_name
      // note that index_reg_name will be automatically freed when
      // PrepareOperandVisitor destroyed
      reg_pool.freeReg(temp_reg_0);
      reg_pool.freeReg(temp_reg_2);
      break;
    }
    case KOOPA_RVT_GET_ELEM_PTR: {
      /**
       * 1. load src address (not the content in src address, so don't use
       * prepare operand) to register
       * 2. load dest content to register
       * 3. calculate ptr offset (*dest * width(src->array->base))
       * 4. add offset to src
       * 5. store to stack
       *
       * for examples:
       * @arr = alloc [i32, 2]
       * %ptr = getelemptr @arr, 1
       *
       * @arr at 24(sp)
       * add t0, sp, 24
       * li t1, 1
       * li t2, 4
       * mul t1, t1, t2
       * add t0, t0, t1
       * sw t0, 20(sp)
       */
      auto& src_value = raw_value->kind.data.get_elem_ptr.src;
      auto& index_value = raw_value->kind.data.get_elem_ptr.index;
      assert(src_value->ty->tag == KOOPA_RTT_POINTER);
      assert(src_value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);

      // int stack_offset = func_stack.get_offset(src_value);
      int array_offset =
          get_type_width(src_value->ty->data.pointer.base->data.array.base);
      auto temp_reg_0 = reg_pool.getReg();
      auto prepare_operand_visitor =
          PrepareOperandVisitor(&func_stack, &reg_pool);
      prepare_operand_visitor.visit(index_value);
      auto& index_reg_name = prepare_operand_visitor.load_reg_name;
      auto temp_reg_2 = reg_pool.getReg();

      // start to gen code
      if (func_stack.find(src_value)) {
        int offset = func_stack.get_offset(src_value);
        if(src_value->kind.tag == KOOPA_RVT_GET_ELEM_PTR || src_value->kind.tag == KOOPA_RVT_GET_PTR) {
          if (offset < 2048 && offset >= -2048) {
            code_stream << "  lw " + temp_reg_0 + ", " +
                               std::to_string(offset) + "(sp)"
                        << std::endl;
          } else {
            auto tmp_reg = reg_pool.getReg();
            code_stream << "  li " + tmp_reg + ", " + std::to_string(offset)
                        << std::endl;
            code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
            code_stream << "  lw " + temp_reg_0 + ", 0(" + tmp_reg + ")" << std::endl;
            reg_pool.freeReg(tmp_reg);
          }
        } else if(src_value->kind.tag == KOOPA_RVT_ALLOC){
          if (offset < 2048 && offset >= -2048) {
            code_stream << "  add " + temp_reg_0 + ", sp, " +
                                std::to_string(offset)
                        << std::endl;
          } else {
            code_stream << "  li " + temp_reg_0 + ", " +
                                std::to_string(offset)
                        << std::endl;
            code_stream << "  add " + temp_reg_0 + ", sp, " + temp_reg_0
                        << std::endl;
          }
        } else {
          assert(0);
        }

      } else {
        assert(src_value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC);
        code_stream << "  la " + temp_reg_0 + ", " +
                           std::string(src_value->name).substr(1)
                    << std::endl;
      }

      code_stream << prepare_operand_visitor.asm_code;
      code_stream << "  li " + temp_reg_2 + ", " + std::to_string(array_offset)
                  << std::endl;
      code_stream << "  mul " + index_reg_name + ", " + index_reg_name + ", " +
                         temp_reg_2
                  << std::endl;
      code_stream << "  add " + temp_reg_0 + ", " + temp_reg_0 + ", " +
                         index_reg_name
                  << std::endl;
      store_func_stack(raw_value, temp_reg_0);

      // release temp_reg_name
      // note that index_reg_name will be automatically freed when
      // PrepareOperandVisitor destroyed
      reg_pool.freeReg(temp_reg_0);
      reg_pool.freeReg(temp_reg_2);

      break;
    }
    case KOOPA_RVT_BINARY: {
      auto op = kind.data.binary.op;
      auto lhs = kind.data.binary.lhs;
      auto rhs = kind.data.binary.rhs;

      auto op_1_visitor =
          PrepareOperandVisitor(&func_stack, &reg_pool);
          op_1_visitor.set_load_reg_name("t0");
          op_1_visitor.visit(lhs);
      auto op_2_visitor = 
          PrepareOperandVisitor(&func_stack, &reg_pool);
          op_2_visitor.set_load_reg_name("t1");
          op_2_visitor.visit(rhs);
      code_stream << op_1_visitor.asm_code << std::endl;
      code_stream << op_2_visitor.asm_code << std::endl;

      switch (op) {
        case KOOPA_RBO_NOT_EQ: {
          code_stream << "  xor t0, t0, t1" << std::endl;
          code_stream << "  snez t0, t0" << std::endl;

          break;
        }
        case KOOPA_RBO_EQ: {
          code_stream << "  xor t0, t0, t1" << std::endl;
          code_stream << "  seqz t0, t0" << std::endl;
          break;
        }
        case KOOPA_RBO_GT: {
          code_stream << "  sgt t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_LT: {
          code_stream << "  slt t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_GE: {
          code_stream << "  slt t0, t0, t1" << std::endl;
          code_stream << "  seqz t0, t0" << std::endl;
          break;
        }
        case KOOPA_RBO_LE: {
          code_stream << "  sgt t0, t0, t1" << std::endl;
          code_stream << "  seqz t0, t0" << std::endl;
          break;
        }
        case KOOPA_RBO_ADD: {
          code_stream << "  add t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_SUB: {
          code_stream << "  sub t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_MUL: {
          code_stream << "  mul t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_DIV: {
          code_stream << "  div t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_MOD: {
          code_stream << "  rem t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_AND: {
          code_stream << "  and t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_OR: {
          code_stream << "  or t0, t0, t1" << std::endl;
          break;
        }
        case KOOPA_RBO_XOR: {
          code_stream << "  xor t0, t0, t1" << std::endl;
          break;
        }
        default: {
          assert(false);
        }
      }

      // func_stack.insert(raw_value);
      // int stack_offset = func_stack.get_offset(raw_value);
      // if (stack_offset < 2048 && stack_offset >= -2048) {
      //   code_stream << "  sw t0, " + std::to_string(stack_offset) + "(sp)"
      //               << std::endl;
      // } else {
      //   code_stream << "  li t1, " + std::to_string(stack_offset) <<
      //   std::endl; code_stream << "  add t1, sp, t1" << std::endl;
      //   code_stream << "  sw t0, 0(t1)" << std::endl;
      // }

      store_func_stack(raw_value, "t0");

      // reg_pool.freeReg("t0");
      // reg_pool.freeReg("t1");
      break;
    }
    case KOOPA_RVT_BRANCH: {
      auto prepareOperand = PrepareOperandVisitor(&func_stack, &reg_pool);
      prepareOperand.visit(kind.data.branch.cond);
      code_stream << prepareOperand.asm_code;
      auto& load_reg_name = prepareOperand.load_reg_name;
      auto skip_label = std::string(kind.data.branch.true_bb->name).substr(1) + "_skip";
      // since we have traverse all basic block when visiting raw function
      // we don't need to deal with the true_bb and false_bb here
      code_stream << "  bnez " + load_reg_name + ", " +
                        //  std::string(kind.data.branch.true_bb->name).substr(1)
                        skip_label
                  << std::endl;
      code_stream << "  j " +
                         std::string(kind.data.branch.false_bb->name).substr(1)
                  << std::endl;
      code_stream << skip_label + ":" << std::endl;
      code_stream << "  j " +
                         std::string(kind.data.branch.true_bb->name).substr(1)
                  << std::endl;
      break;
    }
    case KOOPA_RVT_JUMP: {
      code_stream << "  j " + std::string(kind.data.jump.target->name).substr(1)
                  << std::endl;
      break;
    }
    case KOOPA_RVT_CALL: {
      /**
       * 1. load first 8 params to a0-a7
       * 2. if more than 8 params, store to stack
       * 3. call function
       * 5. store return value to stack if needed (func has ret value)
       */
      int param_count = kind.data.call.args.len;
      int i = 0;
      for (; i < param_count && i < 8; ++i) {  // first 8 params
        auto prepareOperandVisitor =
            PrepareOperandVisitor(&func_stack, &reg_pool);
        prepareOperandVisitor.set_load_reg_name("a" + std::to_string(i));
        prepareOperandVisitor.visit(
            reinterpret_cast<koopa_raw_value_t>(kind.data.call.args.buffer[i]));
        code_stream << prepareOperandVisitor.asm_code;
      }
      for (; i < param_count; ++i) {  // more than 8 params
        auto prepareOperandVisitor =
            PrepareOperandVisitor(&func_stack, &reg_pool);
        prepareOperandVisitor.set_load_reg_name("t0");
        prepareOperandVisitor.visit(
            reinterpret_cast<koopa_raw_value_t>(kind.data.call.args.buffer[i]));
        code_stream << prepareOperandVisitor.asm_code;
        auto& load_reg_name = prepareOperandVisitor.load_reg_name;
        int offset = (i - 8) * 4;
        if (offset < 2048 && offset >= -2048) {
          code_stream << "  sw " + load_reg_name + ", " +
                             std::to_string(offset) + "(sp)"
                      << std::endl;
        } else {
          auto tmp_reg = reg_pool.getReg();
          code_stream << "  li " + tmp_reg + ", " + std::to_string(offset)
                      << std::endl;
          code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
          code_stream << "  sw " + load_reg_name + ", 0(" + tmp_reg + ")"
                      << std::endl;
          reg_pool.freeReg(tmp_reg);
        }
      }

      code_stream << "  call " +
                         std::string(kind.data.call.callee->name).substr(1)
                  << std::endl;
      if (kind.data.call.callee->ty->data.function.ret->tag != KOOPA_RTT_UNIT) {
        assert(kind.data.call.callee->ty->data.function.ret->tag ==
               KOOPA_RTT_INT32);
        // store return value to stack
        // func_stack.insert(raw_value);
        // int stack_offset = func_stack.get_offset(raw_value);
        // if (stack_offset < 2048 && stack_offset >= -2048) {
        //   code_stream << "  sw a0, " + std::to_string(stack_offset) + "(sp)"
        //               << std::endl;
        // } else {
        //   auto tmp_reg = reg_pool.getReg();
        //   code_stream << "  li " + tmp_reg + ", " +
        //   std::to_string(stack_offset)
        //               << std::endl;
        //   code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg <<
        //   std::endl; code_stream << "  sw a0, 0(" + tmp_reg + ")" <<
        //   std::endl; reg_pool.freeReg(tmp_reg);
        // }

        store_func_stack(raw_value, "a0");
      }
      break;
    }
    case KOOPA_RVT_RETURN: {
      visit(kind.data.ret);
      break;
    }

    default: {
      assert(false);
    }
  }
}

void GenASMVisitor::visit(const koopa_raw_function_t& raw_func) {
  if (raw_func->bbs.len == 0) {  // pass declaration
    return;
  }
  auto func_name = std::string(raw_func->name).substr(1);
  auto stack_calculator = StackCalculatorVisitor();
  stack_calculator.visit(raw_func);
  int stack_size = stack_calculator.stack_size;
  assert(stack_size % 16 == 0);
  func_stack.reset(stack_size);

  // start to generate asm code
  code_stream << "  .text" << std::endl;
  code_stream << "  .global " + func_name << std::endl;
  code_stream << func_name + ":" << std::endl;
  if (0 < stack_size && stack_size < 2048) {
    code_stream << "  addi sp, sp, -" + std::to_string(stack_size) << std::endl;
  } else if (stack_size >= 2048) {
    code_stream << "  li t0, -" + std::to_string(stack_size) << std::endl;
    code_stream << "  add sp, sp, t0" << std::endl;
  }

  // store ra if needed
  if (stack_calculator.ra_size > 0) {
    // int offset = stack_offset - 4;
    func_stack.insert_ra();
    int offset = func_stack.get_offset_ra();
    if (offset < 2048 && offset >= -2048) {
      code_stream << "  sw ra, " + std::to_string(offset) + "(sp)" << std::endl;
    } else {
      code_stream << "  li t0, " + std::to_string(offset) << std::endl;
      code_stream << "  add t0, sp, t0" << std::endl;
      code_stream << "  sw ra, 0(t0)" << std::endl;
    }
  }

  for (int i = 0; i < raw_func->bbs.len; ++i) {
    auto ptr = raw_func->bbs.buffer[i];
    visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
  }
}

void GenASMVisitor::visit(const koopa_raw_basic_block_t& raw_bb) {
  if (raw_bb->name) {
    // TODO: this way is pretty hacky
    code_stream << std::string(raw_bb->name).substr(1) + ":" << std::endl;
  }
  // visit all the instructions
  assert(raw_bb->insts.kind == KOOPA_RSIK_VALUE);
  for (int i = 0; i < raw_bb->insts.len; ++i) {
    auto ptr = raw_bb->insts.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr));
  }
}

void GenASMVisitor::visit(const koopa_raw_store_t& store) {
  /**1. load operand to register using prepareOperand
   * 2. store register to stack_ofset(sp) that binded to dest
   *
   * for examples:
   * %ptr0 = getelemptr @arr_2, 0
   * store 1, %ptr0
   * @arr_2 at 24(sp)
   * %ptr0 at 20(sp) with content 24(sp)
   *
   * li t0, 1
   * lw t1, 20(sp)
   * sw t0, 0(t1)
   */
  // auto prepareOperandVisitor = PrepareOperandVisitor(&func_stack, &reg_pool);
  // prepareOperandVisitor.visit(store.value);
  // code_stream << prepareOperandVisitor.asm_code;
  // auto& load_reg_name = prepareOperandVisitor.load_reg_name;

  auto src_visitor = PrepareOperandVisitor(&func_stack, &reg_pool);
  src_visitor.visit(store.value);
  code_stream << src_visitor.asm_code;
  auto& tmp_reg_0 = src_visitor.load_reg_name;

  if (func_stack.find(store.dest)) {
    int offset = func_stack.get_offset(store.dest);
    if(store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR || store.dest->kind.tag == KOOPA_RVT_GET_PTR) {
      auto tmp_reg_1 = reg_pool.getReg();
      if (offset < 2048 && offset >= -2048) {
        code_stream << "  lw " + tmp_reg_1 + ", " +
                           std::to_string(offset) + "(sp)"
                    << std::endl;
        code_stream << "  sw " + tmp_reg_0 + ", 0(" + tmp_reg_1 + ")"
                    << std::endl;
      } else {
        auto tmp_reg = reg_pool.getReg();
        code_stream << "  li " + tmp_reg + ", " + std::to_string(offset)
                    << std::endl;
        code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
        code_stream << "  lw " + tmp_reg_1 + ", 0(" + tmp_reg + ")" << std::endl;
        code_stream << "  sw " + tmp_reg_0 + ", 0(" + tmp_reg_1 + ")"
                    << std::endl;
        reg_pool.freeReg(tmp_reg);
      }
      reg_pool.freeReg(tmp_reg_1);
    } else if(store.dest->kind.tag == KOOPA_RVT_ALLOC){
      if (offset < 2048 && offset >= -2048) {
        code_stream << "  sw " + tmp_reg_0 + ", " + std::to_string(offset) +
                           "(sp)"
                    << std::endl;
      } else {
        auto tmp_reg = reg_pool.getReg();
        code_stream << "  li " + tmp_reg + ", " + std::to_string(offset)
                    << std::endl;
        code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
        code_stream << "  sw " + tmp_reg_0 + ", 0(" + tmp_reg + ")" << std::endl;
        reg_pool.freeReg(tmp_reg);
      }
    } else {
      assert(0);
    }
    
  } else if (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    /**
     * for examples:
     * global @a_1 = alloc i32, 10
     * store %1, @a_1
     * ===============
     * la t1, a_1
     * sw t0, 0(t1)
     */
    auto tmp_reg_1 = reg_pool.getReg();
    code_stream << "  la " + tmp_reg_1 + ", " +
                       std::string(store.dest->name).substr(1)
                << std::endl;
    code_stream << "  sw " + tmp_reg_0 + ", 0(" + tmp_reg_1 + ")" << std::endl;
    reg_pool.freeReg(tmp_reg_1);

  } else {
    assert(0);
  }

  // reg_pool.freeReg(tmp_reg_0);
}

void GenASMVisitor::visit(const koopa_raw_return_t& ret) {
  if (ret.value) {
    auto prepareOperandVisitor = PrepareOperandVisitor(&func_stack, &reg_pool);
    prepareOperandVisitor.set_load_reg_name("a0");
    prepareOperandVisitor.visit(ret.value);
    code_stream << prepareOperandVisitor.asm_code;
    // std::cout<<"ret value kind tag: "<<ret.value->kind.tag<<std::endl;
    // if(ret.value->kind.tag == KOOPA_RVT_GET_ELEM_PTR || ret.value->kind.tag
    // == KOOPA_RVT_GET_PTR) {
    //   code_stream << "  lw a0, 0(a0)" << std::endl;
    // }
  }

  // recover ra if needed
  if (func_stack.has_ra) {
    // int offset = stack_offset - 4;
    int offset = func_stack.get_offset_ra();
    if (offset < 2048 && offset >= -2048) {
      code_stream << "  lw ra, " + std::to_string(offset) + "(sp)" << std::endl;
    } else {
      code_stream << "  li t0, " + std::to_string(offset) << std::endl;
      code_stream << "  add t0, sp, t0" << std::endl;
      code_stream << "  lw ra, 0(t0)" << std::endl;
    }
  }
  int stack_size = func_stack.size;
  if (0 < stack_size && stack_size < 2048) {
    code_stream << "  addi sp, sp, " + std::to_string(stack_size) << std::endl;
  } else if (stack_size >= 2048) {
    code_stream << "  li t0, " + std::to_string(stack_size) << std::endl;
    code_stream << "  add sp, sp, t0" << std::endl;
  }
  stack_size = 0;
  code_stream << "  ret" << std::endl;
}

void GenASMVisitor::store_func_stack(const koopa_raw_value_t& value,
                                     std::string reg_name) {
  // assert(func_stack.find(value));
  if (func_stack.find(value) == false) {
    func_stack.insert(value);
  }
  int offset = func_stack.get_offset(value);
  if (offset < 2048 && offset >= -2048) {
    code_stream << "  sw " + reg_name + ", " + std::to_string(offset) + "(sp)"
                << std::endl;
  } else {
    auto tmp_reg = reg_pool.getReg();
    code_stream << "  li " + tmp_reg + ", " + std::to_string(offset)
                << std::endl;
    code_stream << "  add " + tmp_reg + ", sp, " + tmp_reg << std::endl;
    code_stream << "  sw " + reg_name + ", 0(" + tmp_reg + ")" << std::endl;
    reg_pool.freeReg(tmp_reg);
  }
}

};  // namespace KOOPA