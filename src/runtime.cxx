#include "runtime.hxx"
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

	const Function &start_func =
		instance.module->funcs[instance.module->start->func_index];

	StackEntry stack[65536];
	int64_t ptr = -1;

	size_t header = 0;
	while (true) {
		Instruction &instruction = start_func.body[header];

		switch (instruction.op_code) {
		case Instruction::OpCode::i32_const: {
			std::cout << "i32.const" << std::endl;
			StackEntry &entry = stack[++ptr];
			entry.type = StackEntry::Type::Value;
			entry.value.operand = instruction.args.literal;
			break;
		}

		case Instruction::OpCode::i32_add: {
			std::cout << "i32.add" << std::endl;
			StackEntry &entry_c2 = stack[ptr--];
			StackEntry &entry_c1 = stack[ptr--];
			const auto result = entry_c1.value.operand + entry_c2.value.operand;

			StackEntry &entry_c = stack[++ptr];
			entry_c.type = StackEntry::Type::Value;
			entry_c.value.operand = result;
			break;
		}

		case Instruction::OpCode::drop: {
			std::cout << "i32.drop" << std::endl;
			ptr--;
			break;
		}

		case Instruction::OpCode::end:
			std::cout << "end" << std::endl;
			return;

		default:
			std::cout << "unknown op code: "
					  << +static_cast<uint8_t>(instruction.op_code)
					  << std::endl;
		}

		print_stack(stack, ptr);

		header++;
	}
}
