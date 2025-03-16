#include "genIR.hpp"
#include "eval.hpp"
#include <cassert>
#include <iostream>

using namespace AST;

void GenIRVisitor::visit(CompUnit& node) {
  std::cout<<"genir visit compunit"<<std::endl;
  // initialize sym_table_stack for global scop
  sym_table_stack.push_table();
  ir_code = std::make_unique<std::string>("");
  auto item_ptr = node.item.get();        // here item_ptr's type is either FuncDef or Decl
  while (item_ptr) {
    item_ptr->accept(*this);
    item_ptr = item_ptr->next_compunit_item.get();
  }
}

inline std::string get_last_ir_line(std::unique_ptr<std::string>& ir_code) {
  int last_line_pos = ir_code->find_last_of('\n', ir_code->size()-2);
  return ir_code->substr(last_line_pos+1, ir_code->size()-last_line_pos-2);
}

bool is_last_line_label(std::unique_ptr<std::string>& ir_code) {
  auto line = get_last_ir_line(ir_code);
  return line.find(":") != std::string::npos;
}

void GenIRVisitor::visit(FuncDef& node) {
  std::cout<<"genir visit funcdef"<<std::endl;
  // initialize sym_table_stack for function scope
  sym_table_stack.push_table();
  ir_code->append("fun @" + node.ident + "(): ");
  node.func_type->accept(*this);           // should print i32
  // ir_code->append(" {\n%entry:\n");
  ir_code->append("  {\n%block_" + std::to_string(block_label_counter) + ":\n");
  block_label_counter++;

  // pruning the basic block to make sure
  // there is no stmt after ret stmt
  auto pruning_ret_visitor = PruningRetVisitor();
  assert(node.block_item);
  node.block_item->accept(pruning_ret_visitor);

  // here block_item is a list of blockitem
  auto block_item_ptr = node.block_item.get();
  while(block_item_ptr) {
    block_item_ptr->accept(*this);
    block_item_ptr = block_item_ptr->next_block_item.get();
  }
  if(is_last_line_label(ir_code)){
    ir_code->append("  ret 0\n");
  }
  ir_code->append("}\n");
  sym_table_stack.pop_table();
}

void GenIRVisitor::visit(FuncType& node) {
  assert(node.type_name == "int");
  ir_code->append("i32");
}

void GenIRVisitor::visit(NumberExp& node) {
  push_result(std::to_string(node.number));
}

void GenIRVisitor::visit(NegativeExp& node) {
  node.operand->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = sub 0, " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LogicalNotExp& node) {
  node.operand->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = eq " + rhs_name + ", 0\n");
  push_result(result_name);
}

