#ifndef __TREBLE__INSTRUCTIONS_HXX__
#define __TREBLE__INSTRUCTIONS_HXX__

#include <cstddef>
#include <cstdint>

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

	union Arguments {
		uint32_t literal;

		struct {
			void *block_type;
			size_t instr_1_offset;
			size_t instr_2_offset;
		} if_branch;

		struct {
			size_t end_marker_offset;
		} else_branch;
	} args;
};

#endif
