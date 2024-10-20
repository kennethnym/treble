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

enum class OpCode {
	i32_const = 0x41,
	i64_const = 0x42,
};

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
	ModuleInstance *module;
	struct Function code;
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

uint32_t decode_u32(const std::vector<uint8_t> &bin, size_t &header) {
	uint8_t result;
	size_t shift = 0;
	do {
		result |= ((bin[header++] & 0x01111111) << shift);
		shift += 7;
	} while ((result & 0x10000000) >> 7);
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
		FunctionType current_type = module.types[i];

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
		Function *func = &module.funcs[i];

		uint32_t func_body_size = decode_u32(bin, header);
		uint32_t local_decl_count = decode_u32(bin, header);

		uint8_t op_code = bin[header];
		size_t op_count = 0;
		size_t ptr = header;
		while (op_code != END_MARKER) {
			op_count++;
			op_code = bin[++ptr];
		}

		func->body = static_cast<Instruction *>(
			std::malloc(op_count * sizeof(Instruction)));

		for (size_t j = 0; j < op_count; ++j) {
			auto op_code = static_cast<Instruction::OpCode>(bin[header++]);
			func->body[j].op_code = op_code;
			switch (op_code) {
			case Instruction::OpCode::i32_const:
				func->body[j].args.literal = decode_u32(bin, header);
				break;

			default:
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "please pass in at least one executable wasm binary."
				  << std::endl;
		return -1;
	}

	uint8_t stack[65536];

	std::ifstream f(argv[1], std::ios::binary);
	std::vector<uint8_t> bin(std::istreambuf_iterator<char>(f), {});

	// check wasm header
	if (bin[0] != 0 || bin[1] != 0x61 || bin[2] != 0x73 || bin[3] != 0x6D) {
		std::cerr << "invalid wasm binary" << std::endl;
	}

	Module module;

	size_t header = 8;
	size_t end = bin.size();
	while (header++ < end) {
		auto byte = bin[header];
		switch (static_cast<SectionType>(byte)) {
		case SectionType::Custom:

		case SectionType::Type:
			read_type_section(module, bin, header);
			break;

		case SectionType::Function:
			read_function_section(module, bin, header);
			break;

		case SectionType::Start:
			read_start_section(module, bin, header);
			break;

		case SectionType::Code:
			read_code_section(module, bin, header);
			break;

		default:
			break;
		}
	}
}
