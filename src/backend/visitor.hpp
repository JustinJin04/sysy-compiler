#pragma once

#include <koopa.h>
#include <memory>

namespace KOOPA {

class Visitor {
 public:
  virtual ~Visitor() = default;

  virtual void visit(const koopa_raw_program_t& program) {
    throw std::runtime_error("Program not implemented");
  }

  virtual void visit(const koopa_raw_value_t& value) {
    throw std::runtime_error("Value not implemented");
  }

  virtual void visit(const koopa_raw_function_t& func) {
    throw std::runtime_error("Function not implemented");
  }

  virtual void visit(const koopa_raw_basic_block_t& bb) {
    throw std::runtime_error("BasicBlock not implemented");
  }

  // virtual void visit(const koopa_raw_integer_t& value) {
  //   throw std::runtime_error("IntegerValue not implemented");
  // }

  virtual void visit(const koopa_raw_return_t& value) {
    throw std::runtime_error("ReturnOp not implemented");
  }

  // virtual void visit(const koopa_raw_binary_t& value) {
  //   throw std::runtime_error("BinaryOp not implemented");
  // }

  virtual void visit(const koopa_raw_store_t& value) {
    throw std::runtime_error("StoreOp not implemented");
  }

  // virtual void visit(const koopa_raw_load_t& value) {
  //   throw std::runtime_error("LoadOp not implemented");
  // }

};




























}; // namespace KOOPA