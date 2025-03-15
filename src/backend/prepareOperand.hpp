#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "ir.hpp"

namespace KOOPA {

class PrepareOperandVisitor : public Visitor {
 public:
  PrepareOperandVisitor(std::unordered_map<size_t, int>* _value_to_offset,
                        std::string* _should_load_reg_name = nullptr)
      : value_to_offset(_value_to_offset) {
    prepare_asm_code = std::make_unique<std::string>();
    if(_should_load_reg_name) {
      load_reg_name = *_should_load_reg_name;
    }
  }
  std::unique_ptr<std::string> prepare_asm_code;
  // std::unordered_map<Value*, std::string>* value_to_name;
  std::unordered_map<size_t, int>* value_to_offset;
  std::string load_reg_name;


  void visit(IntegerValue& value) override {
    if(load_reg_name.empty()) {
      load_reg_name = "t0";
    } else if (load_reg_name == "t0") {
      load_reg_name = "t1";
    }
    prepare_asm_code->append("  li " + load_reg_name + ", " + std::to_string(value.value) + "\n");
  }

  void visit(StoreValue& value) override {
    int stack_offset = (*value_to_offset)[reinterpret_cast<size_t>(&value)];
    prepare_asm_code->append("  lw t0, " + std::to_string(stack_offset) + "(sp)\n");
    load_reg_name = "t0";
  }

  void visit(LoadValue& value) override {
    int stack_offset = (*value_to_offset)[reinterpret_cast<size_t>(&value)];
    prepare_asm_code->append("  lw t0, " + std::to_string(stack_offset) + "(sp)\n");
    load_reg_name = "t0";
  }

  void visit(BinaryOp& value) override {
    int stack_offset = (*value_to_offset)[reinterpret_cast<size_t>(&value)];
    prepare_asm_code->append("  lw t0, " + std::to_string(stack_offset) + "(sp)\n");
    load_reg_name = "t0";
  }

  void visit(LocalAllocValue& value) override {
    int stack_offset = (*value_to_offset)[reinterpret_cast<size_t>(&value)];
    prepare_asm_code->append("  lw t0, " + std::to_string(stack_offset) + "(sp)\n");
    load_reg_name = "t0";
  }


};

};  // namespace KOOPA