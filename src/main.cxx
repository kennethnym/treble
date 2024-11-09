#include "module.hxx"
#include "runtime.hxx"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <optional>
#include <sys/types.h>
#include <vector>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "please pass in at least one executable wasm binary."
				  << std::endl;
		return -1;
	}

	std::ifstream f(argv[1], std::ios::binary);
	std::vector<uint8_t> bin(std::istreambuf_iterator<char>(f), {});

	std::optional<Treble::Module> module = Treble::parse_binary(bin);

	std::cout << "binary parsed" << std::endl;

	Treble::describe_module(*module);
	Treble::ModuleInstance module_instance = instantiate_module(*module);

	std::cout << "module instantiated. executing" << std::endl;

	execute_module_instance(module_instance);
}
