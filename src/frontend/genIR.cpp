#include "genIR.hpp"

#include <cassert>
#include <iostream>

#include "eval.hpp"

using namespace AST;

void GenIRVisitor::visit(CompUnit& node) {
  std::cout << "genir visit compunit" << std::endl;
  // initialize sym_table_stack for global scop
  sym_table_stack.push_table();
  ir_code = std::make_unique<std::string>("");

  // add sysy runtime library function declaration
  ir_code->append("decl @getint(): i32\n");
  sym_table_stack.insert_to_top("getint", "@getint(): i32",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @getch(): i32\n");
  sym_table_stack.insert_to_top("getch", "@getch(): i32",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @getarray(*i32): i32\n");
  sym_table_stack.insert_to_top("getarray", "@getarray(*i32): i32",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @putint(i32): i32\n");
  sym_table_stack.insert_to_top("putint", "@putint(i32): i32",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @putch(i32): i32\n");
  sym_table_stack.insert_to_top("putch", "@putch(i32): i32",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @putarray(i32, *i32): i32\n");
  sym_table_stack.insert_to_top("putarray", "@putarray(i32, *i32): i32",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @starttime()\n");
  sym_table_stack.insert_to_top("starttime", "@starttime()",
                                SymbolTables::SymbolKind::FUNC);
  ir_code->append("decl @stoptime()\n");
  sym_table_stack.insert_to_top("stoptime", "@stoptime()",
                                SymbolTables::SymbolKind::FUNC);

  // traverse all funcdef and var or const decl
  auto item_ptr =
      node.item.get();  // here item_ptr's type is either FuncDef or Decl
  while (item_ptr) {
    item_ptr->accept(*this);
    item_ptr = item_ptr->next_compunit_item.get();
  }
}

static inline std::string get_last_ir_line(
    std::unique_ptr<std::string>& ir_code) {
  int last_line_pos = ir_code->find_last_of('\n', ir_code->size() - 2);
  return ir_code->substr(last_line_pos + 1,
                         ir_code->size() - last_line_pos - 2);
}

bool is_last_line_label(std::unique_ptr<std::string>& ir_code) {
  auto line = get_last_ir_line(ir_code);
  return line.find(":") != std::string::npos;
}

void GenIRVisitor::visit(FuncDef& node) {
  std::cout << "genir visit funcdef " << node.ident << std::endl;
  // func symbol format: @func_name(@param: i32, ...): ret_type
  // or @func_name(@param: i32, ...)
  auto func_symbol = "@" + node.ident + "(";
  if (node.func_fparam) {
    auto ptr = node.func_fparam.get();
    while (ptr) {
      auto arr_ptr = dynamic_cast<FuncFParamArr*>(ptr);
      if (arr_ptr) {  // is array
        auto type_visitor = ArrTypeEvaluateVisitor(&sym_table_stack);
        arr_ptr->accept(type_visitor);
        func_symbol += "@" + ptr->ident + ": " + type_visitor.type_name + ",";
      } else {  // scalar
        func_symbol += "@" + ptr->ident + ": i32,";
      }
      ptr = ptr->next_func_fparam.get();
    }
    func_symbol.pop_back();
  }
  if (node.func_type->type_name == "int") {
    func_symbol += "): i32";
  } else {
    func_symbol += ")";
  }
  sym_table_stack.insert_to_top(node.ident, func_symbol,
                                SymbolTables::SymbolKind::FUNC);

  // initialize sym_table_stack for function scope
  sym_table_stack.push_table();

  // initialize temp counter
  reset_counter();

  // pruning the basic block to make sure
  // there is no stmt after ret stmt
  if (node.block_item) {
    auto pruning_ret_visitor = PruningRetVisitor();
    node.block_item->accept(pruning_ret_visitor);
  }

  // start to generate ir
  ir_code->append("fun " + func_symbol + " {\n%entry_"+node.ident+":\n");

  // allocate stack for fparam
  auto fparam_ptr = node.func_fparam.get();
  while (fparam_ptr) {
    fparam_ptr->accept(*this);
    fparam_ptr = fparam_ptr->next_func_fparam.get();
  }

  // here block_item is a list of blockitem
  auto block_item_ptr = node.block_item.get();
  while (block_item_ptr) {
    block_item_ptr->accept(*this);
    block_item_ptr = block_item_ptr->next_block_item.get();
  }
  // TODO: what if the function doesn't have a return statement
  // while it has a return type of int?????????
  if (is_last_line_label(ir_code) ||
      (get_last_ir_line(ir_code).find("ret") == std::string::npos &&
       get_last_ir_line(ir_code).find("jump") == std::string::npos &&
       get_last_ir_line(ir_code).find("br") == std::string::npos)) {
    // ir_code->append("  ret 0\n");
    if (node.func_type->type_name == "int") {
      ir_code->append("  ret 0\n");
    } else {
      ir_code->append("  ret\n");
    }
  }
  ir_code->append("}\n");
  sym_table_stack.pop_table();
}

void GenIRVisitor::visit(FuncFParamArr& node) {
  std::cout << "genir visit fparamarr" << std::endl;
  /**
   * 1. insert to symtable
   * 2. gen ir
   * for examples: int a[][3]
   * @arr = alloc *[i32, 3]        // @arr 的类型是 **[i32, 3]
   * %ptr1 = load @arr             // %ptr1 的类型是 *[i32, 3]
   * %ptr2 = getptr %ptr1, 1       // %ptr2 的类型是 *[i32, 3]
   * %ptr3 = getelemptr %ptr2, 2   // %ptr3 的类型是 *i32
   * %value = load %ptr3           // %value 的类型是 i32
   */
  auto fparam_name = "@" + node.ident;
  auto ir_symbol_name = "%" + node.ident;
  auto type_visitor = ArrTypeEvaluateVisitor(&sym_table_stack);
  node.accept(type_visitor);
  auto type_name = type_visitor.type_name;
  int dims = type_visitor.dims;
  sym_table_stack.insert_to_top(node.ident, type_name, dims,
                                SymbolTables::SymbolKind::PTR);
  ir_code->append("  " + ir_symbol_name + " = alloc " + type_name + "\n");
  ir_code->append("  store " + fparam_name + ", " + ir_symbol_name + "\n");
  // here we do not need to store since it's a pointer
}

void GenIRVisitor::visit(FuncFParam& node) {
  std::cout << "genir visit fparam" << std::endl;
  /**
   * 1. insert x->%x
   * 2. store @x->%x
   */
  auto var_symbol_name = "@" + node.ident;
  // sym_table_stack.insert_to_top(node.ident, var_symbol_name,
  // SymbolTables::SymbolKind::VAR);
  auto ir_symbol_name = "%" + node.ident;
  sym_table_stack.insert_to_top(node.ident, ir_symbol_name,
                                SymbolTables::SymbolKind::VAR);
  ir_code->append("  " + ir_symbol_name + " = alloc i32\n");
  ir_code->append("  store " + var_symbol_name + ", " + ir_symbol_name + "\n");

  push_result(ir_symbol_name);
}

void GenIRVisitor::visit(Type& node) {
  // assert(node.type_name == "int");
  // ir_code->append("i32");
  if (node.type_name == "int") {
    ir_code->append(": i32");
  } else {  // void
  }
}

std::string get_func_type(const std::string& func_symbol) {
  int len = func_symbol.size();
  // std::cout<<"get_func_type(func_symbol): "<<func_symbol<<std::endl;
  if (func_symbol.substr(len - 3) == "i32") {
    return "int";
  } else {
    return "void";
  }
}

void GenIRVisitor::visit(FuncCallExp& node) {
  /**
   * 1. prepare parameters
   * 2. search for function symbol in symbol table
   * 3. if void return type, then no need to push result
   * 4. if int return type, then push result
   */
  std::cout << "genir visit funccallexp: " << node.ident << std::endl;
  // prepare parameters
  auto rparams_ptr = node.rparam.get();
  std::vector<std::string> rparam;
  while (rparams_ptr) {
    rparams_ptr->accept(*this);
    rparam.push_back(pop_last_result());
    rparams_ptr = rparams_ptr->next_func_rparam.get();
  }

  // search for function symbol in symbol table
  // and prepare function call
  if (!sym_table_stack.find(node.ident, SymbolTables::SymbolKind::FUNC)) {
    throw std::runtime_error("undefined function symbol: " + node.ident);
  }
  auto func_symbol = std::get<std::string>(
      sym_table_stack.get(node.ident, SymbolTables::SymbolKind::FUNC));
  auto func_call = "call @" + node.ident + "(";
  for (auto p : rparam) {
    func_call += p + ", ";
  }
  if (rparam.size() > 0) {
    func_call.pop_back();
    func_call.pop_back();
  }
  func_call += ")";

  // using func_type to decide whether need to push result
  // auto func_type = func_symbol.substr(func_symbol.find(":")+1);
  auto func_type = get_func_type(func_symbol);
  if (func_type == "void") {
    ir_code->append("  " + func_call + "\n");
  } else {
    auto result_name = get_new_counter();
    ir_code->append("  " + result_name + " = " + func_call + "\n");
    push_result(result_name);
  }
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
  ir_code->append("  " + result_name + " = add " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(SubExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = sub " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(MulExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = mul " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(DivExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = div " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(ModExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = mod " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LTExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = lt " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(GTExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = gt " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(LEExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = le " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(GEExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = ge " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(EQExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = eq " + lhs_name + ", " + rhs_name +
                  "\n");
  push_result(result_name);
}

void GenIRVisitor::visit(NEExp& node) {
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  node.rhs->accept(*this);
  auto rhs_name = pop_last_result();
  auto result_name = get_new_counter();
  ir_code->append("  " + result_name + " = ne " + lhs_name + ", " + rhs_name +
                  "\n");
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
  // ir_code->append("  " + result_name + " = and " + temp_l + ", " + temp_r +
  // "\n"); push_result(result_name);

  /**
   * short-circuit evaluation
   */
  int count = sym_table_stack.total_accurrences("result",
                                                SymbolTables::SymbolKind::VAR);
  auto result_ident_name = "@result_" + std::to_string(count + 1);
  ir_code->append("  " + result_ident_name + " = alloc i32\n");
  ir_code->append("  store 0, " + result_ident_name + "\n");
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  auto tmp_1 = get_new_counter();
  ir_code->append("  " + tmp_1 + " = ne " + lhs_name + ", 0\n");
  auto then_label = "%then_" + std::to_string(block_label_counter);
  auto end_label = "%end_" + std::to_string(block_label_counter);
  block_label_counter++;
  ir_code->append("  br " + tmp_1 + ", " + then_label + ", " + end_label +
                  "\n");
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
  // ir_code->append("  " + temp_name + " = or " + lhs_name + ", " + rhs_name +
  // "\n"); ir_code->append("  " + result_name + " = ne " + temp_name + ",
  // 0\n"); push_result(result_name);

  /**
   * short-circuit evaluation
   */
  int count = sym_table_stack.total_accurrences("result",
                                                SymbolTables::SymbolKind::VAR);
  auto result_ident_name = "@result_" + std::to_string(count + 1);
  ir_code->append("  " + result_ident_name + " = alloc i32\n");
  ir_code->append("  store 1, " + result_ident_name + "\n");
  node.lhs->accept(*this);
  auto lhs_name = pop_last_result();
  auto tmp_1 = get_new_counter();
  ir_code->append("  " + tmp_1 + " = eq " + lhs_name + ", 0\n");
  auto then_label = "%then_" + std::to_string(block_label_counter);
  auto end_label = "%end_" + std::to_string(block_label_counter);
  block_label_counter++;
  ir_code->append("  br " + tmp_1 + ", " + then_label + ", " + end_label +
                  "\n");
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
  std::cout << "genir visit constdecl" << std::endl;
  auto const_def_ptr = node.const_def.get();
  while (const_def_ptr) {
    const_def_ptr->accept(*this);
    const_def_ptr = const_def_ptr->next_const_def.get();
  }
}

static void gen_array_type_recur(const std::vector<int>& shape, int layer,
                                 std::string& ret) {
  int num_dims = shape.size();
  if (layer == num_dims) {
    ret += "i32";
    return;
  }
  ret += "[";
  gen_array_type_recur(shape, layer + 1, ret);
  ret += ", " + std::to_string(shape[layer]) + "]";
}

void GenIRVisitor::visit(ArrayDims& node) {
  auto link_list_visitor = LinkListVisitor(&sym_table_stack);
  node.accept(link_list_visitor);
  auto& shape = link_list_visitor.result;
  std::string array_type;
  gen_array_type_recur(shape, 0, array_type);
  ir_code->append(array_type);
}

void GenIRVisitor::visit(ConstDef& node) {
  if (sym_table_stack.find_in_current(node.ident,
                                      SymbolTables::SymbolKind::CONST)) {
    throw std::runtime_error("redefined const symbol: " + node.ident +
                             " in current scope");
  }

  if (node.array_dims) {  // const array
    /**
     * 1. use ArrayEvaluator to evaluate const eval value
     * 2. store the result into symbol table
     * 3. compose ir
     *   3.1 if global:
     *   3.2 if local:
     */
    auto array_evaluator = ConstArrayEvaluateVisitor(&sym_table_stack);
    node.accept(array_evaluator);
    auto link_list_visitor = LinkListVisitor(&sym_table_stack);
    node.array_dims->accept(link_list_visitor);
    sym_table_stack.insert_to_top(node.ident, link_list_visitor.result,
                                  array_evaluator.result,
                                  SymbolTables::SymbolKind::CONST_ARR);
    auto sym_name = std::get<std::string>(
        sym_table_stack.get(node.ident, SymbolTables::SymbolKind::CONST_ARR));
    if (node.is_global) {
      ir_code->append("global " + sym_name + " = alloc ");
      node.array_dims->accept(*this);  // generate array type like [[i32, 3], 2]
      ir_code->append(", ");

      // generate like {10, 20}
      std::string const_init_code;
      int idx = 0;
      gen_const_arr_init_val_global_recur(link_list_visitor.result,
                                          array_evaluator.result, 0, idx,
                                          const_init_code);

      ir_code->append(const_init_code);
      ir_code->append("\n");
    } else {
      ir_code->append("  " + sym_name + " = alloc ");
      push_result(sym_name);
      node.array_dims->accept(*this);  // generate array type like [[i32, 3], 2]
      ir_code->append("\n");
      /**
       * store each value into array by using getelemptr
       * for example:
       * @arr = alloc [[i32, 3], 2]   // int a[2][3]
       * %ptr1 = getelemptr @arr, 0   // %ptr1  => *[i32, 3]
       * %ptr2 = getelemptr %ptr1, 0  // %ptr2  => *i32
       * store 0, %ptr2
       * %ptr3 = getelemptr %ptr1, 1  // %ptr3  => *i32
       * store 1, %ptr3
       */
      int idx = 0;
      std::string const_init_code;
      gen_const_arr_init_val_local_recur(link_list_visitor.result,
                                         array_evaluator.result, 0, idx,
                                         const_init_code);

      ir_code->append(const_init_code);
      ir_code->append("\n");
    }
  } else {  // const scalar
    assert(node.const_init_val != nullptr &&
           node.const_init_val->exp != nullptr);
    auto evaluate_visitor = EvaluateVisitor(&sym_table_stack);
    node.const_init_val->exp->accept(evaluate_visitor);
    sym_table_stack.insert_to_top(node.ident, evaluate_visitor.result);
  }
}

void GenIRVisitor::visit(RetStmt& node) {
  std::cout << "genir visit retstmt" << std::endl;
  if (node.exp) {
    node.exp->accept(*this);
    auto result_name = pop_last_result();
    ir_code->append("  ret " + result_name + "\n");
  } else {
    ir_code->append("  ret\n");
  }
  // generate ret label in order to end this basic block
  // ir_code->append("%ret_label_" + std::to_string(ret_label_counter) + ":\n");
  // ret_label_counter++;
  // node.next_block_item->accept(*this);    // TODO: should there be other
  // items after retstmt??
}

void GenIRVisitor::visit(AssignStmt& node) {
  std::cout << "genir visit assignstmt" << std::endl;
  // since const value must be declared
  // hence here lval must be var_symbol
  // auto find_var =
  //     sym_table_stack.find(node.lval->ident, SymbolTables::SymbolKind::VAR);
  // if (find_var == false) {
  //   throw std::runtime_error("undefined var symbol: " + node.lval->ident);
  // }
  // auto var_name = std::get<std::string>(
  //     sym_table_stack.get(node.lval->ident, SymbolTables::SymbolKind::VAR));

  std::string var_name;  // maybe a scalar or array
  auto kind = sym_table_stack.find(node.lval->ident);
  if (kind == SymbolTables::SymbolKind::VAR) {
    var_name = std::get<std::string>(
        sym_table_stack.get(node.lval->ident, SymbolTables::SymbolKind::VAR));
  } else if (kind == SymbolTables::SymbolKind::VAR_ARR) {
    // node.lval->accept(*this);
    /**
     * here we only need to get a ptr of the value in array, so we cannot visit
     * lval directly for example: %ptr1 = getelemptr @arr, %0 %ptr2 = getelemptr
     * %ptr1, %1 is enough
     */
    auto arr_sym_name = std::get<std::string>(sym_table_stack.get(
        node.lval->ident, SymbolTables::SymbolKind::VAR_ARR));
    auto index_ptr = node.lval->array_dims.get();
    assert(index_ptr != nullptr);
    std::stack<std::string> ptr_stack;
    ptr_stack.push(arr_sym_name);
    while (index_ptr) {
      // index_ptr->exp
      assert(index_ptr->exp != nullptr);
      index_ptr->exp->accept(*this);
      auto index_name = pop_last_result();
      auto rptr_name = ptr_stack.top();
      ptr_stack.pop();
      auto lptr_name = get_new_counter();
      ir_code->append("  " + lptr_name + " = getelemptr " + rptr_name + ", " +
                      index_name + "\n");
      ptr_stack.push(lptr_name);
      index_ptr = index_ptr->next_dim.get();
    }
    var_name = ptr_stack.top();
  } else if (kind == SymbolTables::SymbolKind::PTR) {
    // similar to VAR_ARR, we just don't need to load ptr at last compared to lval
    auto sym_name = std::get<std::string>(
        sym_table_stack.get(node.lval->ident, SymbolTables::SymbolKind::PTR));
    auto tmp_0 = get_new_counter();
    ir_code->append("  " + tmp_0 + " = load " + sym_name + "\n");
    auto tmp_1 = get_new_counter();
    auto ptr = node.lval->array_dims.get();
    assert(ptr != nullptr);
    ptr->exp->accept(*this);
    auto array_offset = pop_last_result();
    ir_code->append("  " + tmp_1 + " = getptr " + tmp_0 + ", " + array_offset + "\n");
    ptr = ptr->next_dim.get();
    while(ptr) {
      auto tmp_2 = get_new_counter();
      ptr->exp->accept(*this);
      auto array_offset = pop_last_result();
      ir_code->append("  " + tmp_2 + " = getelemptr " + tmp_1 + ", " + array_offset + "\n");
      tmp_1 = tmp_2;
      ptr = ptr->next_dim.get();
    }
    var_name = tmp_1;
  } else {
    throw std::runtime_error("undefined var symbol: " + node.lval->ident);
  }

  // start to compose ir
  // First we have to evaluate exp and push the result
  // Then we load lval and store the result
  node.exp->accept(*this);
  auto store_counter_name = pop_last_result();
  ir_code->append("  store " + store_counter_name + ", " + var_name + "\n");
}

void GenIRVisitor::visit(ExpStmt& node) {
  if (node.exp.get() == nullptr) {
    std::cout << "genir visit empty stmt" << std::endl;
    return;
  } else {
    std::cout << "genir visit non-empty exp stmt" << std::endl;
    node.exp->accept(*this);
  }
}

void GenIRVisitor::visit(BlockStmt& node) {
  std::cout << "genir visit blockstmt" << std::endl;

  sym_table_stack.push_table();
  auto block_item_ptr = node.block_item.get();
  while (block_item_ptr) {
    block_item_ptr->accept(*this);
    block_item_ptr = block_item_ptr->next_block_item.get();
  }
  sym_table_stack.pop_table();
}

void GenIRVisitor::visit(IfStmt& node) {
  std::cout << "genir visit ifstmt" << std::endl;
  node.cond->accept(*this);
  auto cond_name = pop_last_result();
  if (node.else_body) {
    auto then_label = "%then_" + std::to_string(block_label_counter);
    auto else_label = "%else_" + std::to_string(block_label_counter);
    auto end_label = "%end_" + std::to_string(block_label_counter);
    block_label_counter++;
    ir_code->append("  br " + cond_name + ", " + then_label + ", " +
                    else_label + "\n");

    // handle then_body
    ir_code->append(then_label + ":\n");
    node.then_body->accept(*this);
    // TODO: this way is pretty hacky
    if (get_last_ir_line(ir_code).substr(2, 3) != "ret" &&
        get_last_ir_line(ir_code).substr(2, 4) != "jump" &&
        get_last_ir_line(ir_code).substr(2, 2) != "br") {
      ir_code->append("  jump " + end_label + "\n");
    }

    // handle else_body
    ir_code->append(else_label + ":\n");
    node.else_body->accept(*this);
    if (get_last_ir_line(ir_code).substr(2, 3) != "ret" &&
        get_last_ir_line(ir_code).substr(2, 4) != "jump" &&
        get_last_ir_line(ir_code).substr(2, 2) != "br") {
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
    ir_code->append("  br " + cond_name + ", " + then_label + ", " + end_label +
                    "\n");

    // handle then_body
    ir_code->append(then_label + ":\n");
    node.then_body->accept(*this);
    if (get_last_ir_line(ir_code).substr(2, 3) != "ret" &&
        get_last_ir_line(ir_code).substr(2, 4) != "jump" &&
        get_last_ir_line(ir_code).substr(2, 2) != "br") {
      ir_code->append("  jump " + end_label + "\n");
    }

    // add end_label
    ir_code->append(end_label + ":\n");
  }
}

void GenIRVisitor::visit(WhileStmt& node) {
  std::cout << "genir visit whilestmt" << std::endl;
  auto while_entry_name = "%while_entry_" + std::to_string(block_label_counter);
  auto while_body_name = "%while_body_" + std::to_string(block_label_counter);
  auto while_end_name = "%while_end_" + std::to_string(block_label_counter);
  while_stack.push_while_label(while_entry_name, while_end_name);
  block_label_counter++;
  ir_code->append("  jump " + while_entry_name + "\n");
  ir_code->append(while_entry_name + ":\n");
  node.cond->accept(*this);
  auto cond_name = pop_last_result();
  ir_code->append("  br " + cond_name + ", " + while_body_name + ", " +
                  while_end_name + "\n");
  ir_code->append(while_body_name + ":\n");
  node.body->accept(*this);
  // TODO: this way is pretty hacky
  if (get_last_ir_line(ir_code).substr(2, 3) != "ret" &&
      get_last_ir_line(ir_code).substr(2, 4) != "jump" &&
      get_last_ir_line(ir_code).substr(2, 2) != "br") {
    ir_code->append("  jump " + while_entry_name + "\n");
  }
  ir_code->append(while_end_name + ":\n");
  while_stack.pop_while_label();
}

void GenIRVisitor::visit(BreakStmt& node) {
  std::cout << "genir visit breakstmt" << std::endl;
  auto while_end_name = while_stack.get_top_while_end();
  ir_code->append("  jump " + while_end_name + "\n");
}

void GenIRVisitor::visit(ContinueStmt& node) {
  std::cout << "genir visit continuestmt" << std::endl;
  auto while_entry_name = while_stack.get_top_while_entry();
  ir_code->append("  jump " + while_entry_name + "\n");
}

void GenIRVisitor::visit(LValExp& node) {
  // auto find_it = sym_table.find(node.ident);

  auto kind = sym_table_stack.find(node.ident);
  switch (kind) {
    case SymbolTables::SymbolKind::CONST: {
      int value = std::get<int>(
          sym_table_stack.get(node.ident, SymbolTables::SymbolKind::CONST));
      push_result(std::to_string(value));
      break;
    }
    case SymbolTables::SymbolKind::VAR: {
      auto var_name = std::get<std::string>(
          sym_table_stack.get(node.ident, SymbolTables::SymbolKind::VAR));
      auto load_counter_name = get_new_counter();
      ir_code->append("  " + load_counter_name + " = load " + var_name + "\n");
      push_result(load_counter_name);
      break;
    }
    case SymbolTables::SymbolKind::CONST_ARR: {
      std::cout << "gen lval const arr: " << node.ident << "\n";
      /**
       * 1. prepare indices
       * 2. sequentially getelemptr. for example:
       * %ptr1 = getelemptr @arr, %0
       * %ptr2 = getelemptr %ptr1, %1
       * %val = load %ptr2
       */
      auto arr_sym_name = std::get<std::string>(
          sym_table_stack.get(node.ident, SymbolTables::SymbolKind::CONST_ARR));
      auto index_ptr = node.array_dims.get();
      assert(index_ptr != nullptr);
      std::stack<std::string> ptr_stack;
      ptr_stack.push(arr_sym_name);
      while (index_ptr) {
        // index_ptr->exp
        assert(index_ptr->exp != nullptr);
        index_ptr->exp->accept(*this);
        auto index_name = pop_last_result();
        auto rptr_name = ptr_stack.top();
        ptr_stack.pop();
        auto lptr_name = get_new_counter();
        ir_code->append("  " + lptr_name + " = getelemptr " + rptr_name + ", " +
                        index_name + "\n");
        ptr_stack.push(lptr_name);
        index_ptr = index_ptr->next_dim.get();
      }
      auto result_name = get_new_counter();
      ir_code->append("  " + result_name + " = load " + ptr_stack.top() + "\n");
      push_result(result_name);

      break;
    }
    case SymbolTables::SymbolKind::VAR_ARR: {
      /**
       * for examples: int a[2][3]; f(a[1])
       * @a // *[[i32, 3], 2]
       * %0 = getelemptr @a, 1 // *[i32, 3]
       * %1 = getelemptr %0, 0 // *i32
       * 
       * // f(a[1][2])
       * %0 = getelemptr @a, 1 // *[i32, 3]
       * %1 = getelemptr %0, 2 // *i32
       * %2 = load %1 
       */
      std::cout << "gen lval var arr: " << node.ident << "\n";
      auto arr_sym_name = std::get<std::string>(
          sym_table_stack.get(node.ident, SymbolTables::SymbolKind::VAR_ARR));
      auto index_ptr = node.array_dims.get();
      // assert(index_ptr != nullptr);
      std::stack<std::string> ptr_stack;
      ptr_stack.push(arr_sym_name);
      int array_dim = sym_table_stack.get_var_arr_info(node.ident).dims.size();
      int refer_dim = 0;
      while (index_ptr) {
        // index_ptr->exp
        refer_dim += 1;
        assert(index_ptr->exp != nullptr);
        index_ptr->exp->accept(*this);
        auto index_name = pop_last_result();
        auto rptr_name = ptr_stack.top();
        ptr_stack.pop();
        auto lptr_name = get_new_counter();
        ir_code->append("  " + lptr_name + " = getelemptr " + rptr_name + ", " +
                        index_name + "\n");
        ptr_stack.push(lptr_name);
        index_ptr = index_ptr->next_dim.get();
      }
      /**
       * 1. when referred to a value in array, load it
       * 2. when referred to an array, use getelemptr to convert to ptr
       */
      if(refer_dim < array_dim) {
        auto result_name = get_new_counter();
        ir_code->append("  " + result_name + " = getelemptr " + ptr_stack.top() + ", 0\n");
        push_result(result_name);
      } else {
        auto result_name = get_new_counter();
        ir_code->append("  " + result_name + " = load " + ptr_stack.top() + "\n");
        push_result(result_name);
      }
      break;
    }
    case SymbolTables::SymbolKind::PTR: {
      /*
       * for examples: f(int arr[]) return a[1]
       * %0 = load %arr // *i32
       * %1 = getptr %0, 1 // *i32
       * %2 = load %1 // i32
       * 
       * f(int arr[][3]) return arr[1][2]
       * %0 = load %arr // *[i32, 3]
       * %1 = getptr %0, 1 // *[i32, 3]
       * %2 = getelemptr %1, 2 // *i32
       * %3 = load %2 // i32
       * 
       * f(int arr[][3][4]) return arr[1][2][3]
       * %0 = load %arr // *[[i32, 4], 3]
       * %1 = getptr %0, 1 // *[[i32, 4], 3]
       * %2 = getelemptr %1, 2 // *[i32, 4]
       * %3 = getelemptr %2, 3 // *i32
       * %4 = load %3 // i32
       * 
       * f(int arr[][10]) return arr[1]
       * %0 = load %arr // *[i32, 10]
       * %1 = getptr %0, 1 // *[i32, 10]
       * %2 = getelemptr %1, 0 // *i32
       * 
       * f(int arr[]) g(arr);
       * %0 = load %arr // *i32
       */
      std::cout<<"gen lval ptr: "<<node.ident<<"\n";
      auto sym_name = sym_table_stack.get_ptr_info(node.ident).sym_name;
      int array_dim = sym_table_stack.get_ptr_info(node.ident).dims;
      auto tmp_0 = get_new_counter();
      ir_code->append("  " + tmp_0 + " = load " + sym_name + "\n");
      auto ptr = node.array_dims.get();
      // assert(ptr != nullptr);
      if(ptr == nullptr){
        push_result(tmp_0);
        return;
      }
      auto tmp_1 = get_new_counter();
      ptr->exp->accept(*this);
      auto array_offset = pop_last_result();
      ir_code->append("  " + tmp_1 + " = getptr " + tmp_0 + ", " + array_offset + "\n");
      ptr = ptr->next_dim.get();
      int refer_dim = 1;
      while(ptr) {
        refer_dim += 1;
        auto tmp_2 = get_new_counter();
        ptr->exp->accept(*this);
        auto array_offset = pop_last_result();
        ir_code->append("  " + tmp_2 + " = getelemptr " + tmp_1 + ", " + array_offset + "\n");
        tmp_1 = tmp_2;
        ptr = ptr->next_dim.get();
      }
      if(refer_dim < array_dim) {
        auto result_name = get_new_counter();
        ir_code->append("  " + result_name + " = getelemptr " + tmp_1 + ", 0\n");
        push_result(result_name);
      } else {
        auto result_name = get_new_counter();
        ir_code->append("  " + result_name + " = load " + tmp_1 + "\n");
        push_result(result_name);
      }
      break;
    }
    default: {
      assert(0);
    }
  }
}

void GenIRVisitor::visit(VarDecl& node) {
  std::cout << "genir visit vardecl" << std::endl;
  auto var_def_ptr = node.var_def.get();
  while (var_def_ptr) {
    var_def_ptr->accept(*this);
    var_def_ptr = var_def_ptr->next_var_def.get();
  }
}

void GenIRVisitor::visit(VarDef& node) {
  if (sym_table_stack.find_in_current(node.ident,
                                      SymbolTables::SymbolKind::VAR)) {
    throw std::runtime_error("redefined var symbol: " + node.ident);
  }

  if (node.array_dims) {  // array
    /**
     * 1. use VarArrayEvaluate to store either Exp* or int(0) into symbol table
     * 2. if global, evaluate each array value
     * 3. if local, prepare each array value
     */
    std::cout << "gen var array\n";
    auto array_evaluator = VarArrayEvaluateVisitor(&sym_table_stack);
    node.accept(array_evaluator);
    auto link_list_visitor = LinkListVisitor(&sym_table_stack);
    node.array_dims->accept(link_list_visitor);
    sym_table_stack.insert_to_top(node.ident, link_list_visitor.result,
                                  array_evaluator.result,
                                  SymbolTables::SymbolKind::VAR_ARR);
    auto sym_name = std::get<std::string>(
        sym_table_stack.get(node.ident, SymbolTables::SymbolKind::VAR_ARR));

    if (node.is_global) {
      std::cout << "gen global var array\n";
      ir_code->append("global " + sym_name + " = alloc ");
      node.array_dims->accept(*this);  // generate array type like [[i32, 3], 2]
      ir_code->append(", ");

      // generate like {10, 20}. should evaluate each array value
      int idx = 0;
      std::string init_code;
      if(node.var_init_val->exp == nullptr && node.var_init_val->array_init_val_hierarchy.size() == 0){
        // zeroinit
        // TODO: note that this kind of way to judge whether to use zero init 
        // is pretty hacky. Try to find a better way to do this
        init_code = "zeroinit";
      } else {
        // {0,1,2...}
        gen_var_arr_init_val_global_recur(
            link_list_visitor.result, array_evaluator.result, 0, idx, init_code);
      }
      ir_code->append(init_code);
      ir_code->append("\n");
    } else {
      std::cout << "gen local var array\n";
      ir_code->append("  " + sym_name + " = alloc ");
      push_result(sym_name);
      node.array_dims->accept(*this);  // generate array type like [[i32, 3], 2]
      ir_code->append("\n");

      // prepare each array value
      int idx = 0;
      std::string init_code;
      gen_var_arr_init_val_local_recur(
          link_list_visitor.result, array_evaluator.result, 0, idx, init_code);
      ir_code->append(init_code);
      ir_code->append("\n");
    }

  } else {  // single scalar
    std::cout << "gen var scalar\n";
    int count = sym_table_stack.total_accurrences(
        node.ident, SymbolTables::SymbolKind::VAR);
    auto var_symbol_name = "@" + node.ident + "_" + std::to_string(count + 1);
    // sym_table[node.ident] = var_symbol_name;
    sym_table_stack.insert_to_top(node.ident, var_symbol_name,
                                  SymbolTables::SymbolKind::VAR);
    std::cout << "store var " << var_symbol_name << " into symbol table"
              << std::endl;

    // start to compose ir code
    if (node.is_global) {
      ir_code->append("global " + var_symbol_name + " = alloc i32, ");
      if (node.var_init_val) {
        auto evaluate_visitor = EvaluateVisitor(&sym_table_stack);
        node.var_init_val->exp->accept(evaluate_visitor);
        ir_code->append(std::to_string(evaluate_visitor.result) + "\n");
      } else {
        ir_code->append("zeroinit\n");
      }
    } else {
      ir_code->append("  " + var_symbol_name + " = alloc i32\n");
      if (node.var_init_val) {
        node.var_init_val->exp->accept(*this);
        auto result = pop_last_result();
        ir_code->append("  store " + result + ", " + var_symbol_name + "\n");
      }
    }
  }
}

void GenIRVisitor::gen_const_arr_init_val_local_recur(
    const std::vector<int>& shape, const std::vector<int>& data, int layer,
    int& idx, std::string& ret) {
  assert(shape.size() >= 1);
  int num_dims = shape.size();
  if (layer == num_dims) {
    auto last_result = pop_last_result();
    ir_code->append("  store " + std::to_string(data[idx]) + ", " +
                    last_result + "\n");
    idx++;
    return;
  }
  for (int i = 0; i < shape[layer]; ++i) {
    auto temp_ptr = get_new_counter("ptr");
    ir_code->append("  " + temp_ptr + " = getelemptr " + peek_last_result() +
                    ", " + std::to_string(i) + "\n");
    push_result(temp_ptr);
    gen_const_arr_init_val_local_recur(shape, data, layer + 1, idx, ret);
  }
  pop_last_result();
}

void GenIRVisitor::gen_const_arr_init_val_global_recur(
    const std::vector<int>& shape, const std::vector<int>& data, int layer,
    int& idx, std::string& ret) {
  assert(shape.size() >= 1);
  int num_dims = shape.size();
  if (layer == num_dims) {
    ret += std::to_string(data[idx]);
    idx++;
    return;
  }
  ret += "{";
  for (int i = 0; i < shape[layer]; ++i) {
    gen_const_arr_init_val_global_recur(shape, data, layer + 1, idx, ret);
    if (i != shape[layer] - 1) {
      ret += ", ";
    }
  }
  ret += "}";
}

void GenIRVisitor::gen_var_arr_init_val_local_recur(
    const std::vector<int>& shape,
    const std::vector<std::variant<Exp*, int>>& data, int layer, int& idx,
    std::string& ret) {
  assert(shape.size() >= 1);
  int num_dims = shape.size();
  if (layer == num_dims) {
    auto last_result = pop_last_result();
    if (data[idx].index() == 0) {
      // std::cout<<"gen var arr init val local recur "<<idx<<std::endl;
      Exp* exp = std::get<Exp*>(data[idx]);
      exp->accept(*this);
      auto result_name = pop_last_result();
      ir_code->append("  store " + result_name + ", " + last_result + "\n");
    } else {
      // std::cout<<"gen var arr init val local recur "<<idx<<std::endl;
      ir_code->append("  store " + std::to_string(std::get<int>(data[idx])) +
                      ", " + last_result + "\n");
    }
    idx++;
    return;
  }
  for (int i = 0; i < shape[layer]; ++i) {
    auto temp_ptr = get_new_counter("ptr");
    ir_code->append("  " + temp_ptr + " = getelemptr " + peek_last_result() +
                    ", " + std::to_string(i) + "\n");
    push_result(temp_ptr);
    gen_var_arr_init_val_local_recur(shape, data, layer + 1, idx, ret);
  }
  pop_last_result();
}

void GenIRVisitor::gen_var_arr_init_val_global_recur(
    const std::vector<int>& shape,
    const std::vector<std::variant<Exp*, int>>& data, int layer, int& idx,
    std::string& ret) {
  assert(shape.size() >= 1);
  int num_dims = shape.size();
  if (layer == num_dims) {
    // ret += std::to_string(std::get<int>(data[idx]));
    if (data[idx].index() == 0) {
      auto evaluator = EvaluateVisitor(&sym_table_stack);
      std::get<Exp*>(data[idx])->accept(evaluator);
      ret += std::to_string(evaluator.result);
    } else {
      ret += std::to_string(std::get<int>(data[idx]));
    }
    idx++;
    return;
  }
  ret += "{";
  for (int i = 0; i < shape[layer]; ++i) {
    gen_var_arr_init_val_global_recur(shape, data, layer + 1, idx, ret);
    if (i != shape[layer] - 1) {
      ret += ", ";
    }
  }
  ret += "}";
}