#include <Bus.h>

bool
bus_switch_rom_bank(
	bus& b, 
	uint32_t bank) {
	b.rom_bank = bank;
	
	return true;
}

uint8_t
bus_read(
	bus& b, 
	uint16_t addr) {
	if (addr <= 0x3FFF) {
		return ca_read(b.car, addr);
	}
	else if (addr <= 0x7FFF) {
		return ca_read(b.car, (addr - 0x4000) + b.rom_bank * ROM_BANK_SIZE);
	}

	return 0;
}

void
bus_write(
	bus& b, 
	uint16_t addr, 
	uint8_t value) {
	return;
}
