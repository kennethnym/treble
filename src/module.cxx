#include "module.hxx"
#include <cstdint>
#include <iostream>
#include <optional>

namespace Treble {

uint32_t decode_u32(const std::vector<uint8_t> &bin, size_t &header) {
	uint32_t result = 0;
	size_t shift = 0;
	do {
		result |= ((bin[header++] & 0b01111111) << shift);
		shift += 7;
	} while ((result & 0b10000000) >> 7);
	return result;
}

void read_code_section(Treble::Module &module, const std::vector<uint8_t> &bin,
					   size_t &header) {
	uint32_t section_size = decode_u32(bin, header);
	uint32_t num_functions = decode_u32(bin, header);

	for (size_t i = 0; i < num_functions; ++i) {
		Treble::Function &func = module.funcs[i];

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

void read_start_section(Treble::Module &module, const std::vector<uint8_t> &bin,
						size_t &header) {
	uint32_t section_size = decode_u32(bin, header);

	module.start =
		static_cast<Treble::Start *>(std::malloc(sizeof(Treble::Start)));
	module.start->func_index = decode_u32(bin, header);
}

void read_function_section(Treble::Module &module,
						   const std::vector<uint8_t> &bin, size_t &header) {
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

void read_type_section(Treble::Module &module, const std::vector<uint8_t> &bin,
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

void read_custom_section(Module &module, const std::vector<uint8_t> &bin,
						 size_t &header) {
	uint32_t section_size = decode_u32(bin, header);
	header += section_size;
}

std::optional<Treble::Module> parse_binary(const std::vector<uint8_t> &bin) {
	// check wasm header
	if (bin[0] != 0 || bin[1] != 0x61 || bin[2] != 0x73 || bin[3] != 0x6D) {
		return std::nullopt;
	}

	std::cout << "valid wasm binary!" << std::endl;

	auto module = std::make_optional<Treble::Module>();

	size_t header = 8;
	size_t end = bin.size();
	while (header < end) {
		auto byte = bin[header];
		switch (static_cast<SectionType>(byte)) {
		case SectionType::Custom:
			read_custom_section(*module, bin, ++header);
			break;

		case SectionType::Type:
			read_type_section(*module, bin, ++header);
			break;

		case SectionType::Function:
			read_function_section(*module, bin, ++header);
			break;

		case SectionType::Start:
			read_start_section(*module, bin, ++header);
			break;

		case SectionType::Code:
			read_code_section(*module, bin, ++header);
			break;

		default:
			break;
		}
	}

	return module;
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

} // namespace Treble
