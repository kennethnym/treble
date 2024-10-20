#include <cstdint>

struct Instruction {
	enum class OpCode { i32_const = 0x41, drop = 0x1a, end = 0x0b };
	OpCode op_code;

	union Arguments {
		uint32_t literal;
	} args;
};