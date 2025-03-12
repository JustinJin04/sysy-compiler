#include <ast.hpp>
#include <cassert>

static inline std::string indent(int depth) { return std::string(2 * depth, ' '); }

static inline int get_operand_num(ExpKind kind) {
  switch (kind) {
    case ExpKind::NUMBER:
      return 0;
    case ExpKind::POSITIVE:
    case ExpKind::NEGATIVE:
    case ExpKind::LOGICAL_NOT:
      return 1;
    case ExpKind::ADD:
    case ExpKind::SUB:
    case ExpKind::MUL:
    case ExpKind::DIV:
    case ExpKind::REM:

    case ExpKind::LT:
    case ExpKind::GT:
    case ExpKind::LEQ:
    case ExpKind::GEQ:
    case ExpKind::EQ:
    case ExpKind::NEQ:

    case ExpKind::LAND:
    case ExpKind::LOR:
      return 2;
    default:
      assert(0);
  }
}

static inline std::string opkind_to_ir(ExpKind kind) {
  assert(get_operand_num(kind) == 2);
  switch (kind) {
    case ExpKind::ADD:
      return "add";
    case ExpKind::SUB:
      return "sub";
    case ExpKind::MUL:
      return "mul";
    case ExpKind::DIV:
      return "div";
    case ExpKind::REM:
      return "mod";
    
    case ExpKind::LT:
      return "lt";
    case ExpKind::GT:
      return "gt";
    case ExpKind::LEQ:
      return "le";
    case ExpKind::GEQ:
      return "ge";
    case ExpKind::EQ:
      return "eq";
    case ExpKind::NEQ:
      return "ne";
    
    case ExpKind::LAND:
      return "and";
    case ExpKind::LOR:
      return "or";
    default:
      assert(0);
  }
}


void CompUnitAST::Dump(int depth) const {
  std::cout << indent(depth) << "CompUnitAST {\n";
  func_def->Dump(depth + 1);
  std::cout << indent(depth) << "}\n";
}

void CompUnitAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  func_def->GenerateIR(ir, ctx);
}

void FuncDefAST::Dump(int depth) const {
  std::cout << indent(depth) << "FuncDefAST {\n";
  func_type->Dump(depth + 1);
  std::cout << indent(depth + 1) << IDENT << ",\n";
  block->Dump(depth + 1);
  std::cout << indent(depth) << "}\n";
}

void FuncDefAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  // std::cout<<indent(depth)<<"fun @"<<ident<<"(): ";
  ir->append(indent(ctx.depth)+"fun @"+IDENT+"(): ");
  func_type->GenerateIR(ir, ctx);
  // std::cout<<" {\n";
  // std::cout<<"%entry:\n";
  ir->append(" {\n%entry:\n");
  ctx.depth += 1;
  block->GenerateIR(ir, ctx);
  ctx.depth -= 1;
  // std::cout<<indent(depth)<<"}\n";
  ir->append(indent(ctx.depth)+"}\n");
}

void FuncTypeAST::Dump(int depth) const {
  std::cout << indent(depth) << "FuncTypeAST {\n";
  std::cout << indent(depth + 1) << TYPE_NAME << "\n";
  std::cout << indent(depth) << "},\n";
}

void FuncTypeAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  // assert(ctx.depth==0);
  assert(TYPE_NAME == "int");
  // std::cout<<"i32";
  ir->append("i32");
}

void BlockAST::Dump(int depth) const {
  std::cout << indent(depth) << "BlockAST {\n";
  stmt->Dump(depth + 1);
  std::cout << indent(depth) << "}\n";
}

void BlockAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  stmt->GenerateIR(ir, ctx);
}

void StmtAST::Dump(int depth) const {
  std::cout << indent(depth) << "StmtAST {\n";
  // std::cout << indent(depth + 1) << number << "\n";
  exp->Dump(depth+1);
  std::cout << indent(depth) << "}\n";
}

void StmtAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  // std::cout<<indent(depth) <<"ret "<<number<<"\n";
  // ir->append(indent(depth)+"ret "+std::to_string(number)+"\n");
  exp->GenerateIR(ir, ctx);
  // TODO: 默认当前最后一个temp counter就是返回值
  
  ir->append(indent(ctx.depth)+"ret "+ctx.pop_last_result());
  ir->append("\n");
}

void ExpAST::Dump(int depth) const {
  // unaryexp->Dump(depth);
  assert(0);
}

void ExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  int op_num = get_operand_num(kind);
  switch (op_num) {
    case 0: {
      ctx.push_result(number);
      break;
    }
    case 1: {
      rhs->GenerateIR(ir, ctx);
      auto rhs_name = ctx.pop_last_result();
      switch (kind) {
        case ExpKind::NEGATIVE: {
          auto result_name = ctx.get_new_counter();
          ir->append(indent(ctx.depth)+result_name+" = sub 0, "+rhs_name+"\n");
          ctx.push_result(result_name);
          break;
        }
        case ExpKind::LOGICAL_NOT: {
          auto result_name = ctx.get_new_counter();
          ir->append(indent(ctx.depth)+result_name+" = eq "+rhs_name + ", 0\n");
          ctx.push_result(result_name);
          break;
        }
        default: {
          assert(0);
        }
      }
      break;
    }
    case 2: {
      lhs->GenerateIR(ir, ctx);
      auto lhs_name = ctx.pop_last_result();
      rhs->GenerateIR(ir, ctx);
      auto rhs_name = ctx.pop_last_result();
      auto result_name = ctx.get_new_counter();
      ctx.push_result(result_name);
      auto op_str = opkind_to_ir(kind);
      // TODO: 暂时对逻辑and和or进行特殊处理。后续添加更多指令后再考虑抽象出IR指令类
      if (kind == ExpKind::LAND) {
        auto temp_l = ctx.get_new_counter();
        auto temp_r = ctx.get_new_counter();
        ir->append(indent(ctx.depth) + temp_l + " = ne " + lhs_name + ", 0\n");
        ir->append(indent(ctx.depth) + temp_r + " = ne " + rhs_name + ", 0\n");
        ir->append(indent(ctx.depth) + result_name + " = and " + temp_l + ", " + temp_r + "\n");
      } else if (kind == ExpKind::LOR) {
        ctx.pop_last_result();
        auto final_result_name = ctx.get_new_counter();
        ctx.push_result(final_result_name);
        ir->append(indent(ctx.depth) + result_name + " = or " + lhs_name + ", " + rhs_name + "\n");
        ir->append(indent(ctx.depth) + final_result_name + " = ne " + result_name + ", 0\n");
      } else {
        ir->append(indent(ctx.depth) + result_name + " = "+ op_str +" " + lhs_name + ", " + rhs_name + "\n");
      }
      // ir->append(indent(ctx.depth) + result_name + " = "+ op_str +" " + lhs_name + ", " + rhs_name + "\n");
      // if (kind == ExpKind::LAND || kind == ExpKind::LOR) {
      //   auto new_result_name = ctx.get_new_counter();
      //   ctx.pop_last_result();
      //   ctx.push_result(new_result_name);
      //   ir->append(indent(ctx.depth) + new_result_name + " = ne " + result_name + ", 0\n");
      // }
      break;
    }
    default:
      assert(0);
  }
}

// void UnaryExpAST::Dump(int depth) const {
//   if (kind == UnaryExpKind::Primary) {
//     primaryexp->Dump(depth);
//   } else if (kind == UnaryExpKind::UnaryOp) {
//     assert(0);
//   }
// }

// void UnaryExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   if (kind == UnaryExpKind::Primary) {
//     primaryexp->GenerateIR(ir, ctx);
//   } else if (kind == UnaryExpKind::UnaryOp) {
//     unaryexp->GenerateIR(ir, ctx);
//     unaryop->GenerateIR(ir, ctx);
//   }
// }

// void PrimaryExpAST::Dump(int depth) const {
//   assert(0);
// }

