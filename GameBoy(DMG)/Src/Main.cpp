#include <iostream>
#include <Bus.h>
#include <Cpu.h>
#include <Cartridge.h>

int main() {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	bus b{};

	if (!ca_load_cartridge(b.car, "Content\\Tetris (USA) (Rev-A).gb"))
		return -1;

	if (!ca_parse_header(b.car, b.car.header, false, true))
		return -1;

	sm83 sm{ b };
	for (uint32_t i{ 0 }; i < 10; ++i) { 
		uint16_t last_pc{ sm.reg.pc };
		cpu_step(sm);
		printf("Addr: 0x%04X OpCode 0x%02X\n", last_pc, sm.curr_opcode);
	}


	return 0;
}
