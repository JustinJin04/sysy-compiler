#pragma once
#include <koopa.h>
#include <cassert>
#include "genASM.hpp"
#include <fstream>

void IR_to_ASM(std::unique_ptr<std::string>& ir, const std::string& file_name) {
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

  // generate asm
  KOOPA::GenASMVisitor gen_asm_visitor(file_name);
  gen_asm_visitor.visit(raw);

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
}