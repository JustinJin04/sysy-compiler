#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::istringstream;
using std::string;
using std::vector;

bool is_terminator(const string& instr) {
  return instr.substr(2, 2) == "br" || instr.substr(2, 4) == "jump" ||
         instr.substr(2, 3) == "ret";
}

void verify_koopa_blocks(const string& ir) {
  vector<string> blocks;
  string current_block;

  istringstream iss(ir);
  string line;
  while (getline(iss, line)) {
    if (line.find('{') != string::npos || line.find('}') != string::npos) {
      continue;
    }
    if (line.find('%') == 0 && line.find(':') != string::npos) {
      if (!current_block.empty()) {
        blocks.push_back(current_block);
        current_block.clear();
      }
    }
    if (!line.empty()) {
      current_block += line + "\n";
    }
  }
  if (!current_block.empty()) {
    blocks.push_back(current_block);
  }

  // 检查每个基本块
  for (const auto& block : blocks) {
    vector<string> instructions;
    istringstream bss(block);
    string bline;
    while (getline(bss, bline)) {
      if (!bline.empty()) {
        instructions.push_back(bline);
      }
    }

    if (instructions.empty()) {
      exit(1);
    }

    // 检查最后一条是否是终止指令
    const string& last = instructions.back();
    // std::cout<<"last: "<<last<<std::endl;
    if (!is_terminator(last)) {
      exit(2);
    }

    // 检查是否有其他终止指令
    for (size_t i = 0; i < instructions.size() - 1; ++i) {
      if (is_terminator(instructions[i])) {
        exit(3);
      }
    }
  }
}
