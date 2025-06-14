#pragma once

#include <string>
#include <stack>

namespace AST {

class WhileStack {
 public:
  struct WhileLabel {
    std::string while_entry;
    std::string while_end;
  };
  std::stack<WhileLabel> while_label_stack;

  void push_while_label(std::string while_entry, std::string while_end) {
    WhileLabel while_label;
    while_label.while_entry = while_entry;
    while_label.while_end = while_end;
    while_label_stack.push(while_label);
  }

  void pop_while_label() { while_label_stack.pop(); }

  std::string get_top_while_entry() {
    return while_label_stack.top().while_entry;
  }

  std::string get_top_while_end() { return while_label_stack.top().while_end; }
};

};  // namespace AST