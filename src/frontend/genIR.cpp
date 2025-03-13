#include "genIR.hpp"
#include "eval.hpp"
#include <cassert>
#include <iostream>

using namespace AST;

void GenIRVisitor::visit(CompUnit& node) {
  std::cout<<"genir visit compunit"<<std::endl;
  ir_code = std::make_unique<std::string>("");
  auto item_ptr = node.item.get();        // here item_ptr's type is either FuncDef or Decl
  while (item_ptr) {
    item_ptr->accept(*this);
    item_ptr = item_ptr->next_compunit_item.get();
  }
}

void GenIRVisitor::visit(FuncDef& node) {
  std::cout<<"genir visit funcdef"<<std::endl;
  ir_code->append("fun @" + node.ident + "(): ");
  node.func_type->accept(*this);           // should print i32
  ir_code->append(" {\n%entry:\n");
  // here block_item is a list of blockitem
  auto block_item_ptr = node.block_item.get();
  while(block_item_ptr) {
    std::cout<<"call block_item accept"<<std::endl;
    block_item_ptr->accept(*this);
    block_item_ptr = block_item_ptr->next_block_item.get();
  }
  ir_code->append("}\n");
}

void GenIRVisitor::visit(FuncType& node) {
  assert(node.type_name == "int");
  ir_code->append("i32");
}

void GenIRVisitor::visit(NumberExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(NegativeExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(LogicalNotExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(AddExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(SubExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(MulExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(DivExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(ModExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(LTExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(GTExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(LEExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(GEExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(EQExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(NEExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(LAndExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(LOrExp& node) {
  auto evaluate_visitor = EvaluateVisitor();
  node.accept(evaluate_visitor);
  ir_code->append(std::to_string(evaluate_visitor.result)); 
}

void GenIRVisitor::visit(ConstDecl& node) {
  // std::cout<<"currently just pass genir visit constdecl"<<std::endl;
  std::cout<<"genir visit constdecl"<<std::endl;
  auto const_def_ptr = node.const_def.get();
  while (const_def_ptr) {
    const_def_ptr->accept(*this);
    const_def_ptr = const_def_ptr->next_const_def.get();
  }
}

void GenIRVisitor::visit(ConstDef& node) {
  if(const_sym_table.find(node.ident) != const_sym_table.end()) {
    throw std::runtime_error("redefined symbol: " + node.ident);
  }
  auto evaluate_visitor = EvaluateVisitor(const_sym_table);
  node.const_init_val->accept(evaluate_visitor);
  const_sym_table[node.ident] = evaluate_visitor.result;
  std::cout<<"store "<<node.ident<<" "<<evaluate_visitor.result<<" into symbol table"<<std::endl;
}

void GenIRVisitor::visit(RetStmt& node) {
  std::cout<<"genir visit retstmt"<<std::endl;
  ir_code->append("  ret ");
  node.exp->accept(*this);
  ir_code->append("\n");
  // node.next_block_item->accept(*this);    // TODO: should there be other items after retstmt??
}

void GenIRVisitor::visit(AssignStmt& node) {
  std::cout<<"currently just pass genir visit assignstmt"<<std::endl;
  // auto evaluate_visitor = EvaluateVisitor();
  // node.exp->accept(evaluate_visitor);
  // const_sym_table[node.lval->ident] = evaluate_visitor.result;
  // std::cout<<"store "<<node.lval->ident<<" "<<evaluate_visitor.result<<" into symbol table"<<std::endl;
}

void GenIRVisitor::visit(LValExp& node) {
  std::cout<<"genir visit lval"<<std::endl;
  if (const_sym_table.find(node.ident) == const_sym_table.end()) {
    throw std::runtime_error("undefined symbol: " + node.ident);
  }
  ir_code->append(std::to_string(const_sym_table[node.ident]));
}


