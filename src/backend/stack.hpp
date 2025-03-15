#pragma once

#include "ir.hpp"
#include <iostream>

namespace KOOPA {

class StackCalculatorVisitor : public Visitor {
 public:
  int stack_size = 0;

  void visit(Function& func) override {
    stack_size = 0;
    func.bbs->accept(*this);
    stack_size = (stack_size + 15) / 16 * 16;
  }

  void visit(Slice& slice) override {
    for (auto& item : slice.items) {
      item->accept(*this);
    }
  }

  void visit(BasicBlock& bb) override {
    bb.insts->accept(*this);
  }

  void visit(LocalAllocValue& value) override {
    stack_size += 4;
  }

  void visit(BinaryOp& value) override {
    stack_size += 4;
  }

  void visit(LoadValue& value) override {
    stack_size += 4;
  }
  void visit(ReturnOp& value) override {
    // DO NOTHING
  }
  void visit(StoreValue& value) override {
    // DO NOTHING
  }
};



};  // namespace KOOPA