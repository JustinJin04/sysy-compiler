#include "genASM.hpp"
#include <cassert>
#include "prepareOperand.hpp"
#include "stack.hpp"
#include <string>


namespace KOOPA {

void GenASMVisitor::visit(const koopa_raw_program_t& raw) {
  asm_code.append("  .text\n");

  // 遍历global value
  assert(raw.values.kind == KOOPA_RSIK_VALUE);
  for(int i=0;i<raw.values.len;++i){
    auto ptr = raw.values.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr));
  }

  // 遍历global func
  assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
  for(int i=0;i<raw.funcs.len;++i){
    auto ptr = raw.funcs.buffer[i];
    visit(reinterpret_cast<koopa_raw_function_t>(ptr));
  }
}

void GenASMVisitor::visit(const koopa_raw_value_t& raw_value) {
  const auto &kind = raw_value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_ALLOC: {
      /**1. bind value to stack_offset
       * 2. move stack_offset
       */ 
      value_to_offset[raw_value] = stack_offset;
      stack_offset += 4;
      break;
    }
    case KOOPA_RVT_LOAD: {
      /**1. load value.src to register 
       * 2. bind value to stack_offset
       * 3. store register to stack_offset(sp)
       * 4. move stack_offset
      */
      auto prepare_op_visitor = PrepareOperandVisitor(&value_to_offset);
      prepare_op_visitor.visit(kind.data.load.src);
      asm_code.append(prepare_op_visitor.asm_code);
      auto& load_reg_name = prepare_op_visitor.load_reg_name;
      value_to_offset[raw_value] = stack_offset;
      auto dst_name = std::to_string(stack_offset) + "(sp)";
      asm_code.append("  sw " + load_reg_name + ", " + dst_name + "\n");
      stack_offset += 4;
      break;
    }
    case KOOPA_RVT_STORE: {
      visit(kind.data.store);
      break;
    }
    case KOOPA_RVT_RETURN: {
      visit(kind.data.ret);
      break;
    }
    case KOOPA_RVT_BINARY: {
      auto op = kind.data.binary.op;
      auto lhs = kind.data.binary.lhs;
      auto rhs = kind.data.binary.rhs;

      auto prepareOperandVisitor = PrepareOperandVisitor(&value_to_offset);
      prepareOperandVisitor.set_load_reg_name("t0");
      prepareOperandVisitor.visit(lhs);
      prepareOperandVisitor.set_load_reg_name("t1");
      prepareOperandVisitor.visit(rhs);
      asm_code.append(prepareOperandVisitor.asm_code);

      switch (op) {
        case KOOPA_RBO_NOT_EQ: {
          // asm_code.append("  snez t0, t0, t1\n");
          asm_code.append("  xor t0, t0, t1\n");
          asm_code.append("  snez t0, t0\n");
          break;
        }
        case KOOPA_RBO_EQ: {
          // asm_code.append("  seqz t0, t0, t1\n");
          asm_code.append("  xor t0, t0, t1\n");
          asm_code.append("  seqz t0, t0\n");
          break;
        }
        case KOOPA_RBO_GT: {
          asm_code.append("  sgt t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_LT: {
          asm_code.append("  slt t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_GE: {
          // asm_code.append("  sge t0, t0, t1\n");
          asm_code.append("  slt t0, t0, t1\n");
          asm_code.append("  seqz t0, t0\n");
          break;
        }
        case KOOPA_RBO_LE: {
          // asm_code.append("  sle t0, t0, t1\n");
          asm_code.append("  sgt t0, t0, t1\n");
          asm_code.append("  seqz t0, t0\n");
          break;
        }
        case KOOPA_RBO_ADD: {
          asm_code.append("  add t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_SUB: {
          asm_code.append("  sub t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_MUL: {
          asm_code.append("  mul t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_DIV: {
          asm_code.append("  div t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_MOD: {
          asm_code.append("  rem t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_AND: {
          asm_code.append("  and t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_OR: {
          asm_code.append("  or t0, t0, t1\n");
          break;
        }
        case KOOPA_RBO_XOR: {
          asm_code.append("  xor t0, t0, t1\n");
          break;
        }
        default:{
          assert(false);
        }
      }

      value_to_offset[raw_value] = stack_offset;
      auto dst_name = std::to_string(stack_offset) + "(sp)";
      asm_code.append("  sw t0, " + dst_name + "\n");
      stack_offset += 4;
      break;
    }

    default:{
      assert(false);
    }
  }
}

void GenASMVisitor::visit(const koopa_raw_function_t& raw_func) {
  auto func_name = std::string(raw_func->name).substr(1);
  auto stack_calculator = StackCalculatorVisitor();
  stack_calculator.visit(raw_func);
  stack_size = stack_calculator.stack_size;
  assert(stack_size % 16 == 0);
  stack_offset = 0;

  // start to generate asm code
  asm_code.append("  .global " + func_name + "\n" + func_name + ":\n");
  if(0 < stack_size  && stack_size < 2048) {
    asm_code.append("  addi sp, sp, -" + std::to_string(stack_size) + "\n");
  } else if (stack_size >= 2048) {
    asm_code.append("  li t0, -" + std::to_string(stack_size) + "\n");
    asm_code.append("  add sp, sp, t0\n");
  }
  for(int i=0;i<raw_func->bbs.len;++i){
    auto ptr = raw_func->bbs.buffer[i];
    visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
  }

  assert(stack_size == 0);

}

void GenASMVisitor::visit(const koopa_raw_basic_block_t& raw_bb) {
  if(raw_bb->name) {
    // asm_code->append(std::string(block->name)+":\n");
    // TODO: 如何处理带名字的block??
  }
  // visit all the instructions
  assert(raw_bb->insts.kind == KOOPA_RSIK_VALUE);
  for(int i=0;i<raw_bb->insts.len;++i){
    auto ptr = raw_bb->insts.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr));
  }
}

void GenASMVisitor::visit(const koopa_raw_store_t& load) {
  /**1. load operand to register using prepareOperand 
   * 2. store register to stack_ofset(sp) that binded to dest
  */
  auto prepareOperandVisitor = PrepareOperandVisitor(&value_to_offset);
  prepareOperandVisitor.visit(load.value);
  asm_code.append(prepareOperandVisitor.asm_code);
  auto& load_reg_name = prepareOperandVisitor.load_reg_name;
  auto dst_name = std::to_string(value_to_offset[load.dest]) + "(sp)";
  asm_code.append("  sw " + load_reg_name + ", " + dst_name + "\n");
}

void GenASMVisitor::visit(const koopa_raw_return_t& ret) {
  if (ret.value) {
    auto prepareOperandVisitor = PrepareOperandVisitor(&value_to_offset);
    prepareOperandVisitor.set_load_reg_name("a0");
    prepareOperandVisitor.visit(ret.value);
    asm_code.append(prepareOperandVisitor.asm_code);
  }
  if(0 < stack_size  && stack_size < 2048) {
    asm_code.append("  addi sp, sp, " + std::to_string(stack_size) + "\n");
  } else if (stack_size >= 2048) {
    asm_code.append("  li t0, " + std::to_string(stack_size) + "\n");
    asm_code.append("  add sp, sp, t0\n");
  }
  stack_size = 0;
  asm_code.append("  ret\n");
}















};  // namespace KOOPA