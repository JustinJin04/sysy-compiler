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
    item_ptr = item_ptr->next.get();
  }
}

void GenIRVisitor::visit(FuncDef& node) {
  std::cout<<"genir visit funcdef"<<std::endl;
  ir_code->append("fun @" + node.ident + "(): ");
  node.func_type->accept(*this);           // should print i32
  ir_code->append(" {\n%entry:\n");
  node.block_item->accept(*this);          // here block_item is either Decl or Stmt
  ir_code->append("}\n");
}

void GenIRVisitor::visit(FuncType& node) {
  assert(node.type_name == "int");
  ir_code->append("i32");
}

void GenIRVisitor::visit(RetStmt& node) {
  std::cout<<"genir visit retstmt"<<std::endl;
  ir_code->append("  ret ");
  node.exp->accept(*this);
  ir_code->append("\n");
  // node.next_block_item->accept(*this);    // TODO: should there be other items after retstmt??
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



