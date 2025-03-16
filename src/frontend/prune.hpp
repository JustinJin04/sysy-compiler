#include "visitor.hpp"
#include "ast.hpp"

namespace AST {

class PruningRetVisitor: public Visitor{
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
    if(constdecl.next_block_item) {
      constdecl.next_block_item->accept(*this);
    }
  }

  void visit(VarDecl& vardecl) override {
    if(vardecl.next_block_item) {
      vardecl.next_block_item->accept(*this);
    }
  }

  void visit(RetStmt& retstmt) override {
    std::cout<<"pruning retstmt"<<std::endl;
    retstmt.next_block_item.reset(nullptr);
    pruning_ret = true;
  }

  void visit(AssignStmt& assignstmt) override {
    if(assignstmt.next_block_item) {
      assignstmt.next_block_item->accept(*this);
    }
  }

  void visit(ExpStmt& expstmt) override {
    if(expstmt.next_block_item) {
      expstmt.next_block_item->accept(*this);
    }
  }

  void visit(BlockStmt& blockstmt) override {
    if(blockstmt.block_item){
      blockstmt.block_item->accept(*this);
    }
    if(pruning_ret) {
      blockstmt.next_block_item.reset(nullptr);
    } else {
      if(blockstmt.next_block_item) {
        blockstmt.next_block_item->accept(*this);
      }
    }
  }

  void visit(IfStmt& ifstmt) override {
    // std::cout<<"pruning ifstmt"<<std::endl;
    assert(pruning_ret == false);
    if(ifstmt.then_body){
      std::cout<<"pruning then"<<std::endl;
      ifstmt.then_body->accept(*this);
    }
    // assert(pruning_end == false);
    pruning_ret = false;
    if(ifstmt.else_body){
      std::cout<<"pruning else"<<std::endl;
      ifstmt.else_body->accept(*this);
    }
    // assert(pruning_end == false);
    pruning_ret = false;
    if(ifstmt.next_block_item) {
      ifstmt.next_block_item->accept(*this);
    }
  }

  void visit(WhileStmt& whilestmt) override {
    if(whilestmt.body) {
      whilestmt.body->accept(*this);
    }
    if(pruning_ret) {
      whilestmt.next_block_item.reset(nullptr);
    } else {
      if(whilestmt.next_block_item) {
        whilestmt.next_block_item->accept(*this);
      }
    }
    
    // pruning_end = false;
    // if(whilestmt.next_block_item) {
    //   whilestmt.next_block_item->accept(*this);
    // }
  }

  void visit(BreakStmt& breakstmt) override {
    breakstmt.next_block_item.reset(nullptr);
  }

  void visit(ContinueStmt& continuestmt) override {
    continuestmt.next_block_item.reset(nullptr);
  };

};





};  // namespace AST