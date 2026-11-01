#pragma once
#include <Common.h>
#include <Cartridge.h>

/*
The Game Boy has a 16-bit address bus, which is used to address ROM, RAM, and I/O.

Start	End		Description						Notes
0000	3FFF	16 KiB ROM bank 00				From cartridge, usually a fixed bank
4000	7FFF	16 KiB ROM Bank 01–NN			From cartridge, switchable bank via mapper (if any)
8000	9FFF	8 KiB Video RAM (VRAM)			In CGB mode, switchable bank 0/1
A000	BFFF	8 KiB External RAM				From cartridge, switchable bank if any
C000	CFFF	4 KiB Work RAM (WRAM)
D000	DFFF	4 KiB Work RAM (WRAM)			In CGB mode, switchable bank 1–7
E000	FDFF	Echo RAM (mirror of C000–DDFF)	Nintendo says use of this area is prohibited.
FE00	FE9F	Object attribute memory (OAM)
FEA0	FEFF	Not Usable						Nintendo says use of this area is prohibited.
FF00	FF7F	I/O Registers
FF80	FFFE	High RAM (HRAM)
FFFF	FFFF	Interrupt Enable register (IE)
*/

#define ROM_BANK_SIZE (16 * 1024)
#define VRAM_SIZE (8 * 1024)
#define EXRAM_SIZE (8 * 1024)
#define WRAM_SIZE (4 * 1024)
#define OAM_SIZE (0xFE9F - 0xFE00)
#define IO_SIZE (0xFF7F - 0xFF00)
#define HRAM_SIZE (0xFFFE - 0xFF80)

typedef struct bus {
	uint8_t			vram[VRAM_SIZE]{};
	uint8_t			wram[WRAM_SIZE]{};
	uint8_t			oam[OAM_SIZE]{};
	uint8_t			io[IO_SIZE]{};
	uint8_t			hram[HRAM_SIZE]{};
	//IE

	cartridge		car;
	uint32_t		rom_bank{ 0 };
} bus;

bool bus_switch_rom_bank(
	bus& b, 
	uint32_t bank);

uint8_t bus_read(
	bus& b, 
	uint16_t addr);

void bus_write(
	bus& b, 
	uint16_t addr, 
	uint8_t value);
