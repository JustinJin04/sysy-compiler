#include <ast.hpp>
#include <cassert>

static std::string indent(int depth) { return std::string(2 * depth, ' '); }

void CompUnitAST::Dump(int depth) const {
  std::cout << indent(depth) << "CompUnitAST {\n";
  func_def->Dump(depth + 1);
  std::cout << indent(depth) << "}\n";
}

void CompUnitAST::GenerateIR(std::unique_ptr<std::string>& ir, int depth) const {
  func_def->GenerateIR(ir, depth);
}



void FuncDefAST::Dump(int depth) const {
  std::cout << indent(depth) << "FuncDefAST {\n";
  func_type->Dump(depth + 1);
  std::cout << indent(depth + 1) << ident << ",\n";
  block->Dump(depth + 1);
  std::cout << indent(depth) << "}\n";
}

void FuncDefAST::GenerateIR(std::unique_ptr<std::string>& ir, int depth) const {
  // std::cout<<indent(depth)<<"fun @"<<ident<<"(): ";
  ir->append(indent(depth)+"fun @"+ident+"(): ");
  func_type->GenerateIR(ir, 0);
  // std::cout<<" {\n";
  // std::cout<<"%entry:\n";
  ir->append(" {\n%entry:\n");
  block->GenerateIR(ir, depth+1);
  // std::cout<<indent(depth)<<"}\n";
  ir->append(indent(depth)+"}\n");
}

void FuncTypeAST::Dump(int depth) const {
  std::cout << indent(depth) << "FuncTypeAST {\n";
  std::cout << indent(depth + 1) << value << "\n";
  std::cout << indent(depth) << "},\n";
}

void FuncTypeAST::GenerateIR(std::unique_ptr<std::string>& ir, int depth) const {
  assert(depth==0);
  assert(value == "int");
  // std::cout<<"i32";
  ir->append("i32");
}

void BlockAST::Dump(int depth) const {
  std::cout << indent(depth) << "BlockAST {\n";
  stmt->Dump(depth + 1);
  std::cout << indent(depth) << "}\n";
}

void BlockAST::GenerateIR(std::unique_ptr<std::string>& ir, int depth) const {
  stmt->GenerateIR(ir, depth);
}

void StmtAST::Dump(int depth) const {
  std::cout << indent(depth) << "StmtAST {\n";
  std::cout << indent(depth + 1) << number << "\n";
  std::cout << indent(depth) << "}\n";
}

void StmtAST::GenerateIR(std::unique_ptr<std::string>& ir, int depth) const {
  // std::cout<<indent(depth) <<"ret "<<number<<"\n";
  ir->append(indent(depth)+"ret "+std::to_string(number)+"\n");
}