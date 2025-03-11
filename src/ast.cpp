#include <ast.hpp>
#include <cassert>

static std::string indent(int depth) { return std::string(2 * depth, ' '); }

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
  
  ir->append(indent(ctx.depth)+"ret "+ctx.get_last_result());
  ir->append("\n");
}

void NumberAST::Dump(int depth) const {
  std::cout<<indent(depth)<<"\n";
}

void NumberAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  // ir->append(std::to_string(INT_CONST));
  ctx.set_number(INT_CONST);
}

void ExpAST::Dump(int depth) const {
  unaryexp->Dump(depth);
}

void ExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  unaryexp->GenerateIR(ir, ctx);
}

void UnaryExpAST::Dump(int depth) const {
  if (kind == UnaryExpKind::Primary) {
    primaryexp->Dump(depth);
  } else if (kind == UnaryExpKind::UnaryOp) {
    assert(0);
  }
}

void UnaryExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  if (kind == UnaryExpKind::Primary) {
    primaryexp->GenerateIR(ir, ctx);
  } else if (kind == UnaryExpKind::UnaryOp) {
    unaryexp->GenerateIR(ir, ctx);
    unaryop->GenerateIR(ir, ctx);
  }
}

void PrimaryExpAST::Dump(int depth) const {
  assert(0);
}

void PrimaryExpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  if (kind == PrimaryExpKind::Exp) {
    exp->GenerateIR(ir, ctx);
  } else if (kind == PrimaryExpKind::Number) {
    number->GenerateIR(ir, ctx);
  }
}

void UnaryOpAST::Dump(int depth) const {
  assert(0);
}

void UnaryOpAST::GenerateIR(std::unique_ptr<std::string>& ir, IRContext& ctx) const {
  switch (kind) {
    case UnaryOpKind::Add: {
      break;
    }
    case UnaryOpKind::Sub: {
      auto new_counter_str = ctx.get_new_counter();
      auto last_result_str = ctx.get_last_result();
      ir->append(indent(ctx.depth)+new_counter_str+" = sub 0, "+last_result_str+"\n");
      ctx.set_counter(new_counter_str);
      break;
    }
    case UnaryOpKind::LogicalNot: {
      auto new_counter_str = ctx.get_new_counter();
      auto last_result_str = ctx.get_last_result();
      ir->append(indent(ctx.depth)+new_counter_str+" = eq 0, "+last_result_str+"\n");
      ctx.set_counter(new_counter_str);
      break;
    }
    default:
      assert(0);
  }
}

