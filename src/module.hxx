#ifndef __TREBLE__MODULE_HXX__
#define __TREBLE__MODULE_HXX__

#include "instructions.hxx"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <vector>

namespace Treble {

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

std::optional<Module> parse_binary(const std::vector<uint8_t> &bin);

ModuleInstance instantiate_module(const Module &module);

void describe_module(const Module &module);

} // namespace Treble

#endif
