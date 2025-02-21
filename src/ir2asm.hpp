#include <memory>
#include <string>
#include <koopa.h>

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
