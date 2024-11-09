#include "module.hxx"
#include "instructions.hxx"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <stack>
#include <sys/types.h>
#include <vector>

namespace Treble {

struct BlockBegin {
	size_t instr_pos;
};

size_t varint_size(const std::vector<uint8_t> &bin, size_t header) {
	size_t size = 0;
	size_t shift = 0;
	size_t i = header;
	while (true) {
		const auto b = bin[i++];
		size++;
		if ((b & 0b10000000) == 0) {
			break;
		}
	}
	return size;
}

uint32_t decode_u32(const std::vector<uint8_t> &bin, size_t &header) {
	uint32_t result = 0;
	size_t shift = 0;
	while (true) {
		const auto b = bin[header++];
		std::cout << "header " << header << std::endl;
		result |= ((b & 0b01111111) << shift);
		if ((b & 0b10000000) == 0) {
			std::cout << "result " << result << std::endl;
			break;
		}
		shift += 7;
	}
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

		uint block_level = 0;
		bool end_reached = false;
		while (!end_reached) {
			switch (static_cast<Instruction::OpCode>(op_code)) {
			case Instruction::OpCode::i32_const:
				op_count++;
				std::cout << "ptr before " << ptr << std::endl;
				ptr++;
				ptr += varint_size(bin, ptr);
				std::cout << "ptr after " << ptr << std::endl;
				op_code = bin[ptr];
				std::cout << "op code after " << +op_code << std::endl;
				break;

			case Instruction::OpCode::if_:
				block_level++;
				op_count++;
				ptr += 2;
				op_code = bin[ptr];
				break;

			case Instruction::OpCode::end:
				op_count++;
				if (block_level == 0) {
					end_reached = true;
				} else {
					block_level--;
				}
				break;

			default:
				op_count++;
				op_code = bin[++ptr];
				break;
			}
		}

		std::cout << "op count " << op_count << std::endl;

		func.body = static_cast<Instruction *>(
			std::malloc((op_count) * sizeof(Instruction)));

		std::stack<BlockBegin> block_stack;

		for (size_t j = 0; j < op_count; ++j) {
			auto op_code = static_cast<Instruction::OpCode>(bin[header++]);
			func.body[j].op_code = op_code;
			switch (op_code) {
			case Instruction::OpCode::i32_const:
				func.body[j].args.literal = decode_u32(bin, header);
				break;

			case Instruction::OpCode::if_:
				func.body[j].args.if_branch.block_type = nullptr;
				header++;
				func.body[j].args.if_branch.instr_1_offset = 1;
				block_stack.push({.instr_pos = j});
				break;

			case Instruction::OpCode::else_: {
				auto &block_begin = block_stack.top();

				func.body[block_begin.instr_pos].args.if_branch.instr_2_offset =
					j - block_begin.instr_pos + 1;

				block_begin.instr_pos = j;

				break;
			}

			case Instruction::OpCode::end: {
				if (block_stack.empty()) {
					break;
				}

				auto block_begin = block_stack.top();

				func.body[block_begin.instr_pos]
					.args.else_branch.end_marker_offset =
					j - block_begin.instr_pos;

				block_stack.pop();

				break;
			}

			default:
				break;
			}

			std::cout << "header " << header << std::endl;
		}
	}

	std::cout << "code section finished: " << header << std::endl;
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
			std::cout << "header after code section: " << header << std::endl;
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
