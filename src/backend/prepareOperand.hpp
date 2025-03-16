#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <memory>
#include <string>
#include <unordered_map>


namespace KOOPA {

class PrepareOperandVisitor: public Visitor {
 public:
  std::string asm_code; 
  std::string load_reg_name;
  std::unordered_map<koopa_raw_value_t, int>* value_to_offset;


  PrepareOperandVisitor(std::unordered_map<koopa_raw_value_t, int>* _value_to_offset) {
    value_to_offset = _value_to_offset;
    load_reg_name = "t0";
  }

  void set_load_reg_name(const std::string& name){
    load_reg_name = name;
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
        asm_code.append("  li t0, " + std::to_string(stack_offset) + "\n");
        asm_code.append("  add t0, sp, t0\n");
        asm_code.append("  lw " + load_reg_name + ", 0(t0)\n");
      }
      
    }
  }
};





};