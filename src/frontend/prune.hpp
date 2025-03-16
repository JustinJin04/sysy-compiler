#include "visitor.hpp"
#include "ast.hpp"

namespace AST {

class PruningRetVisitor: public Visitor{
 public:
  bool pruning_end = false;

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
    retstmt.next_block_item.reset(nullptr);
    pruning_end = true;
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
    if(pruning_end) {
      blockstmt.next_block_item.reset(nullptr);
    } else {
      if(blockstmt.next_block_item) {
        blockstmt.next_block_item->accept(*this);
      }
    }
  }



};





};  // namespace AST