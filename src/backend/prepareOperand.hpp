#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include "regpool.hpp"
#include <vector>


namespace KOOPA {

class PrepareOperandVisitor: public Visitor {
 public:
  std::string asm_code; 
  std::string load_reg_name;
  std::unordered_map<koopa_raw_value_t, int>* value_to_offset;

  
  /**
   * currently when prepareOperandVisitor return visiting
   * it should only keep load_reg_name regiser
   * other temporary register used during loading should be freed
   */
  RegPool* reg_pool;

  ~PrepareOperandVisitor(){
    if(load_reg_name[0] == 't'){
      reg_pool->freeReg(load_reg_name);
    }
  }


  PrepareOperandVisitor(std::unordered_map<koopa_raw_value_t, int>* _value_to_offset, RegPool* _reg_pool) {
    value_to_offset = _value_to_offset;
    reg_pool = _reg_pool;
    load_reg_name = reg_pool->getReg();
  }

  /**
   * 1. free previous acquired register(load_reg_name)
   * 2. set load_reg_name to new register
   * 3. get new register if it is temporary register
   */
  void set_load_reg_name(const std::string& name){
    reg_pool->freeReg(load_reg_name);
    load_reg_name = name;
    if(name[0] == 't'){
      reg_pool->getReg(name);
    }
  }

  void visit(const koopa_raw_value_t& value) override {
    if(value->kind.tag == KOOPA_RVT_INTEGER) {
      auto integer = value->kind.data.integer.value;
      asm_code.append("  li " + load_reg_name + ", " + std::to_string(integer) + "\n");
    } else {
      if(value_to_offset->find(value) == value_to_offset->end()) {
        throw std::runtime_error("Value not found in value_to_offset");
      }
      auto stack_offset = (*value_to_offset)[value];
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
};





};