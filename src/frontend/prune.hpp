#include "visitor.hpp"
#include "ast.hpp"

namespace AST {

class PruningRetVisitor : public Visitor {
 public:
  // bool pruning_end = false;
  bool pruning_ret = false;
  // bool pruning_break = false;
  // bool pruning_continue = false;

  /**
   * pruning the AST to remove all stmt that is
   * behind a return statement
   */

  void visit(ConstDecl& constdecl) override {
    if (constdecl.next_block_item) {
      constdecl.next_block_item->accept(*this);
    }
  }

  void visit(VarDecl& vardecl) override {
    if (vardecl.next_block_item) {
      vardecl.next_block_item->accept(*this);
    }
  }

  void visit(RetStmt& retstmt) override {
    std::cout << "pruning retstmt" << std::endl;
    assert(pruning_ret == false);
    retstmt.next_block_item.reset(nullptr);
    pruning_ret = true;
  }

  void visit(AssignStmt& assignstmt) override {
    assert(pruning_ret == false);
    if (assignstmt.next_block_item) {
      assignstmt.next_block_item->accept(*this);
    }
  }

  void visit(ExpStmt& expstmt) override {
    assert(pruning_ret == false);
    if (expstmt.next_block_item) {
      expstmt.next_block_item->accept(*this);
    }
  }

  void visit(BlockStmt& blockstmt) override {
    assert(pruning_ret == false);
    if (blockstmt.block_item) {
      blockstmt.block_item->accept(*this);
    }
    if (pruning_ret) {
      blockstmt.next_block_item.reset(nullptr);
    } else {
      if (blockstmt.next_block_item) {
        blockstmt.next_block_item->accept(*this);
      }
    }
  }

  void visit(IfStmt& ifstmt) override {
    // std::cout<<"pruning ifstmt"<<std::endl;
    assert(pruning_ret == false);
    if (ifstmt.then_body) {
      std::cout << "pruning then" << std::endl;
      ifstmt.then_body->accept(*this);
    }
    pruning_ret = false;
    if (ifstmt.else_body) {
      std::cout << "pruning else" << std::endl;
      ifstmt.else_body->accept(*this);
    }
    pruning_ret = false;
    if (ifstmt.next_block_item) {
      ifstmt.next_block_item->accept(*this);
    }
  }

  void visit(WhileStmt& whilestmt) override {
    assert(pruning_ret == false);
    if (whilestmt.body) {
      whilestmt.body->accept(*this);
    }
    // since we don't know if the while loop will be executed
    // we can't prune the next block item
    pruning_ret = false;
    if (whilestmt.next_block_item) {
      whilestmt.next_block_item->accept(*this);
    }
  }

  void visit(BreakStmt& breakstmt) override {
    assert(pruning_ret == false);
    breakstmt.next_block_item.reset(nullptr);
    // Here we also need to set pruning_ret to true
    // For example:
    // while(1) {
    //  {
    //    break;
    //  }
    //  return;
    // }
    pruning_ret = true;
  }

  void visit(ContinueStmt& continuestmt) override {
    assert(pruning_ret == false);
    continuestmt.next_block_item.reset(nullptr);
    pruning_ret = true;
  };
};

};  // namespace AST