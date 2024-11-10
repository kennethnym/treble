#ifndef __TREBLE__INSTRUCTIONS_HXX__
#define __TREBLE__INSTRUCTIONS_HXX__

#include <cstddef>
#include <cstdint>

/**
 * Represents a WASM instruction as defined in the specification.
 */
struct Instruction {
	enum class OpCode {
		if_ = 0x04,
		else_ = 0x05,
		i32_const = 0x41,
		i32_add = 0x6A,
		i32_sub = 0x6B,
		i32_mul = 0x6C,
		i32_div_u = 0x6E,
		drop = 0x1A,
		end = 0x0B
	};

	OpCode op_code;

	// associated arguments for an instruction
	union Arguments {
		// i32.const
		uint32_t literal;

		// if
		struct {
			/**
			 * Unused for now
			 */
			void *block_type;

			/**
			 * Stores the offset from the current if instruction to reach the
			 * true branch of the if statement
			 */
			size_t instr_1_offset;

			/**
			 * Stores the offset from the current if instruction to reach the
			 * else branch of the if statement
			 */
			size_t instr_2_offset;
		} if_branch;

		// else
		struct {
			/**
			 * Stores the offset from the current else instruction to reach the
			 * corresponding end marker
			 */
			size_t end_marker_offset;
		} else_branch;
	} args;
};

#endif
