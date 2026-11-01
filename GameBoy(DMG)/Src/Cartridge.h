#pragma once
#include <Common.h>

#define CA_TITLE_LENGHT 16

enum cartidge_type : uint32_t {
	ca_rom_only			= 0x0,
	ca_mbc1				= 0x00001,
	ca_mbc2				= 0x00002,
	ca_mbc3				= 0x00004,
	ca_mbc5				= 0x00008,
	ca_mbc6				= 0x00010,
	ca_mbc7				= 0x00020,
	ca_ram				= 0x00040,
	ca_rom				= 0x00080,
	ca_mmm01			= 0x00100,
	ca_battery			= 0x00200,
	ca_timer			= 0x00400,
	ca_rumble			= 0x00800,
	ca_sensor			= 0x01000,
	ca_pocket_camera	= 0x02000,
	ca_bandai_tama5		= 0x04000,
	ca_huc1				= 0x08000,
	ca_huc3				= 0x10000,
};

typedef struct cartridge_header {
	std::string					title{};
	bool						cgb{};
	uint32_t					type{};
	uint32_t					rom_size{};
	uint32_t					ram_size{};
} cartridge_header;

typedef struct cartridge {
	std::unique_ptr<uint8_t[]>	memory{};
	uint32_t					size{};
	cartridge_header			header{};
} cartridge;

bool ca_load_cartridge(
	cartridge& c,
	const char* path);

//Returns false if GB would allow rom to run
bool ca_parse_header(
	cartridge& c,
	cartridge_header& h,
	bool allow_cgb,
	bool dump = false);

uint8_t ca_read(
	cartridge& c,
	uint16_t addr);

void ca_write(
	cartridge& c,
	uint16_t addr,
	uint8_t value);
