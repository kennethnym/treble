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
		f32_const = 0x43,

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

		i64_eqz = 0x50,
		i64_eq = 0x51,
		i64_ne = 0x52,
		i64_lt_s = 0x53,
		i64_lt_u = 0x54,
		i64_gt_s = 0x55,
		i64_gt_u = 0x56,
		i64_le_s = 0x57,
		i64_le_u = 0x58,
		i64_ge_s = 0x59,
		i64_ge_u = 0x5A,

		i32_clz = 0x67,
		i32_ctz = 0x68,
		i32_popcnt = 0x69,
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

		i64_clz = 0x79,
		i64_ctz = 0x7A,
		i64_popcnt = 0x7B,
		i64_add = 0x7C,
		i64_sub = 0x7D,
		i64_mul = 0x7E,
		i64_div_s = 0x7F,
		i64_div_u = 0x80,
		i64_rem_s = 0x81,
		i64_rem_u = 0x82,
		i64_and = 0x83,
		i64_or = 0x84,
		i64_xor = 0x85,
		i64_shl = 0x86,
		i64_shr_s = 0x87,
		i64_shr_u = 0x88,
		i64_rotl = 0x89,
		i64_rotr = 0x8A,

		i32_wrap_i64 = 0xA7,
	};

	OpCode op_code;

	// associated arguments for an instruction
	union Arguments {
		// i32.const
		uint32_t i32;

		// i64.const
		uint64_t i64;

		// f32.const
		float f32;

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
