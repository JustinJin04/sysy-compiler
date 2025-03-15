#pragma once

#include "ir.hpp"
#include <cassert>
#include "genASM.hpp"

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

  // conver to cpp
  std::cout<<"start to convert raw to cpp form\n";
  KOOPA::Program* program_cpp_ptr = dynamic_cast<KOOPA::Program*>(KOOPA::convert_to_cpp(raw));
  // auto program_cpp = std::unique_ptr<KOOPA::Program>(std::move(program_cpp_ptr));
  koopa_delete_raw_program_builder(builder);

  std::cout<<"start to generate asm\n";
  // generate asm
  auto gen_asm_visitor = std::unique_ptr<KOOPA::GenASMVisitor>(new KOOPA::GenASMVisitor());
  program_cpp_ptr->accept(*gen_asm_visitor);
  std::cout<<"end gen asm\n";
  asm_code = std::make_unique<std::string>(*(gen_asm_visitor->asm_code));
  std::cout<<*asm_code<<std::endl;

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  // koopa_delete_raw_program_builder(builder);
}