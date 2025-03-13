#include "ast.hpp"

namespace AST {

void CompUnit::accept(Visitor& v) { v.visit(*this); }
void FuncDef::accept(Visitor& v) { v.visit(*this); }
void FuncType::accept(Visitor& v) { v.visit(*this); }

void ConstDecl::accept(Visitor& v) { v.visit(*this); }
void ConstDef::accept(Visitor& v) { v.visit(*this); }
void VarDecl::accept(Visitor& v) { v.visit(*this); }
void VarDef::accept(Visitor& v) { v.visit(*this); }
void BType::accept(Visitor& v) { v.visit(*this); }

void RetStmt::accept(Visitor& v) { v.visit(*this); }
void AssignStmt::accept(Visitor& v) { v.visit(*this); }

void Exp::accept(Visitor& v) {
  // v.visit(*this);
  throw std::runtime_error("Exp is an abstract class");
}
void NumberExp::accept(Visitor& v) { v.visit(*this); }
void LValExp::accept(Visitor& v) { v.visit(*this); }
void UnaryExp::accept(Visitor& v) {
  throw std::runtime_error("UnaryExp is an abstract class");
}
void NegativeExp::accept(Visitor& v) { v.visit(*this); }
void LogicalNotExp::accept(Visitor& v) { v.visit(*this); }
void BinaryExp::accept(Visitor& v) {
  throw std::runtime_error("BinaryExp is an abstract class");
}
void AddExp::accept(Visitor& v) { v.visit(*this); }
void SubExp::accept(Visitor& v) { v.visit(*this); }
void MulExp::accept(Visitor& v) { v.visit(*this); }
void DivExp::accept(Visitor& v) { v.visit(*this); }
void ModExp::accept(Visitor& v) { v.visit(*this); }
void LTExp::accept(Visitor& v) { v.visit(*this); }
void GTExp::accept(Visitor& v) { v.visit(*this); }
void LEExp::accept(Visitor& v) { v.visit(*this); }
void GEExp::accept(Visitor& v) { v.visit(*this); }
void EQExp::accept(Visitor& v) { v.visit(*this); }
void NEExp::accept(Visitor& v) { v.visit(*this); }
void LAndExp::accept(Visitor& v) { v.visit(*this); }
void LOrExp::accept(Visitor& v) { v.visit(*this); }

}  // namespace AST