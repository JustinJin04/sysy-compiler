#include"ir.hpp"
#include <cassert>

namespace KOOPA {

void Program::accept(Visitor& v) {
  v.visit(*this);
}
void Slice::accept(Visitor& v) {
  v.visit(*this);
}
void INT32Type::accept(Visitor& v) {
  v.visit(*this);
}
void UnitType::accept(Visitor& v) {
  v.visit(*this);
}
void ArrayType::accept(Visitor& v) {
  v.visit(*this);
}
void PointerType::accept(Visitor& v) {
  v.visit(*this);
}
void FunctionType::accept(Visitor& v) {
  v.visit(*this);
}
void Function::accept(Visitor& v) {
  v.visit(*this);
}
void BasicBlock::accept(Visitor& v) {
  v.visit(*this);
}
void IntegerValue::accept(Visitor& v) {
  v.visit(*this);
}
void ZeroInitValue::accept(Visitor& v) {
  v.visit(*this);
}
void LocalAllocValue::accept(Visitor& v) {
  v.visit(*this);
}
void AggregateValue::accept(Visitor& v) {
  v.visit(*this);
}
void FuncArgRefValue::accept(Visitor& v) {
  v.visit(*this);
}
void BlockArgRef::accept(Visitor& v) {
  v.visit(*this);
}
void GlobalAllocValue::accept(Visitor& v) {
  v.visit(*this);
}
void LoadValue::accept(Visitor& v) {
  v.visit(*this);
}
void StoreValue::accept(Visitor& v) {
  v.visit(*this);
}
void GetPtrValue::accept(Visitor& v) {
  v.visit(*this);
}
void GetElemPtrValue::accept(Visitor& v) {
  v.visit(*this);
}
void BinaryOp::accept(Visitor& v) {
  v.visit(*this);
}
void BranchOp::accept(Visitor& v) {
  v.visit(*this);
}
void JumpOp::accept(Visitor& v) {
  v.visit(*this);
}
void CallOp::accept(Visitor& v) {
  v.visit(*this);
}
void ReturnOp::accept(Visitor& v) {
  v.visit(*this);
}


// helper function for downcasting and wrap with unique_ptr
template<typename TARGET>
std::unique_ptr<TARGET> cast_uptr(Base* base) {
  TARGET* target = dynamic_cast<TARGET*>(base);
  if (target == nullptr) {
    throw std::runtime_error("cast_uptr failed");
  }
  return std::unique_ptr<TARGET>(target);
}

template<typename TARGET>
std::shared_ptr<TARGET> cast_shptr(Base* base) {
  TARGET* target = dynamic_cast<TARGET*>(base);
  if (target == nullptr) {
    throw std::runtime_error("cast_uptr failed");
  }
  return std::shared_ptr<TARGET>(target);
}

template<typename TARGET>
std::weak_ptr<TARGET> cast_wptr(Base* base) {
  TARGET* target = dynamic_cast<TARGET*>(base);
  if (target == nullptr) {
    throw std::runtime_error("cast_uptr failed");
  }
  return std::weak_ptr<TARGET>(target);
}





std::unordered_map<size_t, size_t> raw_addr_to_cpp_addr;



Base* convert_to_cpp(const koopa_raw_program_t& raw) {
  std::cout<<"convert raw program to cpp form\n";
  auto program = new Program();
  program->values = cast_uptr<Slice>(convert_to_cpp(raw.values));
  program->funcs = cast_uptr<Slice>(convert_to_cpp(raw.funcs));
  return program;
}

Base* convert_to_cpp(const koopa_raw_slice_t& raw) {
  std::cout<<"convert raw slice to cpp form "<<raw.kind<<"\n";
  auto slice = new Slice();
  switch (raw.kind) {
    case KOOPA_RSIK_TYPE: {
      for(int i=0;i<raw.len;++i){
        slice->items.push_back(
          cast_uptr<SliceItem>(convert_to_cpp(reinterpret_cast<koopa_raw_type_t>(raw.buffer[i])))
        );
      }
      break;
    }
    case KOOPA_RSIK_FUNCTION: {
      for(int i=0;i<raw.len;++i){
        slice->items.push_back(
          cast_shptr<SliceItem>(convert_to_cpp(reinterpret_cast<koopa_raw_function_t>(raw.buffer[i])))
        );
      }
      break;
    }
    case KOOPA_RSIK_BASIC_BLOCK: {
      for(int i=0;i<raw.len;++i){
        slice->items.push_back(
          cast_shptr<SliceItem>(convert_to_cpp(reinterpret_cast<koopa_raw_basic_block_t>(raw.buffer[i])))
        );
      }
      break;
    }
    case KOOPA_RSIK_VALUE: {
      for(int i=0;i<raw.len;++i){
        size_t raw_addr = reinterpret_cast<size_t>(raw.buffer[i]);
        if(raw_addr_to_cpp_addr.find(raw_addr) != raw_addr_to_cpp_addr.end()){
          slice->items.push_back(
            cast_shptr<SliceItem>(reinterpret_cast<Base*>(raw_addr_to_cpp_addr[raw_addr]))
          );
        } else {
          auto value = cast_shptr<SliceItem>(convert_to_cpp(reinterpret_cast<koopa_raw_value_t>(raw.buffer[i])));
          slice->items.push_back(std::move(value));
        }
      }
      break;
    }
    default: {
      assert(0);
    }
  }
  return slice;
}

Base* convert_to_cpp(const koopa_raw_type_t& raw) {
  // std::cout<<"convert raw type to cpp form "<<raw->tag<<"\n";
  switch (raw->tag) {
    case KOOPA_RTT_INT32: {
      auto type =  new INT32Type();
      return type;
    }
    case KOOPA_RTT_UNIT: {
      auto type =  new UnitType();
      return type;
    }
    case KOOPA_RTT_ARRAY: {
      auto type =  new ArrayType();
      type->base = cast_uptr<Type>(convert_to_cpp(raw->data.array.base));
      type->len = raw->data.array.len;
      return type;
    }
    case KOOPA_RTT_POINTER: {
      auto type =  new PointerType();
      type->base = cast_uptr<Type>(convert_to_cpp(raw->data.array.base));
      return type;
    }
    case KOOPA_RTT_FUNCTION: {
      auto type =  new FunctionType();
      type->params = cast_uptr<Slice>(convert_to_cpp(raw->data.function.params));
      type->ret = cast_uptr<Type>(convert_to_cpp(raw->data.function.ret));
      return type;
    }
    default: {
      assert(0);
    }
  }
}

Base* convert_to_cpp(const koopa_raw_function_t& raw) {
  std::cout<<"convert raw function to cpp form\n";
  auto func = new Function();
  raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(func);
  func->type = cast_uptr<Type>(convert_to_cpp(raw->ty));
  func->name = std::string(raw->name);
  func->params = cast_uptr<Slice>(convert_to_cpp(raw->params));
  func->bbs = cast_uptr<Slice>(convert_to_cpp(raw->bbs));
  return func;
}

Base* convert_to_cpp(const koopa_raw_basic_block_t& raw) {
  std::cout<<"convert raw basic block to cpp form\n";
  auto bb = new BasicBlock();
  raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(bb);
  if (raw->name) {
    bb->name = std::string(raw->name);
  }
  bb->params = cast_uptr<Slice>(convert_to_cpp(raw->params));
  bb->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
  bb->insts = cast_uptr<Slice>(convert_to_cpp(raw->insts));
  return bb;
}

Base* convert_to_cpp(const koopa_raw_value_t& raw) {
  std::cout<<"convert raw value to cpp form "<<raw->kind.tag<<"\n";
  switch (raw->kind.tag) {
    case KOOPA_RVT_INTEGER :{
      auto value = new IntegerValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->value = raw->kind.data.integer.value;
      return value;
    }
    case KOOPA_RVT_ZERO_INIT:{

    }
    case KOOPA_RVT_AGGREGATE :{
      auto value = new AggregateValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->elems = cast_uptr<Slice>(convert_to_cpp(raw->kind.data.aggregate.elems));
      return value;
    }
    case KOOPA_RVT_FUNC_ARG_REF :{
      auto value = new FuncArgRefValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->index = raw->kind.data.func_arg_ref.index;
      return value;
    }
    case KOOPA_RVT_BLOCK_ARG_REF :{
      auto value = new BlockArgRef();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->index = raw->kind.data.block_arg_ref.index;
      return value;
    }
    case KOOPA_RVT_ALLOC: {
      auto value = new LocalAllocValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      return value;
    }
    case KOOPA_RVT_GLOBAL_ALLOC :{
      auto value = new GlobalAllocValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->init = cast_uptr<Value>(convert_to_cpp(raw->kind.data.global_alloc.init));
      return value;
    }
    case KOOPA_RVT_LOAD :{
      auto value = new LoadValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->src = cast_uptr<Value>(convert_to_cpp(raw->kind.data.load.src));
      return value;
    }
    case KOOPA_RVT_STORE :{
      auto value = new StoreValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->value = cast_uptr<Value>(convert_to_cpp(raw->kind.data.store.value));
      value->dest = cast_uptr<Value>(convert_to_cpp(raw->kind.data.store.dest));
      return value;
    }
    case KOOPA_RVT_GET_PTR :{
      auto value = new GetPtrValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->src = cast_uptr<Value>(convert_to_cpp(raw->kind.data.get_ptr.src));
      value->index = cast_uptr<Value>(convert_to_cpp(raw->kind.data.get_ptr.index));
      return value;
    }
    case KOOPA_RVT_GET_ELEM_PTR :{
      auto value = new GetElemPtrValue();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->src = cast_uptr<Value>(convert_to_cpp(raw->kind.data.get_elem_ptr.src));
      value->index = cast_uptr<Value>(convert_to_cpp(raw->kind.data.get_elem_ptr.index));
      return value;
    }
    case KOOPA_RVT_BINARY :{
      auto value = new BinaryOp();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->op = raw->kind.data.binary.op;
      value->lhs = cast_uptr<Value>(convert_to_cpp(raw->kind.data.binary.lhs));
      value->rhs = cast_uptr<Value>(convert_to_cpp(raw->kind.data.binary.rhs));
      return value;
    }
    case KOOPA_RVT_BRANCH :{
      auto value = new BranchOp();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->cond = cast_uptr<Value>(convert_to_cpp(raw->kind.data.branch.cond));
      value->true_bb = cast_uptr<BasicBlock>(convert_to_cpp(raw->kind.data.branch.true_bb));
      value->false_bb = cast_uptr<BasicBlock>(convert_to_cpp(raw->kind.data.branch.false_bb));
      value->true_args = cast_uptr<Slice>(convert_to_cpp(raw->kind.data.branch.true_args));
      value->false_args = cast_uptr<Slice>(convert_to_cpp(raw->kind.data.branch.false_args));
      return value;
    }
    case KOOPA_RVT_JUMP :{
      auto value = new JumpOp();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->target = cast_uptr<BasicBlock>(convert_to_cpp(raw->kind.data.jump.target));
      value->args = cast_uptr<Slice>(convert_to_cpp(raw->kind.data.jump.args));
      return value;
    }
    case KOOPA_RVT_CALL :{
      auto value = new CallOp();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      value->callee = cast_uptr<Function>(convert_to_cpp(raw->kind.data.call.callee));
      value->args = cast_uptr<Slice>(convert_to_cpp(raw->kind.data.call.args));
      return value;
    }
    case KOOPA_RVT_RETURN :{
      auto value = new ReturnOp();
      raw_addr_to_cpp_addr[reinterpret_cast<size_t>(raw)] = reinterpret_cast<size_t>(value);
      value->ty = cast_uptr<Type>(convert_to_cpp(raw->ty));
      if(raw->name){
        value->name = std::string(raw->name);
      }
      value->used_by = cast_wptr<Slice>(convert_to_cpp(raw->used_by));
      if(raw->kind.data.ret.value){
        value->value = cast_uptr<Value>(convert_to_cpp(raw->kind.data.ret.value));
      }
      return value;
    }
    default: {
      std::cout<<raw->kind.tag<<std::endl;
      assert(0);
    }
  }
}


};  // namespace KOOPA