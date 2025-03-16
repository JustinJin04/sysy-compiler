#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace KOOPA {

class GenASMVisitor : public Visitor {
 public:
  
  std::string asm_code;

  int stack_offset = 0;
  int stack_size = 0;

  std::unordered_map<koopa_raw_value_t, int> value_to_offset;

  // used to add label after ret
  // int ret_label_counter = 0;


  void visit(const koopa_raw_program_t& program) override;
  void visit(const koopa_raw_value_t& value) override;
  void visit(const koopa_raw_function_t& func) override;
  void visit(const koopa_raw_basic_block_t& bb) override;


  // void visit(const koopa_raw_integer_t& value) override;
  void visit(const koopa_raw_return_t& value) override;
  // void visit(const koopa_raw_binary_t& value) override;

  void visit(const koopa_raw_store_t& value) override;
  // void visit(const koopa_raw_load_t& value) override;
};



























};  // namespace KOOPA