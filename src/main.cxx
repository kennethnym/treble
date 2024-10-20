#include "instructions.hxx"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <sys/types.h>
#include <type_traits>
#include <vector>

#define END_MARKER 0x0b

typedef uint32_t Address;

enum class SectionType {
	Custom = 0,
	Type = 1,
	Function = 3,
	Start = 8,
	Code = 10
};

enum class ValueType {
	i32,
	i64,
	f32,
	f64,
};

enum class TypeId : uint8_t {
	Function = 0x60,
};

struct ModuleInstance;

struct Function {
	uint32_t type_index;
	Instruction *body;
};

struct FunctionType {
	ValueType *param_types;
	size_t param_count;
	ValueType *result_types;
	size_t result_count;
};

struct FunctionInstance {
	FunctionType type;
	ModuleInstance *module;
	Function code;
};

struct ModuleStore {
	FunctionInstance *funcs;
};

struct Start {
	uint32_t func_index;
};

struct Module {
	FunctionType *types;
	size_t type_count;
	Function *funcs;
	size_t func_count;
	Start *start;
};

struct ModuleInstance {
	const Module *module;
	ModuleStore store;
	FunctionType *types;
	size_t type_count;
	Address *funcaddrs;
	size_t funcaddr_count;
};

struct StackEntry {
	enum class Type { Value, Label, Activations };
	Type type;
	union {
		uint32_t operand;
	} value;
};

uint32_t decode_u32(const std::vector<uint8_t> &bin, size_t &header) {
	uint32_t result = 0;
	size_t shift = 0;
	do {
		result |= ((bin[header++] & 0b01111111) << shift);
		shift += 7;
	} while ((result & 0b10000000) >> 7);
	return result;
}

void read_custom_section(Module &module, const std::vector<uint8_t> &bin,
						 size_t &header) {
	uint32_t section_size = decode_u32(bin, header);
	header += section_size;
}

void read_type_section(Module &module, const std::vector<uint8_t> &bin,
					   size_t &header) {
	uint32_t section_size = decode_u32(bin, header);
	uint32_t num_types = decode_u32(bin, header);

	if (num_types <= 0) {
		return;
	}

	module.types = static_cast<FunctionType *>(
		std::malloc(num_types * sizeof(FunctionType)));
	module.type_count = num_types;

	for (size_t i = 0; i < num_types; ++i) {
		FunctionType &current_type = module.types[i];

		if (bin[header++] !=
			static_cast<std::underlying_type_t<TypeId>>(TypeId::Function)) {
			continue;
		}

		uint32_t num_params = decode_u32(bin, header);
		current_type.param_count = num_params;
		if (num_params > 0) {
			current_type.param_types = static_cast<ValueType *>(
				std::malloc(num_params * sizeof(ValueType)));
			for (size_t j = 0; j < num_params; ++j) {
				current_type.param_types[j] =
					static_cast<ValueType>(bin[header++]);
			}
		}

		uint32_t num_rettype = decode_u32(bin, header);
		current_type.result_count = num_rettype;
		if (num_rettype > 0) {
			current_type.param_types = static_cast<ValueType *>(
				std::malloc(num_rettype * sizeof(ValueType)));
			for (size_t j = 0; j < num_rettype; ++j) {
				current_type.param_types[j] =
					static_cast<ValueType>(bin[header++]);
			}
		}
	}
}

void read_function_section(Module &module, const std::vector<uint8_t> &bin,
						   size_t &header) {
	uint32_t section_size = decode_u32(bin, header);
	uint32_t num_functions = decode_u32(bin, header);

	module.func_count = num_functions;

	if (num_functions <= 0) {
		return;
	}

	module.funcs =
		static_cast<Function *>(std::malloc(num_functions * sizeof(Function)));

	for (size_t i = 0; i < num_functions; ++i) {
		module.funcs[i].type_index = decode_u32(bin, header);
	}
}

void read_start_section(Module &module, const std::vector<uint8_t> &bin,
						size_t &header) {
	uint32_t section_size = decode_u32(bin, header);

	module.start = static_cast<Start *>(std::malloc(sizeof(Start)));
	module.start->func_index = decode_u32(bin, header);
}

