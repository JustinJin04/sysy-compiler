// conver to c++ version
#pragma once
#include <koopa.h>

#include <memory>
#include <vector>
#include <iostream>
#include <unordered_map>

namespace KOOPA {

// Forward declaration
class Visitor;
class Slice;
class SliceItem;
class Type;
class INT32Type;
class UnitType;
class ArrayType;
class PointerType;
class FunctionType;
class Function;
class BasicBlock;
class Value;
class IntegerValue;
class ZeroInitValue;
class LocalAllocValue;
class AggregateValue;
class FuncArgRefValue;
class BlockArgRef;
class GlobalAllocValue;
class LoadValue;
class StoreValue;
class GetPtrValue;
class GetElemPtrValue;
class BinaryOp;
class BranchOp;
class JumpOp;
class CallOp;
class ReturnOp;



// Declaration
class Base {
 public:
  virtual ~Base() = default;

  virtual void accept(Visitor& v) = 0;
};

class Program : public Base {
 public:
  std::unique_ptr<Slice> values;
  std::unique_ptr<Slice> funcs;

  void accept(Visitor& v) override;
};

class Slice : public Base {
 public:
  std::vector<std::shared_ptr<SliceItem>> items;

  void accept(Visitor& v) override;
};

class SliceItem : public Base {
 public:
  virtual void accept(Visitor& v) = 0;
};

class Type : public SliceItem {
 public:
  virtual void accept(Visitor& v) = 0;
};

class INT32Type : public Type {
 public:
  void accept(Visitor& v) override;
};

class UnitType : public Type {
 public:
  void accept(Visitor& v) override;
};

class ArrayType : public Type {
 public:
  std::unique_ptr<Type> base;
  int len;

  void accept(Visitor& v) override;
};

class PointerType : public Type {
 public:
  std::unique_ptr<Type> base;

  void accept(Visitor& v) override;
};

class FunctionType : public Type {
 public:
  std::unique_ptr<Slice> params;
  std::unique_ptr<Type> ret;

  void accept(Visitor& v) override;
};

class Function : public SliceItem {
 public:
  std::unique_ptr<Type> type;
  std::string name;
  std::unique_ptr<Slice> params;
  std::unique_ptr<Slice> bbs;

  void accept(Visitor& v) override;
};

class BasicBlock : public SliceItem {
 public:
  std::string name{};
  std::unique_ptr<Slice> params;
  // std::vector<std::shared_ptr<SliceItem>> used_by;
  std::weak_ptr<Slice> used_by;
  // std::shared_ptr<Slice> used_by;
  std::unique_ptr<Slice> insts;

  void accept(Visitor& v) override;
};

class Value : public SliceItem {
 public:
  std::unique_ptr<Type> ty;
  std::string name;
  std::weak_ptr<Slice> used_by;
  // std::shared_ptr<Slice> used_by;

  virtual void accept(Visitor& v) = 0;      // since we don't visit value directly
                                            // instead we visit its subclass
};

class IntegerValue : public Value {
 public:
  int value;

  void accept(Visitor& v) override;
};

class ZeroInitValue : public Value {
 public:
  void accept(Visitor& v) override;
};

class LocalAllocValue : public Value {
 public:

  void accept(Visitor& v) override;
};

class AggregateValue : public Value {
 public:
  std::unique_ptr<Slice> elems;

  void accept(Visitor& v) override;
};

class FuncArgRefValue : public Value {
 public:
  size_t index;

  void accept(Visitor& v) override;
};

class BlockArgRef : public Value {
 public:
  size_t index;

  void accept(Visitor& v) override;
};

class GlobalAllocValue : public Value {
 public:
  std::unique_ptr<Value> init;

  void accept(Visitor& v) override;
};

class LoadValue : public Value {
 public:
  std::unique_ptr<Value> src;

  void accept(Visitor& v) override;
};

class StoreValue : public Value {
 public:
  std::unique_ptr<Value> value;
  std::unique_ptr<Value> dest;

  void accept(Visitor& v) override;
};

class GetPtrValue : public Value {
 public:
  std::unique_ptr<Value> src;
  std::unique_ptr<Value> index;

  void accept(Visitor& v) override;
};

class GetElemPtrValue : public Value {
 public:
  std::unique_ptr<Value> src;
  std::unique_ptr<Value> index;

  void accept(Visitor& v) override;
};

/**
 * Here we don't inherit BinaryOp for the sake of simplicity
 * Just use switch case in visitor according to th op
 */
class BinaryOp : public Value {
 public:
  koopa_raw_binary_op_t op;
  std::unique_ptr<Value> lhs;
  std::unique_ptr<Value> rhs;

