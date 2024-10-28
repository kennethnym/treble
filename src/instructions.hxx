#ifndef __TREBLE__INSTRUCTIONS_HXX__
#define __TREBLE__INSTRUCTIONS_HXX__

#include <cstdint>

struct Instruction {
	enum class OpCode {
		i32_const = 0x41,
		i32_add = 0x6a,
		drop = 0x1a,
		end = 0x0b
	};
	OpCode op_code;

	union Arguments {
		uint32_t literal;
	} args;
};

#endif
