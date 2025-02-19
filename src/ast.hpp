#pragma once
#include <iostream>
#include <memory>
#include <string>

class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump(int depth = 0) const = 0;

  virtual void GenerateIR(std::unique_ptr<std::string>& ir, int depth=0) const =0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir, int depth=0) const override;
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir, int depth=0) const override;
};

class FuncTypeAST : public BaseAST {
 public:
  std::string value;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir, int depth=0) const override;
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;

  void Dump(int depth = 0) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir, int depth=0) const override;
};

class StmtAST : public BaseAST {
 public:
  int number;
  void Dump(int depth) const override;

  void GenerateIR(std::unique_ptr<std::string>& ir, int depth=0) const override;
};
