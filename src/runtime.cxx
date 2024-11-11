#include "runtime.hxx"
#include "instructions.hxx"
#include <bit>
#include <cstdint>
#include <iostream>

#define BINARY_OPERATION(dtype, instr_name, stack_type, operator)              \
	case Instruction::OpCode::instr_name: {                                    \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
		const auto result = entry_c1.value.dtype##_operand operator entry_c2   \
								.value.dtype##_operand;                        \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand = result;                                \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}

#define SIGNED_BINARY_OPERATION(dtype, instr_name, signed_type,                \
								stack_type, operator)                          \
	case Instruction::OpCode::instr_name: {                                    \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
		const auto result =                                                    \
			static_cast<signed_type>(entry_c1.value.dtype##_operand)           \
			operator static_cast<signed_type>(entry_c2.value.dtype##_operand); \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand = result;                                \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}

#define INTEGER_INSTRUCTIONS(dtype, bit_width, signed_type, stack_type)        \
	case Instruction::OpCode::dtype##_const: {                                 \
		std::cout << #dtype ".const" << std::endl;                             \
		StackEntry &entry = stack[++stack_ptr];                                \
		entry.type = StackEntry::Type::stack_type;                             \
		entry.value.dtype##_operand = instruction.args.dtype;                  \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_eqz: {                                   \
		std::cout << #dtype ".eqz" << std::endl;                               \
		StackEntry &entry_c1 = stack[stack_ptr];                               \
		entry_c1.type = StackEntry::Type::I32Value;                            \
		entry_c1.value.i32_operand =                                           \
			entry_c1.value.dtype##_operand == 0 ? 1 : 0;                       \
		header++;                                                              \
		break;                                                                 \
	};                                                                         \
	case Instruction::OpCode::dtype##_eq: {                                    \
		std::cout << #dtype ".eq" << std::endl;                                \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			entry_c1.value.dtype##_operand == entry_c2.value.dtype##_operand   \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_ne: {                                    \
		std::cout << #dtype ".ne" << std::endl;                                \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			entry_c1.value.dtype##_operand == entry_c2.value.dtype##_operand   \
				? 0                                                            \
				: 1;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_lt_u: {                                  \
		std::cout << #dtype ".lt_u" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			entry_c1.value.dtype##_operand < entry_c2.value.dtype##_operand    \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_lt_s: {                                  \
		std::cout << #dtype ".lt_u" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			static_cast<int32_t>(entry_c1.value.dtype##_operand) <             \
					static_cast<int32_t>(entry_c2.value.dtype##_operand)       \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_gt_u: {                                  \
		std::cout << #dtype ".gt_u" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			entry_c1.value.dtype##_operand > entry_c2.value.dtype##_operand    \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_gt_s: {                                  \
		std::cout << #dtype ".gt_u" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			static_cast<signed_type>(entry_c1.value.dtype##_operand) >         \
					static_cast<signed_type>(entry_c2.value.dtype##_operand)   \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_le_u: {                                  \
		std::cout << #dtype ".le_u" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			entry_c1.value.dtype##_operand <= entry_c2.value.dtype##_operand   \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_ge_u: {                                  \
		std::cout << #dtype ".ge_u" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			entry_c1.value.dtype##_operand >= entry_c2.value.dtype##_operand   \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_ge_s: {                                  \
		std::cout << #dtype ".le_s" << std::endl;                              \
                                                                               \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.i32_operand =                                            \
			static_cast<signed_type>(entry_c1.value.dtype##_operand) >=        \
					static_cast<signed_type>(entry_c2.value.dtype##_operand)   \
				? 1                                                            \
				: 0;                                                           \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
                                                                               \
		BINARY_OPERATION(dtype, dtype##_add, stack_type, +)                    \
		BINARY_OPERATION(dtype, dtype##_sub, stack_type, -)                    \
		BINARY_OPERATION(dtype, dtype##_mul, stack_type, *)                    \
                                                                               \
		BINARY_OPERATION(dtype, dtype##_div_u, stack_type, /)                  \
		SIGNED_BINARY_OPERATION(dtype, dtype##_div_s, signed_type, stack_type, \
								/)                                             \
                                                                               \
		BINARY_OPERATION(dtype, dtype##_rem_u, stack_type, %)                  \
		SIGNED_BINARY_OPERATION(dtype, dtype##_rem_s, int32_t, stack_type, %)  \
                                                                               \
		BINARY_OPERATION(dtype, dtype##_and, stack_type, &);                   \
		BINARY_OPERATION(dtype, dtype##_or, stack_type, |);                    \
		BINARY_OPERATION(dtype, dtype##_xor, stack_type, ^);                   \
                                                                               \
	case Instruction::OpCode::dtype##_shl: {                                   \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		const auto k = entry_c2.value.dtype##_operand % bit_width;             \
		const auto result = (entry_c1.value.dtype##_operand << k) % bit_width; \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand = result;                                \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_shr_s:                                   \
	case Instruction::OpCode::dtype##_shr_u: {                                 \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		const auto k = entry_c2.value.dtype##_operand % 32;                    \
		const auto result = entry_c1.value.dtype##_operand >> k;               \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand = result;                                \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_rotl: {                                  \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		const auto k = entry_c2.value.dtype##_operand % bit_width;             \
		const auto result =                                                    \
			(entry_c1.value.dtype##_operand << k) |                            \
			(entry_c1.value.dtype##_operand >> (bit_width - k));               \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand = result;                                \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_rotr: {                                  \
		StackEntry &entry_c2 = stack[stack_ptr--];                             \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		const auto k = entry_c2.value.dtype##_operand % bit_width;             \
		const auto result =                                                    \
			(entry_c1.value.dtype##_operand << (bit_width - k)) |              \
			(entry_c1.value.dtype##_operand >> (k));                           \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand = result;                                \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_clz: {                                   \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.dtype##_operand =                                        \
			std::countr_zero(entry_c1.value.dtype##_operand);                  \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_ctz: {                                   \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::I32Value;                             \
		entry_c.value.dtype##_operand =                                        \
			std::countl_zero(entry_c1.value.dtype##_operand);                  \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}                                                                          \
	case Instruction::OpCode::dtype##_popcnt: {                                \
		StackEntry &entry_c1 = stack[stack_ptr--];                             \
                                                                               \
		StackEntry &entry_c = stack[++stack_ptr];                              \
		entry_c.type = StackEntry::Type::stack_type;                           \
		entry_c.value.dtype##_operand =                                        \
			std::popcount(entry_c1.value.dtype##_operand);                     \
                                                                               \
		header++;                                                              \
		break;                                                                 \
	}

struct StackEntry {
	enum class Type { I32Value, I64Value, F32Value, Label, Activations };
	Type type;
	union {
		uint32_t i32_operand;
		uint64_t i64_operand;
		float f32_operand;
	} value;
};

void print_stack(StackEntry *stack, int64_t ptr) {
	if (ptr < 0) {
		std::cout << "stack is empty!" << std::endl;
		return;
	}

	const StackEntry &entry = stack[ptr];
	switch (entry.type) {
	case StackEntry::Type::I32Value:
		std::cout << "top of stack:" << std::endl;
		std::cout << "    type: Value" << std::endl;
		std::cout << "    operand: " << entry.value.i32_operand << std::endl;
		break;
	default:
		break;
	}
}

void Treble::execute_module_instance(ModuleInstance &instance) {
	if (instance.module->start == nullptr) {
		return;
	}

	// the main function for the wasm module
	const Function &start_func =
		instance.module->funcs[instance.module->start->func_index];

	// the program stack
	StackEntry stack[65536];

	// points to the top-most entry in the current execution stack.
	int64_t stack_ptr = -1;
	// points to the current instruction being executed.
	size_t header = 0;
	// keep track of block nesting levels
	uint block_level = 0;
	// whether the execution is finished.
	bool is_finished = false;

	while (!is_finished) {
		Instruction &instruction = start_func.body[header];

		switch (instruction.op_code) {
			INTEGER_INSTRUCTIONS(i32, 32, int32_t, I32Value);
			INTEGER_INSTRUCTIONS(i64, 64, int64_t, I64Value);

		case Instruction::OpCode::f32_const: {
			std::cout << "f32.const" << std::endl;
			StackEntry &entry = stack[++stack_ptr];
			entry.type = StackEntry::Type::F32Value;
			entry.value.f32_operand = instruction.args.f32;

			header++;
			break;
		}

		case Instruction::OpCode::i32_wrap_i64: {
			StackEntry &entry_c = stack[stack_ptr];
			entry_c.type = StackEntry::Type::I32Value;
			entry_c.value.i32_operand = entry_c.value.i64_operand % 4294967296;
			header++;
			break;
		}

		case Instruction::OpCode::drop: {
			std::cout << "i32.drop" << std::endl;
			stack_ptr--;
			header++;
			break;
		}

		case Instruction::OpCode::if_: {
			std::cout << "if" << std::endl;
			StackEntry &c = stack[stack_ptr--];
			if (c.value.i32_operand) {
				header += instruction.args.if_branch.instr_1_offset;
			} else {
				header += instruction.args.if_branch.instr_2_offset;
			}
			block_level++;
			break;
		}

		case Instruction::OpCode::else_: {
			header += instruction.args.else_branch.end_marker_offset;
			break;
		}

		case Instruction::OpCode::end:
			// end marker encountered
			// if inside a block, exit the block
			// otherwise (when block_level is 0) we are done
			if (block_level == 0) {
				is_finished = true;
			} else {
				block_level--;
			}

			header++;
			break;

		default:
			std::cout << "unknown op code: "
					  << +static_cast<uint8_t>(instruction.op_code)
					  << std::endl;

			header++;
			break;
		}

		print_stack(stack, stack_ptr);
	}
}
