#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include "regpool.hpp"
#include "stack.hpp"
#include <fstream>

namespace KOOPA {

class GenASMVisitor : public Visitor {
 public:
  std::ofstream code_stream;

  FuncStack func_stack;

  RegPool reg_pool;

  GenASMVisitor(const std::string& output_file)
      : code_stream(output_file, std::ios::out | std::ios::trunc),
        func_stack(16),
        reg_pool(7) {
    if (!code_stream.is_open()) {
      throw std::runtime_error("Failed to open output file");
    }
  }

  ~GenASMVisitor() { code_stream.close(); }

  void store_func_stack(const koopa_raw_value_t& value, std::string reg_name);

  void visit(const koopa_raw_program_t& program) override;
  void visit(const koopa_raw_value_t& value) override;
  void visit(const koopa_raw_function_t& func) override;
  void visit(const koopa_raw_basic_block_t& bb) override;

  void visit(const koopa_raw_return_t& value) override;

  void visit(const koopa_raw_store_t& value) override;
};

};  // namespace KOOPA