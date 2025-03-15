#pragma once

#include"ir.hpp"
#include <iostream>
#include <string>
#include "stack.hpp"
#include <cassert>
#include "prepareOperand.hpp"
#include <unordered_map>

namespace KOOPA { 

class GenASMVisitor : public Visitor {
 public:


  std::unique_ptr<std::string> asm_code = std::make_unique<std::string>();
  int stack_offset = 0;
  int stack_size = 0;
  // key is the address(converted to size_t) of the value
  // value is the stack offset
  std::unordered_map<size_t, int> value_to_offset;


  void visit(Program& program) override {
    asm_code->append("  .text\n");
    std::cout<<"start to generate global variable definition\n";
    program.values->accept(*this);
    std::cout<<"start to generate function definition\n";
    program.funcs->accept(*this);
  }
  
  void visit(Slice& slice) override {
    for (auto& item : slice.items) {
      item->accept(*this);
    }
  }

  void visit(Function& func) override {
    std::cout<<"gen asm for function "<<func.name<<"\n";
    auto func_name = func.name.substr(1);
    auto stack_calculator = StackCalculatorVisitor();
    func.accept(stack_calculator);
    stack_size = stack_calculator.stack_size;
    assert(stack_size % 16 == 0);
    std::cout<<"stack size "<<stack_size<<"\n";
    stack_offset = 0;
    // stack_offset = stack_calculator.stack_offset;

    // start to generate asm code
    asm_code->append("  .global " + func_name + "\n" + func_name + ":\n");
    if(0 < stack_size  && stack_size < 2048) {
      asm_code->append("  addi sp, sp, -" + std::to_string(stack_size) + "\n");
    } else if (stack_size >= 2048) {
      asm_code->append("  li t0, -" + std::to_string(stack_size) + "\n");
      asm_code->append("  add sp, sp, t0\n");
    }
    func.bbs->accept(*this);
  }

  void visit(BasicBlock& bb) override {
    std::cout<<"gen asm for basic block \n";
    if(bb.name.size() > 0) {
      // asm_code->append(std::string(block->name)+":\n");
      // TODO: 如何处理带名字的block??
    }
    bb.insts->accept(*this);
  }
  
  void visit(LoadValue& value) override {
    /**1. load value.src to register 
     * 2. bind value to stack_offset
     * 3. store register to stack_offset(sp)
     * 4. move stack_offset
    */
    auto prepare_op_visitor = PrepareOperandVisitor(&value_to_offset);
    value.src->accept(prepare_op_visitor);
    asm_code->append(*prepare_op_visitor.prepare_asm_code);
    auto& load_reg_name = prepare_op_visitor.load_reg_name;
    value_to_offset[reinterpret_cast<size_t>(&value)] = stack_offset;
    auto dst_name = std::to_string(stack_offset) + "(sp)";
    asm_code->append("  sw " + load_reg_name + ", " + dst_name + "\n");
    stack_offset += 4;
  }
  
  void visit(StoreValue& value) override {
    /**1. load operand to register using prepareOperand 
     * 2. store register to stack_ofset(sp) that binded to dest
    */
    auto prepare_op_visitor = PrepareOperandVisitor(&value_to_offset);
    value.value->accept(prepare_op_visitor);
    asm_code->append(*prepare_op_visitor.prepare_asm_code);
    auto& load_reg_name = prepare_op_visitor.load_reg_name;
    auto dst_name = std::to_string(value_to_offset[reinterpret_cast<size_t>(value.dest.get())]) + "(sp)";
    asm_code->append("  sw " + load_reg_name + ", " + dst_name + "\n");
  }

  void visit(LocalAllocValue& value) override {
    /**1. bind value to stack_offset
     * 2. move stack_offset
     */
    std::cout<<"gen asm for local alloc\n";
    value_to_offset[reinterpret_cast<size_t>(&value)] = stack_offset;
    stack_offset += 4;
  }

  void visit(BinaryOp& op_value) override {
    /**1. load op_value.lhs and rhs to register
     * 2. do the operation
     * 3. store the result to stack_offset
     * 4. bind value to stack_offset
     * 5. move stack_offset
     */
    auto prepare_op_visitor = PrepareOperandVisitor(&value_to_offset);
    op_value.lhs->accept(prepare_op_visitor);
    asm_code->append(*prepare_op_visitor.prepare_asm_code);
    auto& lreg_name = prepare_op_visitor.load_reg_name;
    op_value.rhs->accept(prepare_op_visitor);
    asm_code->append(*prepare_op_visitor.prepare_asm_code);
    auto& rreg_name = prepare_op_visitor.load_reg_name;

    switch (op_value.op) {
      case KOOPA_RBO_NOT_EQ: {
        asm_code->append("  xor " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        asm_code->append("  seqz " + lreg_name + ", " + lreg_name + "\n");
        break;
      }
      case KOOPA_RBO_EQ: {
        asm_code->append("  xor " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        asm_code->append("  snez " + lreg_name + ", " + lreg_name + "\n");
        break;
      }
      case KOOPA_RBO_GT: {
        asm_code->append("  slt " + lreg_name + ", " + rreg_name + ", " + lreg_name + "\n");
        break;
      }
      case KOOPA_RBO_LT: {
        asm_code->append("  slt " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_GE: {
        asm_code->append("  sge " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_LE: {
        asm_code->append("  sge " + lreg_name + ", " + rreg_name + ", " + lreg_name + "\n");
        break;
      }
      case KOOPA_RBO_ADD: {
        asm_code->append("  add " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_SUB: {
        asm_code->append("  sub " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_MUL: {
        asm_code->append("  mul " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_DIV: {
        asm_code->append("  div " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_MOD: {
        asm_code->append("  rem " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_AND: {
        asm_code->append("  and " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_OR: {
        asm_code->append("  or " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      case KOOPA_RBO_XOR: {
        asm_code->append("  xor " + lreg_name + ", " + lreg_name + ", " + rreg_name + "\n");
        break;
      }
      
      default: {
        assert(0);
      }
    }

    value_to_offset[reinterpret_cast<size_t>(&op_value)] = stack_offset;
    auto dst_name = std::to_string(stack_offset) + "(sp)";
    asm_code->append("  sw " + lreg_name + ", " + dst_name + "\n");
    stack_offset += 4;
  }
  
  void visit(ReturnOp& value) override {
    std::cout<<"gen asm for return\n";
    /**1. load value.value to register a0
     * 2. recover stack
     * 3. ret
     */
    std::string a0 = "a0";
    auto prepare_op_visitor = PrepareOperandVisitor(&value_to_offset, &a0);
    value.value->accept(prepare_op_visitor);
    asm_code->append(*prepare_op_visitor.prepare_asm_code);
    if(0 < stack_size && stack_size < 2048) {
      asm_code->append("  addi sp, sp, " + std::to_string(stack_size) + "\n");
    } else if(stack_size >= 2048) {
      asm_code->append("  li t0, " + std::to_string(stack_size) + "\n");
      asm_code->append("  add sp, sp, t0\n");
    }
    asm_code->append("  ret\n");
  }

};
















};  // namespace KOOPA