void GenIRVisitor::visit(AddExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = add " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(SubExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = sub " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(MulExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = mul " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(DivExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = div " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(ModExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = mod " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LTExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = lt " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(GTExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = gt " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LEExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = le " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(GEExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = ge " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(EQExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = eq " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(NEExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = ne " + lhs_name + ", " + rhs_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LAndExp& node) {
  // node.lhs->accept(*this);
  // auto lhs_name = pop_last_result();
  // node.rhs->accept(*this);
  // auto rhs_name = pop_last_result();
  // auto result_name = get_new_counter();
  // auto temp_l = get_new_counter();
  // auto temp_r = get_new_counter();
  // ir_code->append("  " + temp_l + " = ne " + lhs_name + ", 0\n");
  // ir_code->append("  " + temp_r + " = ne " + rhs_name + ", 0\n");
  // ir_code->append("  " + result_name + " = and " + temp_l + ", " + temp_r + "\n");
  // push_result(result_name);

  /**
   * short-circuit evaluation
   */
  int count = sym_table_stack.total_accurrences("result", false);
  auto result_ident_name = "@result_" + std::to_string(count+1);
  ir_code->append("  " + result_ident_name + " = alloc i32\n");
  ir_code->append("  store 0, " + result_ident_name + "\n");
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  auto tmp_1 = get_new_counter();
  ir_code->append("  " + tmp_1 + " = ne " + lhs_name + ", 0\n");
  auto then_label = "%then_" + std::to_string(block_label_counter);
  auto end_label = "%end_" + std::to_string(block_label_counter);
  block_label_counter++;
  ir_code->append("  br " + tmp_1 + ", " + then_label + ", " + end_label + "\n");
  ir_code->append(then_label + ":\n");
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto tmp_2 = get_new_counter();
  ir_code->append("  " + tmp_2 + " = ne " + rhs_name + ", 0\n");
  ir_code->append("  store " + tmp_2 + ", " + result_ident_name + "\n");
  ir_code->append("  jump " + end_label + "\n");
  ir_code->append(end_label + ":\n");
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = load " + result_ident_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LOrExp& node) {
  // node.lhs->accept(*this);
  // auto lhs_name = pop_last_result();
  // node.rhs->accept(*this);
  // auto rhs_name = pop_last_result();
  // auto temp_name = get_new_counter();
  // auto result_name = get_new_counter();
  // ir_code->append("  " + temp_name + " = or " + lhs_name + ", " + rhs_name + "\n");
  // ir_code->append("  " + result_name + " = ne " + temp_name + ", 0\n");
  // push_result(result_name);

  /**
   * short-circuit evaluation
   */
  int count = sym_table_stack.total_accurrences("result", false);
  auto result_ident_name = "@result_" + std::to_string(count+1);
  ir_code->append("  " + result_ident_name + " = alloc i32\n");
  ir_code->append("  store 1, " + result_ident_name + "\n");
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  auto tmp_1 = get_new_counter();
  ir_code->append("  " + tmp_1 + " = eq " + lhs_name + ", 0\n");
  auto then_label = "%then_" + std::to_string(block_label_counter);
  auto end_label = "%end_" + std::to_string(block_label_counter);
  block_label_counter++;
  ir_code->append("  br " + tmp_1 + ", " + then_label + ", " + end_label + "\n");
  ir_code->append(then_label + ":\n");
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto tmp_2 = get_new_counter();
  ir_code->append("  " + tmp_2 + " = ne " + rhs_name + ", 0\n");
  ir_code->append("  store " + tmp_2 + ", " + result_ident_name + "\n");
  ir_code->append("  jump " + end_label + "\n");
  ir_code->append(end_label + ":\n");
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = load " + result_ident_name + "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(ConstDecl& node) {
  std::cout<<"genir visit constdecl"<<std::endl;
  auto const_def_ptr = node.const_def.get();
  while (const_def_ptr) {
    const_def_ptr->accept(*this);
    const_def_ptr = const_def_ptr->next_const_def.get();
  }
}

void GenIRVisitor::visit(ConstDef& node) {
  if(sym_table_stack.find_in_current(node.ident, true)) {
    throw std::runtime_error("redefined const symbol: " + node.ident + " in current scope");
  }
  auto evaluate_visitor = EvaluateVisitor(&sym_table_stack);
  node.const_init_val->accept(evaluate_visitor);
  sym_table_stack.insert_to_top(node.ident, evaluate_visitor.result);
  std::cout<<"store const "<<node.ident<<" "<<evaluate_visitor.result<<" into symbol table"<<std::endl;
}

void GenIRVisitor::visit(RetStmt& node) {
  std::cout<<"genir visit retstmt"<<std::endl;
  if(node.exp){
    node.exp->accept(*this);
    auto result_name = pop_last_result();
    ir_code->append("  ret " + result_name + "\n");
  } else {
    ir_code->append("  ret\n");
  }
  // generate ret label in order to end this basic block
  // ir_code->append("%ret_label_" + std::to_string(ret_label_counter) + ":\n");
  // ret_label_counter++;
  // node.next_block_item->accept(*this);    // TODO: should there be other items after retstmt??
}

void GenIRVisitor::visit(AssignStmt& node) {
  std::cout<<"genir visit assignstmt"<<std::endl;
  // since const value must be declared
  // hence here lval must be var_symbol
  // auto find_it = sym_table.find(node.lval->ident);
  // auto find_result = sym_table_stack.find(node.lval->ident);
  // if(find_result == SymbolType::NONE) {
  //   throw std::runtime_error("undefined symbol: " + node.lval->ident);
  // } else if (find_result == SymbolType::CONST) { // 1 means const symbol
  //   throw std::runtime_error("const value cannot be assigned");
  // }
  // auto var_name = std::get<std::string>(find_it->second);
  auto find_var = sym_table_stack.find(node.lval->ident, false);
  if(find_var == false) {
    throw std::runtime_error("undefined var symbol: " + node.lval->ident);
  }
  auto var_name = std::get<std::string>(sym_table_stack.get(node.lval->ident, false));

  // start to compose ir
  // First we have to evaluate exp and push the result
  // Then we load lval and store the result
  node.exp->accept(*this);
  auto store_counter_name = pop_last_result();
  ir_code->append("  store " + store_counter_name + ", " + var_name + "\n");
  
}

void GenIRVisitor::visit(ExpStmt& node) {
  std::cout<<"ignore expstmt"<<std::endl;
  // if(node.exp) {
  //   node.exp->accept(*this);
  //   pop_last_result();
  // }
}

void GenIRVisitor::visit(BlockStmt& node) {
  std::cout<<"genir visit blockstmt"<<std::endl;

  sym_table_stack.push_table();
  auto block_item_ptr = node.block_item.get();
  while(block_item_ptr) {
    block_item_ptr->accept(*this);
    block_item_ptr = block_item_ptr->next_block_item.get();
  }
  sym_table_stack.pop_table();
}

void GenIRVisitor::visit(IfStmt& node) {
  std::cout<<"genir visit ifstmt"<<std::endl;
  node.cond->accept(*this);
  auto cond_name = pop_last_result();
  if(node.else_body) {
    auto then_label = "%then_" + std::to_string(block_label_counter);
    auto else_label = "%else_" + std::to_string(block_label_counter);
    auto end_label = "%end_" + std::to_string(block_label_counter);
    block_label_counter++;
    ir_code->append("  br " + cond_name + ", " + then_label + ", " + else_label + "\n");

    // handle then_body
    ir_code->append(then_label + ":\n");
    node.then_body->accept(*this);
    // TODO: this way is pretty hacky
    bool then_body_has_ret = get_last_ir_line(ir_code).substr(2, 3) == "ret";
    bool then_body_has_jump = get_last_ir_line(ir_code).substr(2, 4) == "jump";
    if(!then_body_has_ret && !then_body_has_jump) {
      ir_code->append("  jump " + end_label + "\n");
    }
    
    // handle else_body
    ir_code->append(else_label + ":\n");
    node.else_body->accept(*this);
    bool else_body_has_ret = get_last_ir_line(ir_code).substr(2, 3) == "ret";
    bool else_body_has_jump = get_last_ir_line(ir_code).substr(2, 4) == "jump";
    if(!else_body_has_ret && !else_body_has_jump) {
      ir_code->append("  jump " + end_label + "\n");
    }

    // add end_label
    // TODO: it's possible that after end_label, there is no ret stmt
    // so we need to eliminate the end_label when leave the functioin
    ir_code->append(end_label + ":\n");



    // if(then_body_has_ret && else_body_has_ret) {
    //   // don't add ret label
    // } else {
    //   ir_code->append(end_label + ":\n");
    // }



  } else {
    auto then_label = "%then_" + std::to_string(block_label_counter);
    auto end_label = "%end_" + std::to_string(block_label_counter);
    block_label_counter++;
    ir_code->append("  br " + cond_name + ", " + then_label + ", " + end_label + "\n");

    // handle then_body
    ir_code->append(then_label + ":\n");
    node.then_body->accept(*this);
    if(get_last_ir_line(ir_code).substr(2, 3) != "ret" && get_last_ir_line(ir_code).substr(2, 4) != "jump") {
      ir_code->append("  jump " + end_label + "\n");
    }

    // add end_label
    ir_code->append(end_label + ":\n");
  }

}

void GenIRVisitor::visit(WhileStmt& node) {
  std::cout<<"genir visit whilestmt"<<std::endl;
  auto while_entry_name = "%while_entry_" + std::to_string(block_label_counter);
  auto while_body_name = "%while_body_" + std::to_string(block_label_counter);
  auto while_end_name = "%while_end_" + std::to_string(block_label_counter);
  while_stack.push_while_label(while_entry_name, while_end_name);
  block_label_counter++;
  ir_code->append("  jump " + while_entry_name + "\n");
  ir_code->append(while_entry_name + ":\n");
  node.cond->accept(*this);
  auto cond_name = pop_last_result();
  ir_code->append("  br " + cond_name + ", " + while_body_name + ", " + while_end_name + "\n");
  ir_code->append(while_body_name + ":\n");
  node.body->accept(*this);
  // TODO: this way is pretty hacky
  if(get_last_ir_line(ir_code).substr(2, 3) != "ret" && get_last_ir_line(ir_code).substr(2, 4) != "jump") {
    ir_code->append("  jump " + while_entry_name + "\n");
  }
  ir_code->append(while_end_name + ":\n");
  while_stack.pop_while_label();
}

void GenIRVisitor::visit(BreakStmt& node) {
  std::cout<<"genir visit breakstmt"<<std::endl;
  auto while_end_name = while_stack.get_top_while_end();
  ir_code->append("  jump " + while_end_name + "\n");
}

void GenIRVisitor::visit(ContinueStmt& node) {
  std::cout<<"genir visit continuestmt"<<std::endl;
  auto while_entry_name = while_stack.get_top_while_entry();
  ir_code->append("  jump " + while_entry_name + "\n");
}


void GenIRVisitor::visit(LValExp& node) {
  std::cout<<"genir visit lval"<<std::endl;
  // auto find_it = sym_table.find(node.ident);
  bool find_const = sym_table_stack.find(node.ident, true);
  bool find_var = sym_table_stack.find(node.ident, false);
  if(find_const == false && find_var == false) {
    throw std::runtime_error("undefined symbol: " + node.ident);
  }
  // TODO: What if a symbol is found in both const and var table???????
  // assert(find_const != find_var);
  if(find_const && find_var) {
    int const_layer_num = sym_table_stack.find_layer_num(node.ident, true);
    int var_layer_num = sym_table_stack.find_layer_num(node.ident, false);
    if(const_layer_num < var_layer_num) {
      find_const = false;
    } else {
      find_var = false;
    }
  }

  if(find_const) { // const symbol, could return its value immediately
    // int value = std::get<int>(find_it->second);
    int value = std::get<int>(sym_table_stack.get(node.ident, true));
    push_result(std::to_string(value));

  } else { // var symbol, should load first and then return load_temp_counter
    // auto var_name = std::get<std::string>(find_it->second);
    auto var_name = std::get<std::string>(sym_table_stack.get(node.ident, false));
    auto load_counter_name = get_new_counter();
    ir_code->append("  " + load_counter_name + " = load " + var_name + "\n");
    push_result(load_counter_name);
  }
}

void GenIRVisitor::visit(VarDecl& node) {
  std::cout<<"genir visit vardecl"<<std::endl;
  auto var_def_ptr = node.var_def.get();
  while(var_def_ptr) {
    var_def_ptr->accept(*this);
    var_def_ptr = var_def_ptr->next_var_def.get();
  }
}

void GenIRVisitor::visit(VarDef& node) {
  std::cout<<"genir visit vardef"<<std::endl;
  if(sym_table_stack.find_in_current(node.ident, false)) {
    throw std::runtime_error("redefined var symbol: " + node.ident);
  }
  int count = sym_table_stack.total_accurrences(node.ident, false);
  auto var_symbol_name = "@" + node.ident + "_" + std::to_string(count+1);
  // sym_table[node.ident] = var_symbol_name;
  sym_table_stack.insert_to_top(node.ident, var_symbol_name);
  std::cout<<"store var "<<var_symbol_name<<" into symbol table"<<std::endl;

  // start to compose ir code
  ir_code->append("  " + var_symbol_name + " = alloc i32\n");
  if(node.var_init_val) {
    node.var_init_val->accept(*this);
    auto result = pop_last_result();
    ir_code->append("  store " + result + ", " + var_symbol_name + "\n");
  }

}
