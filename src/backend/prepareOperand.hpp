#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include "regpool.hpp"
#include <vector>
#include "stack.hpp"


namespace KOOPA {

class PrepareOperandVisitor: public Visitor {
 public:
  std::string asm_code; 
  std::string load_reg_name;
  // std::unordered_map<koopa_raw_value_t, int>* value_to_offset;
  // int stack_size;
  FuncStack* stack;

  
  /**
   * currently when prepareOperandVisitor return visiting
   * it should only keep load_reg_name regiser
   * other temporary register used during loading should be freed
   */
  RegPool* reg_pool;

  ~PrepareOperandVisitor(){
    if(load_reg_name != "" && load_reg_name[0] == 't'){
      reg_pool->freeReg(load_reg_name);
    }
  }


  PrepareOperandVisitor(FuncStack* _stack, RegPool* _reg_pool) {
    stack = _stack;
    reg_pool = _reg_pool;
    load_reg_name = reg_pool->getReg();
  }

  /**
   * 1. free previous acquired register(load_reg_name)
   * 2. set load_reg_name to new register
   * 3. get new register if it is temporary register
   */
  void set_load_reg_name(const std::string& name){
    if(name == load_reg_name){
      return;
    }
    if(load_reg_name != "" && load_reg_name[0] == 't'){
      reg_pool->freeReg(load_reg_name);
    }
    load_reg_name = name;
    if(name[0] == 't'){
      reg_pool->getReg(name);
    }
  }
  /**
   * 1. release current hold register (maybe not temp register, like a0-a7)
   * 2. acquire new temp register
   */
  void reset_load_reg_name(){
    if(load_reg_name != "" && load_reg_name[0] == 't'){
      reg_pool->freeReg(load_reg_name);
    }
    load_reg_name = reg_pool->getReg();
  }

  void visit(const koopa_raw_value_t& value) override {
    switch(value->kind.tag){
      case KOOPA_RVT_INTEGER: {
        auto integer = value->kind.data.integer.value;
        asm_code.append("  li " + load_reg_name + ", " + std::to_string(integer) + "\n");
        break;
      }
      case KOOPA_RVT_FUNC_ARG_REF: {
        int index = value->kind.data.func_arg_ref.index;
        if (index < 8) {
          set_load_reg_name("a" + std::to_string(index));
          return;
        } else {
          int stack_offset = stack->size + (index - 8) * 4;
          if(stack_offset < 2048){
            asm_code.append("  lw " + load_reg_name + ", " + std::to_string(stack_offset) + "(sp)\n");
          } else {
            auto tmp_reg = reg_pool->getReg();
            asm_code.append("  li " + tmp_reg + ", " + std::to_string(stack_offset) + "\n");
            // asm_code.append("  li t0, " + std::to_string(stack_offset) + "\n");
            asm_code.append("  add " + tmp_reg + ", sp, " + tmp_reg + "\n");
            // asm_code.append("  add t0, sp, t0\n");
            asm_code.append("  lw " + load_reg_name + ", 0(" + tmp_reg + ")\n");
            // asm_code.append("  lw " + load_reg_name + ", 0(t0)\n");
            reg_pool->freeReg(tmp_reg);
          }
        }
        break;
      }
      case KOOPA_RVT_GLOBAL_ALLOC: {
        /**
         * la t0, x
         * lw t0, 0(t0)
         */
        asm_code.append("  la " + load_reg_name + ", " + std::string(value->name).substr(1) + "\n");
        asm_code.append("  lw " + load_reg_name + ", 0(" + load_reg_name + ")\n");
        break;
      }
      default:{
        int stack_offset = stack->get_offset(value);
        if(stack_offset < 2048){
          asm_code.append("  lw " + load_reg_name + ", " + std::to_string(stack_offset) + "(sp)\n");
        } else {
          auto tmp_reg = reg_pool->getReg();
          asm_code.append("  li " + tmp_reg + ", " + std::to_string(stack_offset) + "\n");
          // asm_code.append("  li t0, " + std::to_string(stack_offset) + "\n");
          asm_code.append("  add " + tmp_reg + ", sp, " + tmp_reg + "\n");
          // asm_code.append("  add t0, sp, t0\n");
          asm_code.append("  lw " + load_reg_name + ", 0(" + tmp_reg + ")\n");
          // asm_code.append("  lw " + load_reg_name + ", 0(t0)\n");
          reg_pool->freeReg(tmp_reg);
        }
      }
    }
  }


};





};