void read_code_section(Module &module, const std::vector<uint8_t> &bin,
					   size_t &header) {
	uint32_t section_size = decode_u32(bin, header);
	uint32_t num_functions = decode_u32(bin, header);

	for (size_t i = 0; i < num_functions; ++i) {
		Function &func = module.funcs[i];

		uint32_t func_body_size = decode_u32(bin, header);
		uint32_t local_decl_count = decode_u32(bin, header);

		uint8_t op_code = bin[header];
		size_t op_count = 0;
		size_t ptr = header;
		while (op_code != END_MARKER) {
			switch (static_cast<Instruction::OpCode>(op_code)) {
			case Instruction::OpCode::i32_const:
				op_count++;
				ptr += 2;
				op_code = bin[ptr];
				break;

			default:
				op_count++;
				op_code = bin[++ptr];
				break;
			}
		}

		func.body = static_cast<Instruction *>(
			std::malloc((op_count + 1) * sizeof(Instruction)));

		for (size_t j = 0; j < op_count + 1; ++j) {
			auto op_code = static_cast<Instruction::OpCode>(bin[header++]);
			func.body[j].op_code = op_code;
			switch (op_code) {
			case Instruction::OpCode::i32_const:
				func.body[j].args.literal = decode_u32(bin, header);
				break;

			default:
				break;
			}
		}
	}

	std::cout << "code section finished" << std::endl;
}

ModuleInstance instantiate_module(const Module &module) {
	ModuleInstance instance{
		.module = &module,
		.types = module.types,
		.type_count = module.type_count,
	};

	instance.store.funcs = static_cast<FunctionInstance *>(
		std::malloc(module.func_count * sizeof(FunctionInstance)));
	for (size_t i = 0; i < module.func_count; ++i) {
		Function &func = module.funcs[i];
		FunctionInstance &func_instance = instance.store.funcs[i];
		func_instance.module = &instance;
		func_instance.code = func;
		func_instance.type = module.types[func.type_index];
	}

	return instance;
};

void describe_module(const Module &module) {
	std::cout << "========== wasm module description ==========" << std::endl;

	std::cout << "number of functypes: " << module.type_count << std::endl;
	std::cout << "number of funcs: " << module.func_count << std::endl;

	for (size_t i = 0; i < module.type_count; ++i) {
		std::cout << "function " << i << ":" << std::endl;
		FunctionType &functype = module.types[i];
		std::cout << "    param count: " << functype.param_count << std::endl;
		std::cout << "    rettype count: " << functype.result_count
				  << std::endl;
	}

	if (module.start) {
		std::cout << "start index: " << module.start->func_index << std::endl;
	}

	std::cout << "========== wasm module description ==========" << std::endl;
}

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

void execute_module_instance(ModuleInstance &instance) {
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

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "please pass in at least one executable wasm binary."
				  << std::endl;
		return -1;
	}

	std::ifstream f(argv[1], std::ios::binary);
	std::vector<uint8_t> bin(std::istreambuf_iterator<char>(f), {});

	std::cout << +bin[0] << +bin[1] << +bin[2] << +bin[3] << std::endl;

	// check wasm header
	if (bin[0] != 0 || bin[1] != 0x61 || bin[2] != 0x73 || bin[3] != 0x6D) {
		std::cerr << "invalid wasm binary" << std::endl;
	}

	std::cout << "valid wasm binary!" << std::endl;

	Module module;

	size_t header = 8;
	size_t end = bin.size();
	while (header < end) {
		auto byte = bin[header];
		switch (static_cast<SectionType>(byte)) {
		case SectionType::Custom:
			read_custom_section(module, bin, ++header);
			break;

		case SectionType::Type:
			read_type_section(module, bin, ++header);
			break;

		case SectionType::Function:
			read_function_section(module, bin, ++header);
			break;

		case SectionType::Start:
			read_start_section(module, bin, ++header);
			break;

		case SectionType::Code:
			read_code_section(module, bin, ++header);
			break;

		default:
			break;
		}
	}

	describe_module(module);

	ModuleInstance module_instance = instantiate_module(module);
	execute_module_instance(module_instance);
}
