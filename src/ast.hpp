#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <stack>


class IRContext {
  private:
  // IRContextKind kind = IRContextKind::TEMP_COUNTER;
  // 上一步解析后保存的Counter标号
  int tempCounter = 0;
  std::stack<std::string> st;
  // 上一步解析的立即数
  // int int_const = 0;
 public:
  int depth = 0;

  std::string get_new_counter() {
    int retval = tempCounter;
    tempCounter += 1;
    return "%" + std::to_string(retval);
  }

  void push_result(std::string result) {
    st.push(result);
  }
  void push_result(int int_const) {
    st.push(std::to_string(int_const));
  }

  // std::string get_last_result(int i=1);
  std::string pop_last_result() {
    std::string ret = st.top();
    st.pop();
    return ret;
  }

};

class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;
class ExpAST;


class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump(int depth = 0) const = 0;

  virtual void GenerateIR(std::unique_ptr<std::string>& ir,
                          IRContext& ctx) const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<FuncDefAST> func_def;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<FuncTypeAST> func_type;
  std::string IDENT;
  std::unique_ptr<BlockAST> block;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class FuncTypeAST : public BaseAST {
 public:
  std::string TYPE_NAME;
  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<StmtAST> stmt;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<ExpAST> exp;
  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

enum class ExpKind  {
  NUMBER,
  
  // used for unary op
  POSITIVE,
  NEGATIVE,
  LOGICAL_NOT,
  
  // used for binary op
  ADD,
  SUB,
  MUL,
  DIV,
  REM,

  // used for compare op
  LT,
  GT,
  LEQ,
  GEQ,
  EQ,
  NEQ,

  // used for logical op
  LAND,
  LOR
};

class ExpAST : public BaseAST {
 public:
  ExpKind kind;

  int number;                            // used for ExpKind::NUMBER 
  std::unique_ptr<ExpAST> lhs;           // only used for binary op
  std::unique_ptr<ExpAST> rhs;           // used both in unary and binary op

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

