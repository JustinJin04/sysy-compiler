#include "ir2asm.hpp"
#include <cassert>
#include <iostream>
#include <map>

static BasicBlockContext GLOBAL_BASIC_BLOCK_CTX;

static std::string indent(int space=2) {
  return std::string(space, ' ');
}

void visit(const koopa_raw_program_t& raw, std::unique_ptr<std::string>& asm_code) {
  asm_code->append(indent()+".text\n");
  
  // 遍历global value
  assert(raw.values.kind == KOOPA_RSIK_VALUE);
  for(int i=0;i<raw.values.len;++i){
    auto ptr = raw.values.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr), asm_code);
  }

  // 遍历global func
  assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
  for(int i=0;i<raw.funcs.len;++i){
    auto ptr = raw.funcs.buffer[i];
    visit(reinterpret_cast<koopa_raw_function_t>(ptr), asm_code);
  }
}

void visit(const koopa_raw_value_t& value, std::unique_ptr<std::string>& asm_code) {
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      GLOBAL_BASIC_BLOCK_CTX.push_current_koopa_raw_value(value);
      visit(kind.data.ret, asm_code);
      break;
    // case KOOPA_RVT_INTEGER:
    //   visit(kind.data.integer, asm_code);
    //   break;
    case KOOPA_RVT_BINARY:
      // std::cout<<"visit binary "<<value<<" ";
      GLOBAL_BASIC_BLOCK_CTX.push_current_koopa_raw_value(value);
      visit(kind.data.binary, asm_code);
      break;
    default:
      assert(false);
  }
}

void visit(const koopa_raw_function_t& func, std::unique_ptr<std::string>& asm_code) {
  // 去掉IR中的@符号
  auto func_name = std::string(func->name).substr(1);
  asm_code->append(indent()+".global "+func_name+"\n"+func_name+":\n");
  
  // visit basic blocks of this function
  assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
  for(int i=0;i<func->bbs.len;++i){
    auto ptr = func->bbs.buffer[i];
    visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), asm_code);
  }
}

void visit(const koopa_raw_basic_block_t& block, std::unique_ptr<std::string>& asm_code) {
  if(block->name) {
    // asm_code->append(std::string(block->name)+":\n");
    // TODO: 如何处理带名字的block??
  }
  GLOBAL_BASIC_BLOCK_CTX.reset();
  // visit all the instructions
  assert(block->insts.kind == KOOPA_RSIK_VALUE);
  for(int i=0;i<block->insts.len;++i){
    auto ptr = block->insts.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr), asm_code);
  }
}

void BasicBlockContext::prepare_operand(const koopa_raw_value_t& operand, std::unique_ptr<std::string>& asm_code) {
  if(value_to_name(operand)){
    return;
  }
  if (operand->kind.tag == koopa_raw_value_tag_t::KOOPA_RVT_INTEGER) {
    auto int_const = operand->kind.data.integer.value;
    // std::cout<<"debug "<<int_const<<"\n";
    if (int_const == 0){
      bind_value_temp_reg("x0", operand);
    } else {
      auto tmp_reg_name = alloc_new_temp_reg();
      bind_value_temp_reg(tmp_reg_name, operand);
      asm_code->append(indent() + "li " + tmp_reg_name + ", " + std::to_string(int_const) + "\n");
    }
  } else {
    assert(0);
  }
}

void BasicBlockContext::prepare_operand(const koopa_raw_value_t& operand, std::unique_ptr<std::string>& asm_code, std::string reg_name) {
  auto orig_name = value_to_name(operand);
  if(orig_name.has_value()){
    if(*orig_name != reg_name) {
      asm_code->append(indent() + "mv " + reg_name + ", " + *orig_name + "\n");
      bind_value_temp_reg(reg_name, operand);
    }
    return;
  }
  if (operand->kind.tag == koopa_raw_value_tag_t::KOOPA_RVT_INTEGER) {
    auto int_const = operand->kind.data.integer.value;
    if (int_const == 0){
      if (reg_name != "x0") {
        asm_code->append(indent() + "mv " + reg_name + ", x0\n");
        bind_value_temp_reg(reg_name, operand);
      }
    } else {
      asm_code->append(indent() + "li " + reg_name + ", " + std::to_string(int_const) + "\n");
      bind_value_temp_reg(reg_name, operand);
    }
  } else {
    assert(0);
  }
}

void visit(const koopa_raw_return_t& ret, std::unique_ptr<std::string>& asm_code) {
  if (ret.value) {
    GLOBAL_BASIC_BLOCK_CTX.prepare_operand(ret.value, asm_code, "a0");
  }
  asm_code->append(indent()+"ret\n");
}

