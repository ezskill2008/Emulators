#include <iostream>
#include <Bus.h>
#include <Cpu.h>
#include <Cartridge.h>

int main() {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	bus b{};

	if (!ca_load_cartridge(b.car, "Content\\cpu_instrs\\individual\\01-special.gb"))
		return -1;

	if (!ca_parse_header(b.car, b.car.header, false, true))
		return -1;

	sm83 sm{ b };
	for (uint32_t i{ 0 }; i < 100; ++i) { 
		uint16_t last_pc{ sm.reg.pc };
		cpu_step(sm);
		std::string istr{ cpu_inst_to_str(sm) };
		printf("$%04X %s\n", last_pc, istr.c_str());
	}

	return 0;
}