  void accept(Visitor& v) override;
};

/**
 * Conditional branch
 */
class BranchOp : public Value {
 public:
  std::unique_ptr<Value> cond;
  std::unique_ptr<BasicBlock> true_bb;
  std::unique_ptr<BasicBlock> false_bb;
  std::unique_ptr<Slice> true_args;
  std::unique_ptr<Slice> false_args;

  void accept(Visitor& v) override;
};

/**
 * Unconditional jump
 */
class JumpOp : public Value {
 public:
  std::unique_ptr<BasicBlock> target;
  std::unique_ptr<Slice> args;

  void accept(Visitor& v) override;
};

/**
 * Function call
 */
class CallOp : public Value {
 public:
  std::unique_ptr<Function> callee;
  std::unique_ptr<Slice> args;

  void accept(Visitor& v) override;
};

/**
 * Function return
 */
class ReturnOp : public Value {
 public:
  std::unique_ptr<Value> value;

  void accept(Visitor& v) override;
};


class Visitor {
 public:
  virtual ~Visitor() = default;

  virtual void visit(Program& program) {
    throw std::runtime_error("Program not implemented");
  }
  virtual void visit(Slice& slice) {
    throw std::runtime_error("Slice not implemented");
  }
  // virtual void visit(Type& type) = 0;
  virtual void visit(INT32Type& type) {
    throw std::runtime_error("INT32Type not implemented");
  }
  virtual void visit(UnitType& type) {
    throw std::runtime_error("UnitType not implemented");
  }
  virtual void visit(ArrayType& type) {
    throw std::runtime_error("ArrayType not implemented");
  }
  virtual void visit(PointerType& type) {
    throw std::runtime_error("PointerType not implemented");
  }
  virtual void visit(FunctionType& type) {
    throw std::runtime_error("FunctionType not implemented");
  }
  virtual void visit(Function& func) {
    throw std::runtime_error("Function not implemented");
  }
  virtual void visit(BasicBlock& bb) {
    throw std::runtime_error("BasicBlock not implemented");
  }
  // virtual void visit(Value& value) = 0;
  virtual void visit(IntegerValue& value) {
    throw std::runtime_error("IntegerValue not implemented");
  }
  virtual void visit(ZeroInitValue& value) {
    throw std::runtime_error("ZeroInitValue not implemented");
  }
  virtual void visit(LocalAllocValue& value) {
    throw std::runtime_error("LocalAllocValue not implemented");
  }
  virtual void visit(AggregateValue& value) {
    throw std::runtime_error("AggregateValue not implemented");
  }
  virtual void visit(FuncArgRefValue& value) {
    throw std::runtime_error("FuncArgRefValue not implemented");
  }
  virtual void visit(BlockArgRef& value) {
    throw std::runtime_error("BlockArgRef not implemented");
  }
  virtual void visit(GlobalAllocValue& value) {
    throw std::runtime_error("GlobalAllocValue not implemented");
  }
  virtual void visit(LoadValue& value) {
    throw std::runtime_error("LoadValue not implemented");
  }
  virtual void visit(StoreValue& value) {
    throw std::runtime_error("StoreValue not implemented");
  }
  virtual void visit(GetPtrValue& value) {
    throw std::runtime_error("GetPtrValue not implemented");
  }
  virtual void visit(GetElemPtrValue& value) {
    throw std::runtime_error("GetElemPtrValue not implemented");
  }
  virtual void visit(BinaryOp& value) {
    throw std::runtime_error("BinaryOp not implemented");
  }
  virtual void visit(BranchOp& value) {
    throw std::runtime_error("BranchOp not implemented");
  }
  virtual void visit(JumpOp& value) {
    throw std::runtime_error("JumpOp not implemented");
  }
  virtual void visit(CallOp& value) {
    throw std::runtime_error("CallOp not implemented");
  }
  virtual void visit(ReturnOp& value) {
    throw std::runtime_error("ReturnOp not implemented");
  }
};

Base* convert_to_cpp(const koopa_raw_program_t& raw);
Base* convert_to_cpp(const koopa_raw_slice_t& raw);
Base* convert_to_cpp(const koopa_raw_type_t& raw);
Base* convert_to_cpp(const koopa_raw_function_t& raw);
Base* convert_to_cpp(const koopa_raw_basic_block_t& raw);
Base* convert_to_cpp(const koopa_raw_value_t& raw);


};  // namespace KOOPA