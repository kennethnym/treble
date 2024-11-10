#include "runtime.hxx"
#include "instructions.hxx"
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
			entry.value.operand = instruction.args.literal;

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
