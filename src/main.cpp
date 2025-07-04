#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "frontend/ast.hpp"
#include "frontend/genIR.hpp"
#include "backend/ir2asm.hpp"
#include "checkIR.hpp"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE* yyin;
extern int yyparse(unique_ptr<AST::CompUnit>& ast);

int main(int argc, const char* argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = std::string(argv[1]);
  auto input = std::string(argv[2]);
  auto output = std::string(argv[4]);

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input.c_str(), "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<AST::CompUnit> ast;
  std::cout << "start parsing" << std::endl;
  auto ret = yyparse(ast);
  std::cout << "end parsing" << std::endl;
  if (ret) {
    exit(11);
  }
  // assert(!ret);

  // 输出解析得到的 AST, 其实就是个字符串
  auto ir_visitor = AST::GenIRVisitor();
  ast->accept(ir_visitor);
  std::cout << "end genIR" << std::endl;
  auto& ir = ir_visitor.ir_code;
  // std::cout<<*ir<<std::endl;
  // check ir
  verify_koopa_blocks(*ir);
  std::cout << "check done" << std::endl;
  FILE* output_file;
  if (mode == "-koopa") {
    output_file = fopen(output.c_str(), "w");
    if (output_file) {
      fwrite(ir->c_str(), sizeof(char), ir->length(), output_file);
      fclose(output_file);
    }
  } else if (mode == "-riscv") {
    IR_to_ASM(ir, output);
  } else if (mode == "-perf") {
    IR_to_ASM(ir, output);
  }

  return 0;
}
