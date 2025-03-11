#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <cassert>


enum class IRContextKind {
  INT_CONST,
  TEMP_COUNTER
};

class IRContext {
 public:
  int depth = 0;

  std::string get_last_result() const {
    if (kind == IRContextKind::INT_CONST) {
      return std::to_string(int_const);
    } else if (kind == IRContextKind::TEMP_COUNTER) {
      return "%" + std::to_string(tempCounter);
    } else {
      assert(0);
    }
  }
  std::string get_new_counter() const {
    return "%"+std::to_string(tempCounter+1);
  }

  void set_number(int number) {
    kind = IRContextKind::INT_CONST;
    int_const = number;
  }
  void set_counter(std::string counter) {
    kind = IRContextKind::TEMP_COUNTER;
    assert(counter[0] == '%');
    tempCounter = std::stoi(counter.substr(1));
  }
  void set_counter(int counter_index) {
    kind = IRContextKind::TEMP_COUNTER;
    tempCounter = counter_index;
  }

 private:
  IRContextKind kind = IRContextKind::TEMP_COUNTER;
  // 上一步解析后保存的Counter标号
  int tempCounter = -1;
  // 上一步解析的立即数
  int int_const = 0;
};

class BaseAST;

class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;
class NumberAST;

class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump(int depth = 0) const = 0;

  virtual void GenerateIR(std::unique_ptr<std::string>& ir,
                          IRContext& ctx) const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string IDENT;
  std::unique_ptr<BaseAST> block;

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
  std::unique_ptr<BaseAST> stmt;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

class NumberAST : public BaseAST {
 public:
  int INT_CONST;
  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};


enum class ExpASTKind {
  UnaryExp,
  AddExp
};


class ExpAST : public BaseAST {
 public:
  ExpASTKind kind
  std::unique_ptr<BaseAST> unaryexp;
  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

enum class UnaryExpKind { Primary, UnaryOp };

class UnaryExpAST : public BaseAST {
 public:
  UnaryExpKind kind;

  std::unique_ptr<BaseAST> primaryexp;

  std::unique_ptr<BaseAST> unaryop;
  std::unique_ptr<BaseAST> unaryexp;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

enum class PrimaryExpKind { Exp, Number };

class PrimaryExpAST : public BaseAST {
 public:
  PrimaryExpKind kind;

  std::unique_ptr<BaseAST> exp;

  std::unique_ptr<BaseAST> number;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};

enum class UnaryOpKind { Add, Sub, LogicalNot };

class UnaryOpAST : public BaseAST {
 public:
  UnaryOpKind kind;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir,
                  IRContext& ctx) const override;
};