#pragma once

#include <memory>
#include <string>
#include <koopa.h>
#include <map>
#include <optional>
#include <cassert>

class BasicBlockContext {
 public:
  void reset() {
    next_temp_reg_idx = 0;
    current_koopa_raw_value = NULL;
    inst_to_temp_reg.clear();
  }
  void push_current_koopa_raw_value(koopa_raw_value_t value) {
    current_koopa_raw_value = value;
  }

  std::optional<std::string> value_to_name(const koopa_raw_value_t& value) {
    auto it = inst_to_temp_reg.find(value);
    if (it != inst_to_temp_reg.end()) {
      return it->second;
    } else {
      return {};
    }
  }

  // just propose, not bind
  std::string alloc_new_temp_reg(){
    // if(next_temp_reg_idx > max_temp_reg_idx) {
    //   assert(0);
    // }
    assert(next_temp_reg_idx <= max_temp_reg_idx);
    auto ret = "t" + std::to_string(next_temp_reg_idx);
    next_temp_reg_idx += 1;
    return ret;
  }

  // if null, bind current_koopa_raw_value to reg
  void bind_value_temp_reg(std::string reg, koopa_raw_value_t value = NULL) {
    // assert(reg[0] == 't');
    // int reg_number = std::stoi(reg.substr(1));
    // assert(reg_number <= max_temp_reg_idx);
    if (value) {
      inst_to_temp_reg[value] = reg;
    } else {
      inst_to_temp_reg[current_koopa_raw_value] = reg;
    }
  }

  /**
   * if operand is already on register, just return
   * if operand is a number(non zero), load into a temp reg
   * and add to GLOBAL_BASIC_BLOCK_CTX
   */
  void prepare_operand(const koopa_raw_value_t& operand, std::unique_ptr<std::string>& asm_code);

  /**
   * if operand is already on register, mv if it wasn't reg_name
   * else if operand is number
   */
  void prepare_operand(const koopa_raw_value_t& operand, std::unique_ptr<std::string>& asm_code, std::string reg_name);

 private:
  int next_temp_reg_idx;
  const int max_temp_reg_idx = 6;
  koopa_raw_value_t current_koopa_raw_value;
  std::map<koopa_raw_value_t, std::string> inst_to_temp_reg;
};


void IR_to_ASM(std::unique_ptr<std::string>& ir, std::unique_ptr<std::string>& asm_code);

/**
 * visit raw program
 */
void visit(const koopa_raw_program_t& raw, std::unique_ptr<std::string>& asm_code);

/**
 * visit different kinds of koopa values
 */
void visit(const koopa_raw_value_t& value, std::unique_ptr<std::string>& asm_code);

/**
 * visit global funcs
 */
void visit(const koopa_raw_function_t& func, std::unique_ptr<std::string>& asm_code);

/**
 * visit basic blocks
 */
void visit(const koopa_raw_basic_block_t& block, std::unique_ptr<std::string>& asm_code);


/**
 * visit return instruction
 */
void visit(const koopa_raw_return_t& ret, std::unique_ptr<std::string>& asm_code);

/**
 * visit integer instruction
 */
void visit(const koopa_raw_integer_t& integer, std::unique_ptr<std::string>& asm_code);


/**
 * visit binary instruction
 */
void visit(const koopa_raw_binary_t& binary, std::unique_ptr<std::string>& asm_code);



