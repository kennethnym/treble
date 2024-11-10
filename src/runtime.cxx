#include "runtime.hxx"
#include "instructions.hxx"
#include <bit>
#include <cstdint>
#include <iostream>

struct StackEntry {
	enum class Type { Value, Label, Activations };
	Type type;
	union {
		uint32_t operand;
	} value;
};

void print_stack(StackEntry *stack, int64_t ptr) {
	if (ptr < 0) {
		std::cout << "stack is empty!" << std::endl;
		return;
	}

	const StackEntry &entry = stack[ptr];
	switch (entry.type) {
	case StackEntry::Type::Value:
		std::cout << "top of stack:" << std::endl;
		std::cout << "    type: Value" << std::endl;
		std::cout << "    operand: " << entry.value.operand << std::endl;
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
		case Instruction::OpCode::i32_const: {
			std::cout << "i32.const" << std::endl;
			StackEntry &entry = stack[++stack_ptr];
			entry.type = StackEntry::Type::Value;
			entry.value.operand = instruction.args.i32;

			header++;
			break;
		}

		case Instruction::OpCode::i32_eqz: {
			// NOTE: popping and pushing should be equivalent to
			// writing the result back to itself
			// this should work just fine but making a note as a reminder

			std::cout << "i32.eqz" << std::endl;

			StackEntry &entry_c1 = stack[stack_ptr];
			entry_c1.value.operand = entry_c1.value.operand == 0 ? 1 : 0;
			header++;
			break;
		};

		case Instruction::OpCode::i32_eq: {
			std::cout << "i32.eq" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				entry_c1.value.operand == entry_c2.value.operand ? 1 : 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_ne: {
			std::cout << "i32.ne" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				entry_c1.value.operand == entry_c2.value.operand ? 0 : 1;

			header++;
			break;
		}

		case Instruction::OpCode::i32_lt_u: {
			std::cout << "i32.lt_u" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				entry_c1.value.operand < entry_c2.value.operand ? 1 : 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_lt_s: {
			std::cout << "i32.lt_u" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				static_cast<int32_t>(entry_c1.value.operand) <
						static_cast<int32_t>(entry_c2.value.operand)
					? 1
					: 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_gt_u: {
			std::cout << "i32.gt_u" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				entry_c1.value.operand > entry_c2.value.operand ? 1 : 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_gt_s: {
			std::cout << "i32.gt_u" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				static_cast<int32_t>(entry_c1.value.operand) >
						static_cast<int32_t>(entry_c2.value.operand)
					? 1
					: 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_le_u: {
			std::cout << "i32.le_u" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				entry_c1.value.operand <= entry_c2.value.operand ? 1 : 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_le_s: {
			std::cout << "i32.le_s" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				static_cast<int32_t>(entry_c1.value.operand) <=
						static_cast<int32_t>(entry_c2.value.operand)
					? 1
					: 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_ge_u: {
			std::cout << "i32.ge_u" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				entry_c1.value.operand >= entry_c2.value.operand ? 1 : 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_ge_s: {
			std::cout << "i32.le_s" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand =
				static_cast<int32_t>(entry_c1.value.operand) >=
						static_cast<int32_t>(entry_c2.value.operand)
					? 1
					: 0;

			header++;
			break;
		}

		case Instruction::OpCode::i32_add: {
			std::cout << "i32.add" << std::endl;
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const auto result = entry_c1.value.operand + entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_sub: {
			std::cout << "i32.sub" << std::endl;
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const auto result = entry_c1.value.operand - entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_mul: {
			std::cout << "i32.mul" << std::endl;
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const auto result = entry_c1.value.operand * entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_div_u: {
			std::cout << "i32.div_u" << std::endl;
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const uint32_t result =
				entry_c1.value.operand / entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_div_s: {
			std::cout << "i32.div_s" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const int32_t result =
				static_cast<int32_t>(entry_c1.value.operand) /
				static_cast<int32_t>(entry_c2.value.operand);

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_rem_u: {
			std::cout << "i32.rem_u" << std::endl;
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const uint32_t result =
				entry_c1.value.operand % entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_rem_s: {
			std::cout << "i32.div_s" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const int32_t result =
				static_cast<int32_t>(entry_c1.value.operand) %
				static_cast<int32_t>(entry_c2.value.operand);

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_and: {
			std::cout << "i32_and" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const auto result = entry_c1.value.operand & entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_or: {
			std::cout << "i32_or" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const auto result = entry_c1.value.operand | entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_xor: {
			std::cout << "i32_or" << std::endl;

			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];
			const auto result = entry_c1.value.operand ^ entry_c2.value.operand;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_shl: {
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			const auto k = entry_c2.value.operand % 32;
			const auto result = (entry_c1.value.operand << k) % 32;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_shr_s:
		case Instruction::OpCode::i32_shr_u: {
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			const auto k = entry_c2.value.operand % 32;
			const auto result = entry_c1.value.operand >> k;

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_rotl: {
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			const auto k = entry_c2.value.operand % 32;
			const auto result = (entry_c1.value.operand << k) |
								(entry_c1.value.operand >> (32 - k));

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_rotr: {
			StackEntry &entry_c2 = stack[stack_ptr--];
			StackEntry &entry_c1 = stack[stack_ptr--];

			const auto k = entry_c2.value.operand % 32;
			const auto result = (entry_c1.value.operand << (32 - k)) |
								(entry_c1.value.operand >> (k));

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;

			header++;
			break;
		}

		case Instruction::OpCode::i32_clz: {
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = std::countr_zero(entry_c1.value.operand);

			header++;
			break;
		}

		case Instruction::OpCode::i32_ctz: {
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = std::countl_zero(entry_c1.value.operand);

			header++;
			break;
		}

		case Instruction::OpCode::i32_popcnt: {
			StackEntry &entry_c1 = stack[stack_ptr--];

			StackEntry &entry_c = stack[++stack_ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = std::popcount(entry_c1.value.operand);

			header++;
			break;
		}

		case Instruction::OpCode::i32_wrap_i64: {
			StackEntry &entry_c = stack[stack_ptr];
			entry_c.value.operand = entry_c.value.operand % 4294967296;
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
			if (c.value.operand) {
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