void visit(const koopa_raw_binary_t& binary, std::unique_ptr<std::string>& asm_code) {
  auto op = binary.op;
  auto lhs = binary.lhs;
  auto rhs = binary.rhs;

  GLOBAL_BASIC_BLOCK_CTX.prepare_operand(lhs, asm_code);
  GLOBAL_BASIC_BLOCK_CTX.prepare_operand(rhs, asm_code);

  auto lreg_name = GLOBAL_BASIC_BLOCK_CTX.value_to_name(lhs);
  auto rreg_name = GLOBAL_BASIC_BLOCK_CTX.value_to_name(rhs);
  assert(lreg_name.has_value() && rreg_name.has_value());

  switch (op) {
    case KOOPA_RBO_SUB:{
      // TODO: default using left operand reg to store result?????
      if (*lreg_name == "x0") {
        asm_code->append(indent() + "sub " + *rreg_name + ", x0, " + *rreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*rreg_name);
      } else {
        asm_code->append(indent() + "sub " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      }
      break;
    }
    case KOOPA_RBO_MUL: {
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*rreg_name);
      asm_code->append(indent() + "mul " + *rreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      break;
    }
    case KOOPA_RBO_ADD: {
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      asm_code->append(indent() + "add " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      break;
    }
    case KOOPA_RBO_DIV: {
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      asm_code->append(indent() + "div " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      break;
    }
    case KOOPA_RBO_MOD: {
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      asm_code->append(indent() + "rem " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      break;
    }
    case KOOPA_RBO_LT: {
      asm_code->append(indent() + "slt " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      break;
    }
    case KOOPA_RBO_GT: {
      asm_code->append(indent() + "sgt " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      break;
    }
    case KOOPA_RBO_LE: {
      // asm_code->append(indent() + "slt " + *lreg_name + ", " + *rreg_name + ", " + *lreg_name + "\n");
      asm_code->append(indent() + "sgt " + *rreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      asm_code->append(indent() + "seqz " + *rreg_name + ", " + *rreg_name + "\n");
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*rreg_name);
      break;
    }
    case KOOPA_RBO_GE: {
      // asm_code->append(indent() + "sge " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      asm_code->append(indent() + "slt " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      asm_code->append(indent() + "seqz " + *lreg_name + ", " + *lreg_name + "\n");
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      break;
    }
    case KOOPA_RBO_EQ: {
      // TODO: is reusing lreg rational?
      // 如果两个都是x0则分配新的reg
      // 否则使用其中一个reg保存结果
      if(*lreg_name == "x0" && *rreg_name == "x0"){
        auto new_reg_name = GLOBAL_BASIC_BLOCK_CTX.alloc_new_temp_reg();
        asm_code->append(indent() + "li " + new_reg_name + ", 1\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(new_reg_name);
      } else if (*lreg_name == "x0" && *rreg_name != "x0") {
        asm_code->append(indent() + "seqz " + *rreg_name + ", " + *rreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*rreg_name);
      } else if (*lreg_name != "x0" && *rreg_name == "x0") {
        asm_code->append(indent() + "seqz " + *lreg_name + ", " + *lreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      } else {
        asm_code->append(indent() + "xor " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
        asm_code->append(indent() + "seqz " + *lreg_name + ", " + *lreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      }
      break;
    }
    case KOOPA_RBO_NOT_EQ: {
      if(*lreg_name == "x0" && *rreg_name == "x0"){
        auto new_reg_name = GLOBAL_BASIC_BLOCK_CTX.alloc_new_temp_reg();
        asm_code->append(indent() + "li " + new_reg_name + ", 0\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(new_reg_name);
      } else if (*lreg_name == "x0" && *rreg_name != "x0") {
        asm_code->append(indent() + "snez " + *rreg_name + ", " + *rreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*rreg_name);
      } else if (*lreg_name != "x0" && *rreg_name == "x0") {
        asm_code->append(indent() + "snez " + *lreg_name + ", " + *lreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      } else {
        asm_code->append(indent() + "xor " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
        asm_code->append(indent() + "snez " + *lreg_name + ", " + *lreg_name + "\n");
        GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      }
      break;
    }
    case KOOPA_RBO_AND: {
      asm_code->append(indent() + "and " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      break;
    }
    case KOOPA_RBO_OR: {
      asm_code->append(indent() + "or " + *lreg_name + ", " + *lreg_name + ", " + *rreg_name + "\n");
      // asm_code->append(indent() + "snez " + *lreg_name + ", " + *lreg_name + "\n");
      GLOBAL_BASIC_BLOCK_CTX.bind_value_temp_reg(*lreg_name);
      break;
    }
    
    default: {
      assert(0);
    }
  }
}



void IR_to_ASM(std::unique_ptr<std::string>& ir, std::unique_ptr<std::string>& asm_code){
  // ir to raw program
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(ir->c_str(), &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);

  // 处理 raw program，生成asm_code
  visit(raw, asm_code);

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
}