#include "ir2asm.hpp"
#include <cassert>

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
      visit(kind.data.ret, asm_code);
      break;
    case KOOPA_RVT_INTEGER:
      visit(kind.data.integer, asm_code);
      break;
    case KOOPA_RVT_BINARY:
      visit(kind.data.binary, asm_code);
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

  // visit all the instructions
  assert(block->insts.kind == KOOPA_RSIK_VALUE);
  for(int i=0;i<block->insts.len;++i){
    auto ptr = block->insts.buffer[i];
    visit(reinterpret_cast<koopa_raw_value_t>(ptr), asm_code);
  }
}

void visit(const koopa_raw_return_t& ret, std::unique_ptr<std::string>& asm_code) {
  if(ret.value) {
    assert(ret.value->kind.tag == KOOPA_RVT_INTEGER);
    asm_code->append(indent()+"li a0, ");
    visit(ret.value->kind.data.integer, asm_code);
    asm_code->append("\n");
  }
  asm_code->append(indent()+"ret\n");
}

void visit(const koopa_raw_integer_t& integer, std::unique_ptr<std::string>& asm_code) {
  auto int_value = integer.value;
  asm_code->append(std::to_string(int_value));
}

void visit(const koopa_raw_binary_t& binary, std::unique_ptr<std::string>& asm_code) {
  // TODO
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