// void PrimaryExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   if (kind == PrimaryExpKind::Exp) {
//     exp->GenerateIR(ir, ctx);
//   } else if (kind == PrimaryExpKind::Number) {
//     number->GenerateIR(ir, ctx);
//   }
// }

// void UnaryOpAST::Dump(int depth) const {
//   assert(0);
// }

// void UnaryOpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   switch (kind) {
//     case UnaryOpKind::Add: {
//       break;
//     }
//     case UnaryOpKind::Sub: {
//       auto new_counter_str = ctx.get_new_counter();
//       auto last_result_str = ctx.pop_last_result();
//       ir->append(indent(ctx.depth)+new_counter_str+" = sub 0, "+last_result_str+"\n");
//       // ctx.set_counter(new_counter_str);
//       ctx.push_result(new_counter_str);
//       break;
//     }
//     case UnaryOpKind::LogicalNot: {
//       auto new_counter_str = ctx.get_new_counter();
//       auto last_result_str = ctx.pop_last_result();
//       ir->append(indent(ctx.depth)+new_counter_str+" = eq "+last_result_str + ", 0\n");
//       // ctx.set_counter(new_counter_str);
//       ctx.push_result(new_counter_str);
//       break;
//     }
//     default:
//       assert(0);
//   }
// }

// void MulExpAST::Dump(int depth) const {
//   assert(0);
// }

// void MulExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   switch (kind) {
//     case MulExpKind::UnaryExp: {
//       unaryexp->GenerateIR(ir, ctx);
//       break;
//     }
//     case MulExpKind::BinaryExp: {
//       mulexp->GenerateIR(ir, ctx);
//       unaryexp->GenerateIR(ir, ctx);
//       binarymulopast->GenerateIR(ir, ctx);
//       break;
//     }
//     default:{
//       assert(0);
//     }
//   }
// }

// void BinaryMulOpAST::Dump(int depth) const {
//   assert(0);
// }

// void BinaryMulOpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   auto unaryexp_name = ctx.pop_last_result();
//   auto mulexp_name = ctx.pop_last_result();
//   auto result_name = ctx.get_new_counter();
//   ctx.push_result(result_name);
//   switch (kind) {
//     case BinaryMulOpKind::Mul: {
//       ir->append(indent(ctx.depth) + result_name + " = mul " + mulexp_name + ", " + unaryexp_name + "\n");
//       break;
//     }
//     case BinaryMulOpKind::Div: {
//       ir->append(indent(ctx.depth) + result_name + " = div " + mulexp_name + ", " + unaryexp_name + "\n");
//       break;
//     }
//     case BinaryMulOpKind::Mod: {
//       ir->append(indent(ctx.depth) + result_name + " = rem " + mulexp_name + ", " + unaryexp_name + "\n");
//       break;
//     }
//     default: {
//       assert(0);
//     }
//   }
// }

// void AddExpAST::Dump(int depth) const {
//   assert(0);
// }

// void AddExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   switch (kind) {
//     case AddExpKind::MulExp: {
//       mulexp->GenerateIR(ir, ctx);
//       break;
//     }
//     case AddExpKind::AddMulExp: {
//       addexp->GenerateIR(ir, ctx);
//       mulexp->GenerateIR(ir, ctx);
//       binaryaddopast->GenerateIR(ir, ctx);
//       break;
//     }
//     default: {
//       assert(0);
//     }
//   }
// }

// void BinaryAddOpAST::Dump(int depth) const {
//   assert(0);
// }

// void BinaryAddOpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
//   auto mul_name = ctx.pop_last_result();
//   auto add_name = ctx.pop_last_result();
//   auto result_name = ctx.get_new_counter();
//   ctx.push_result(result_name);

//   switch (kind) {
//     case BinaryAddOpKind::Add: {
//       ir->append(indent(ctx.depth) + result_name + " = add " + add_name + ", " + mul_name + "\n");
//       break;
//     }
//     case BinaryAddOpKind::Sub: {
//       ir->append(indent(ctx.depth) + result_name + " = sub " + add_name + ", " + mul_name + "\n");
//       break;
//     }
//     default: {
//       assert(0);
//     }
//   }
// }