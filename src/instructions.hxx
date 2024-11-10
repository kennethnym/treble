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

		drop = 0x1A,
		end = 0x0B,

		i32_const = 0x41,
		i64_const = 0x42,

		i32_eqz = 0x45,
		i32_eq = 0x46,
		i32_ne = 0x47,
		i32_lt_s = 0x48,
		i32_lt_u = 0x49,
		i32_gt_s = 0x4A,
		i32_gt_u = 0x4B,
		i32_le_s = 0x4C,
		i32_le_u = 0x4D,
		i32_ge_s = 0x4E,
		i32_ge_u = 0x4F,

		i32_add = 0x6A,
		i32_sub = 0x6B,
		i32_mul = 0x6C,
		i32_div_s = 0x6D,
		i32_div_u = 0x6E,
		i32_rem_s = 0x6F,
		i32_rem_u = 0x70,

		i32_and = 0x71,
		i32_or = 0x72,
		i32_xor = 0x73,
		i32_shl = 0x74,
		i32_shr_s = 0x75,
		i32_shr_u = 0x76,
		i32_rotl = 0x77,
		i32_rotr = 0x78,

		i32_clz = 0x79,
		i32_ctz = 0x7A,
		i32_popcnt = 0x7B,

		i32_wrap_i64 = 0xA7,
	};

	OpCode op_code;

	// associated arguments for an instruction
	union Arguments {
		// i32.const
		uint32_t i32;

		// i64.const
		uint64_t i64;

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
