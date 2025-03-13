#pragma once

#include <memory>
#include <string>
#include <koopa.h>
#include <map>
#include <optional>
#include <cassert>
#include <vector>
#include <set>

class TempRegPool {
 public:
  void reset(int size=7) {
    pool.clear();
    for (int i = 0; i < size; i++) {
      pool.push_back("t" + std::to_string(i));
    }
  }

  bool is_empty() {
    return pool.empty();
  }

  std::string alloc() {
    assert(!pool.empty());
    auto ret = pool.back();
    pool.pop_back();
    return ret;
  }

  void free(std::string reg) {
    pool.push_back(reg);
  }

 private:
  std::vector<std::string> pool;
};

class BasicBlockContext {
 public:
  void reset(koopa_raw_basic_block_t block) {
    current_basic_block = block;
    current_koopa_raw_value = NULL;
    inst_to_temp_reg.clear();
    temp_reg_pool.reset();
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
  std::string alloc_new_temp_reg() {
    if(!temp_reg_pool.is_empty()){
      return temp_reg_pool.alloc();
    }
    // recycle temp reg not used in the future
    std::set<int> will_used_reg_idx;
    bool start_to_add = false;
    for(int i=0;i<current_basic_block->insts.len;++i){
      auto ptr = current_basic_block->insts.buffer[i];
      koopa_raw_value_t value = reinterpret_cast<koopa_raw_value_t>(ptr);
      if(start_to_add == false && value != current_koopa_raw_value){
        continue;
      }
      if(start_to_add == false){
        start_to_add = true;
        continue;
      }
      add_will_used_reg_idx_set(will_used_reg_idx, value);
    }
    for(int i=0;i<7;++i){
      if(will_used_reg_idx.find(i) == will_used_reg_idx.end()){
        temp_reg_pool.free("t" + std::to_string(i));
      }
    }
    if(temp_reg_pool.is_empty()){
      throw std::runtime_error("all temp reg will be used in the future");
    }
    return temp_reg_pool.alloc();
  }

  // add instruction's operand's corresponding temp reg to will_used_reg_idx
  void add_will_used_reg_idx_set(std::set<int>& will_used_reg_idx, koopa_raw_value_t value) {
    const auto &kind = value->kind;
    if(kind.tag != KOOPA_RVT_BINARY) {
      // throw std::runtime_error("not binary instruction but also trigered add_will_used_reg_idx_set");
      return;
    }
    auto lhs = kind.data.binary.lhs;
    auto lhs_name = value_to_name(lhs);
    if(lhs_name.has_value()){
      int reg_number = std::stoi(lhs_name.value().substr(1));
      will_used_reg_idx.insert(reg_number);
    }
    auto rhs = kind.data.binary.rhs;
    auto rhs_name = value_to_name(rhs);
    if(rhs_name.has_value()){
      int reg_number = std::stoi(rhs_name.value().substr(1));
      will_used_reg_idx.insert(reg_number);
    }
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
  // int next_temp_reg_idx;
  // const int max_temp_reg_idx = 6;
  koopa_raw_basic_block_t current_basic_block;
  koopa_raw_value_t current_koopa_raw_value;
  TempRegPool temp_reg_pool;
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



